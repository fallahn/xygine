#include "xyginext/audio/midi/Event.hpp"

using namespace xy;
using namespace xy::Midi;

Event::Event(std::uint32_t dt)
    : m_dt(dt) {}

Event::Event(std::uint32_t dt, const Message& msg)
    : Message(msg), m_dt(dt) {}

Event::Event(std::uint32_t dt, std::uint8_t byte1)
    : m_dt(dt)
{
    push_back(byte1);
}

Event::Event(std::uint32_t dt, std::uint8_t byte1, std::uint8_t byte2)
    : m_dt(dt)
{
    push_back(byte1);
    push_back(byte2);
}

Event::Event(std::uint32_t dt, std::uint8_t byte1, std::uint8_t byte2, std::uint8_t byte3)
    : m_dt(dt)
{
    push_back(byte1);
    push_back(byte2);
    push_back(byte3);
}

std::uint32_t Event::getDt() const
{
    return m_dt;
}

void Event::setDt(std::uint32_t dt)
{
    m_dt = dt;
}