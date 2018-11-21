#include "xyginext/audio/midi/File.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

#include "Util.hpp"

using namespace xy;
using namespace xy::Midi;

File::File() : m_timeDivision(500), m_playing(false) {}

File::File(const std::string& path)
    : m_timeDivision(500),
    m_playing       (false)
{
    loadFromFile(path);
}

//public
bool File::loadFromFile(const std::string& path)
{
    if (m_playing)
    {
        xy::Logger::log("Cannot open " + path + " - file is already playing. Stop the playback first.", xy::Logger::Type::Error);
        return false;
    }

    clear();

    std::fstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        xy::Logger::log("Failed opening " + path, xy::Logger::Type::Error);
        return false;
    }

    file.seekg(0, std::fstream::end);
    auto length = file.tellg();
    file.seekg(0, std::fstream::beg);

    auto headerCount = 0;
    while (file.good() && (length - file.tellg() > 0))
    {
        auto chunkID = Detail::readBigEndian<std::uint32_t>(file);

        switch (chunkID)
        {
        default:
            readUnknown(file);
            break;
        case 0x4D546864: //"MThd"
            if (readHeader(file))
            {
                headerCount++;
            }
            break;
        case 0x4D54726B: //"MTrk"
            readTrack(file);
            break;
        }
    }
    file.close();

    if (headerCount == 0)
    {
        xy::Logger::log(path + ": no header chunk file - possibly not a midi file", xy::Logger::Type::Error);
        clear();
        return false;
    }

    return true;
}

bool File::loadFromMemory(const std::vector<std::uint8_t>&)
{
    xy::Logger::log("Loading MIDI file from memory not yet implemented", xy::Logger::Type::Warning);
    return false;
}

std::size_t File::getTrackCount() const
{
    return size();
}

std::uint16_t File::getTimeDivision() const
{
    return m_timeDivision;
}

void File::setTimeDivision(std::uint16_t division)
{
    m_timeDivision = division;
}

sf::Time File::getDuration() const
{
    xy::Logger::log("File duration calculation not yet implemented", xy::Logger::Type::Warning);
    return {};
}

//private
bool File::readHeader(std::fstream& stream)
{
    auto headerSize = Detail::readBigEndian<std::uint32_t>(stream);
    if (headerSize != 6)
    {
        xy::Logger::log("Failed to read header chunk, invalid size: " + std::to_string(headerSize), xy::Logger::Type::Error);
        return false;
    }

    auto fileType = Detail::readBigEndian<std::uint16_t>(stream);
    if (fileType != 0 && fileType != 1)
    {
        xy::Logger::log(std::to_string(fileType) + ": not a valid MIDI file type", xy::Logger::Type::Error);
        return false;
    }

    auto trackCount = Detail::readBigEndian<std::uint16_t>(stream);
    reserve(trackCount); //we have to read it to move forward in the stream, so might as well do something useful with it!

    auto timeDivision = Detail::readBigEndian<std::uint16_t>(stream);
    if (timeDivision & 0x8000)
    {
        xy::Logger::log("Unsupported MIDI time division", xy::Logger::Type::Error);
        return false;
    }

    return true;
}

void File::readTrack(std::fstream& stream)
{
    emplace_back();
    auto& track = back();

    auto chunkSize = Detail::readBigEndian<std::uint32_t>(stream);
    std::uint8_t status = 0;
    auto streamStart = stream.tellg();
    bool loadContinue = true;

    while (loadContinue)
    {
        auto& evt = track.addEvent();
        auto dt = Detail::getVlq(stream);
        evt.setDt(dt);

        readEvent(stream, evt, loadContinue, status);
    }

    if (chunkSize != stream.tellg() - streamStart)
    {
        //TODO warn here? something's not right...
    }
}
void File::readEvent(std::fstream& stream, Event& dest, bool& loadContinue, std::uint8_t& status)
{
    auto cmd = Detail::readBigEndian<std::uint8_t>(stream);
    bool incomplete = false;

    if (cmd < 0x80)
    {
        incomplete = true; //we're going to need to read more bytes for full event
        dest.push_back(status);
        dest.push_back(cmd);
        cmd = status;
    }
    else
    {
        status = cmd;
        dest.push_back(cmd);
    }

    //check control events
    switch (cmd & 0xf0)
    {
    default: break;
        //one param events
    case Event::ProgramChange:
    case Event::ChannelAftertouch:
    {
        if (!incomplete)
        {
            dest.push_back(Detail::readBigEndian<std::uint8_t>(stream));
        }
    }
    break;
        //two paramter events
    case Event::NoteOn:
    case Event::NoteOff:
    case Event::NoteAftertouch:
    case Event::ControlChange:
    case Event::PitchWheel:
    {
        dest.push_back(Detail::readBigEndian<std::uint8_t>(stream));

        if (!incomplete)
        {
            dest.push_back(Detail::readBigEndian<std::uint8_t>(stream));
        }
    }
    break;
        //meta events, or sysex
    case 0xf0:
        switch (cmd)
        {
        case 0xff: //meta
        {
            std::uint8_t metaEventType = Detail::readBigEndian<std::uint8_t>(stream);
            dest.push_back(metaEventType);

            switch (metaEventType)
            {
            default:

            case Event::SequenceNumber: // size always 2
            case Event::Text:
            case Event::Copyright:
            case Event::TrackName:
            case Event::InstrumentName:
            case Event::Lyrics:
            case Event::Marker:
            case Event::CuePoint:
            case Event::ChannelPrefix: // size always 1
            case Event::OutputCable: //size always 1
            case Event::EndOfTrack: // size always 0
            case Event::Tempo: // size always 3
            case Event::SmpteOffset: // size always 5
            case Event::TimeSignature:
            case Event::KeySignature:
            {
                //read string
                std::uint8_t strLength = Detail::readBigEndian<std::uint8_t>(stream);
                for (int i = 0; i < strLength; i++)
                {
                    dest.push_back(Detail::readBigEndian<std::uint8_t>(stream));
                }

                if (metaEventType == 0x2f)
                {
                    loadContinue = false;
                }
            }
                break;
            }
            break;
        }
        case Event::SysExBegin:
        case Event::SysExEnd:
        {
            auto size = Detail::getVlq(stream);

            for (unsigned int i = 0; i < size; i++)
            {
                dest.push_back(Detail::readBigEndian<std::uint8_t>(stream));
            }
        }
            break;
        }
        break;
    }
}

void File::readUnknown(std::fstream& stream)
{
    //skip unknown chunks
    auto chunkSize = Detail::readBigEndian<std::uint32_t>(stream);
    stream.seekg(chunkSize, std::fstream::cur);
}