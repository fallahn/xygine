#include "xyginext/audio/midi/Message.hpp"

using namespace xy;
using namespace xy::Midi;

Message::Message(std::uint8_t byte1)
{
    push_back(byte1);
}

Message::Message(std::uint8_t byte1, std::uint8_t byte2)
{
    push_back(byte1);
    push_back(byte1);
}

Message::Message(std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3)
{
    push_back(byte1);
    push_back(byte2);
    push_back(byte3);
}

//public
Message::Type Message::getType() const
{
    return (empty()) ? Undefined : static_cast<Type>((*this)[0]);
}

bool Message::isVoice() const
{
    return (empty()) ?
        false :
        ((*this)[0] >= 0x80) && ((*this)[0] <= 0xef);
}

bool Message::isSysCommon() const
{
    return (empty()) ?
        false :
        ((*this)[0] >= 0xf0) && ((*this)[0] <= 0xf7);
}

bool Message::isSysEx() const
{
    return (empty()) ?
        false :
        ((*this)[0] == Type::SysExBegin) || ((*this)[0] == Type::SysExEnd);
}

bool Message::isRealtime() const
{
    return (empty()) ?
        false :
        ((*this)[0] >= 0xf8);
}

bool Message::isMeta() const
{
    return (size() < 2) ?
        false :
        ((*this)[0] == 0xff);
}

bool Message::isMeta(Message::MetaType type) const
{
    return (isMeta() && ((*this)[1] == type));
}

bool Message::hasText() const
{
    return (size() < 2) ?
        false :
        ((*this)[0] == 0xff) 
        && (((*this)[1] == MetaType::Text) 
            || ((*this)[1] == MetaType::Lyrics)
            || ((*this)[1] == MetaType::InstrumentName)
            || ((*this)[1] == MetaType::TrackName)
            || ((*this)[1] == MetaType::Copyright));
}

std::string Message::getText() const
{
    if (hasText())
    {
        return std::string((char*)data() + 2);
    }
    return {};
}

std::string Message::getName() const
{
    if (isMeta())
    {
        switch ((*this)[1])
        {
        default: break;
        case SequenceNumber: 
            return "SequenceNumber";
        case Text: 
            return "Text";
        case Copyright: 
            return "Copyright";
        case TrackName: 
            return "TrackName";
        case InstrumentName: 
            return "InstrumentName";
        case Lyrics: 
            return "Lyrics";
        case Marker: 
            return "Marker";
        case CuePoint: 
            return "CuePoint";
        case ChannelPrefix: 
            return "ChannelPrefix";
        case OutputCable: 
            return "OutputCable";
        case EndOfTrack: 
            return "EndOfTrack";
        case Tempo: 
            return "Tempo";
        case SmpteOffset: 
            return "SmpteOffset";
        case TimeSignature: 
            return "TimeSignature";
        case KeySignature: 
            return "KeySignature";
        }
    }

    if (!empty())
    {
        switch ((*this)[0] & 0xf0)
        {
        default: break;
        case NoteOff:
            return "NoteOff";
        case NoteOn:
            return "NoteOn";
        case NoteAftertouch:
            return "NoteAftertouch";
        case ControlChange:
            return "ControlChange";
        case ProgramChange:
            return "ProgramChange";
        case ChannelAftertouch:
            return "ChannelAftertouch";
        case PitchWheel:
            return "PitchWheel";
        case SysExBegin:
            return "SysExBegin";
        case MtcQuarterFrame:
            return "MtcQuarterFrame";
        case SongPositionPointer:
            return "SongPositionPointer";
        case SongSelect:
            return "SongSelect";
        case TuneRequest:
            return "TuneRequest";
        case SysExEnd:
            return "SysExEnd";
        case Clock:
            return "Clock";
        case Tick:
            return "Tick";
        case Start:
            return "Start";
        case Continue:
            return "Continue";
        case Stop:
            return "Stop";
        case ActiveSense:
            return "ActiveSense";
        case Reset:
            return "Reset";
        }
    }
    return {};
}