#include "xyginext/audio/midi/File.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

using namespace xy;
using namespace xy::Midi;

File::File() : m_timeDivision(500) {}

File::File(const std::string& path)
    : m_timeDivision(500)
{
    loadFromFile(path);
}

//public
bool File::loadFromFile(const std::string& path)
{
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
        auto chunkID = 0;
        XY_ASSERT(chunkID, "haven't implemented endianess utils");
        switch (chunkID)
        {
        default:
            readUnknown(file);
            break;
        case 0x4D546864: //"MThd"
            readHeader(file);
            headerCount++;
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
void File::readHeader(std::fstream& stream)
{

}

void File::readTrack(std::fstream& stream)
{

}
void File::readEvent(std::fstream& stream, Event& dest, bool& loadContinue, std::uint8_t& status)
{

}

void File::readUnknown(std::fstream& stream)
{

}