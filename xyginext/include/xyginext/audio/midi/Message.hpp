#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace xy
{
    namespace Midi
    {
        class Message : private std::vector<std::uint8_t>
        {
        private:
            using ByteVector = std::vector<std::uint8_t>;
        public:
            Message() = default;
            explicit Message(std::uint8_t byte1);
            explicit Message(std::uint8_t byte1, std::uint8_t byte2);
            explicit Message(std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3);
            virtual ~Message() = default;

            using ByteVector::push_back;
            using ByteVector::begin;
            using ByteVector::end;
            using ByteVector::at;
            using ByteVector::data;
            using ByteVector::size;
            using ByteVector::operator[];

            enum Type
            {
                Undefined = 0x00,

                NoteOff = 0x80,
                NoteOn = 0x90,
                NoteAftertouch = 0xa0,
                ControlChange = 0xb0,
                ProgramChange = 0xc0, //size 1
                ChannelAftertouch = 0xd0, //size 1
                PitchWheel = 0xe0,

                SysExBegin = 0xf0,
                MtcQuarterFrame = 0xf1,
                SongPositionPointer = 0xf2,
                SongSelect = 0xf3,
                TuneRequest = 0xf6,
                SysExEnd = 0xf7,

                Clock = 0xf8,
                Tick = 0xf9,
                Start = 0xfa,
                Continue = 0xfb,
                Stop = 0xfc,
                ActiveSense = 0xfe,
                Reset = 0xff,

                Meta = 0xff
            };

            enum MetaType
            {
                SequenceNumber = 0x00, //size 2
                Text = 0x01,
                Copyright = 0x02,
                TrackName = 0x03,
                InstrumentName = 0x04,
                Lyrics = 0x05,
                Marker = 0x06,
                CuePoint = 0x07,
                ChannelPrefix = 0x20, //size 1
                OutputCable = 0x21, //size 1
                EndOfTrack = 0x2f, //size 0
                Tempo = 0x51, //size 3
                SmpteOffset = 0x54, //size 5
                TimeSignature = 0x58,
                KeySignature = 0x59
            };

            Type getType() const;
            bool isVoice() const;
            bool isSysCommon() const;
            bool isSysEx() const;
            bool isRealtime() const;
            bool isMeta() const;
            bool isMeta(MetaType) const;
            bool hasText() const;
            std::string getText() const;
            std::string getName() const;

        private:

        };
    }
}
