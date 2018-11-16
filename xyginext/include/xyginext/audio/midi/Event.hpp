#pragma once

#include "Message.hpp"

#include <cstdint>

namespace xy
{
    namespace Midi
    {
        class Event final : public Message
        {
        public: 
            Event() = default;
            explicit Event(std::uint32_t);
            explicit Event(std::uint32_t, const Message&);
            explicit Event(std::uint32_t, std::uint8_t);
            explicit Event(std::uint32_t, std::uint8_t, std::uint8_t);
            explicit Event(std::uint32_t, std::uint8_t, std::uint8_t, std::uint8_t);

            uint32_t getDt() const;
            void setDt(std::uint32_t);

        private:
            std::uint32_t m_dt;
        };
    }
}