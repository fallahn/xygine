#include "xyginext/audio/midi/Player.hpp"
#include "xyginext/audio/midi/File.hpp"
#include "Util.hpp"

#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

#include <SFML/System/Sleep.hpp>
#include <RtMidi.h>

#include <limits>

using namespace xy;
using namespace xy::Midi;

Player::Player()
    : m_outputOK    (false),
    m_playing       (false),
    m_tempo         (0),
    m_file          (nullptr),
    m_heartbeatTime (0),
    m_thread        (&Player::process, this)
{
    m_output = std::make_unique<RtMidiOut>();
    if (m_output->getPortCount() > 0)
    {
        m_output->openPort(0);
        m_outputOK = true;
        LOG("Opened MIDI output device " + m_output->getPortName(0), xy::Logger::Type::Info);
    }
    else
    {
        xy::Logger::log("Failed opening MIDI output device, MIDI playback is unavailable", xy::Logger::Type::Warning);
    }
}

Player::~Player()
{
    if (m_file)
    {
        m_file->m_playing = false;
        m_playing = false;

        m_thread.wait();
    }
}

//public
void Player::play(const File& file)
{
    if (m_outputOK && !m_playing)
    {
        m_file = &file;
        m_file->m_playing = true;
        m_playing = true;

        m_trackStates.clear();
        m_currentPosition = sf::Time::Zero;
        m_heartbeatTime = 0;
        m_tempo = 500000;
        
        initTrackStates();

        m_thread.launch();
    }
}

void Player::pause()
{
    if (m_playing)
    {

    }
}

//private
bool Player::trackFinished(std::uint32_t id) const
{
    XY_ASSERT(m_file, "no file loaded");
    return (m_trackStates[id].trackPosition > (*m_file)[id].size());
}

std::uint32_t Player::trackPending() const
{
    std::uint32_t dt = std::numeric_limits<std::uint32_t>::max();
    std::uint32_t retVal = 0;

    for (auto i = 0u; i < m_trackStates.size(); ++i)
    {
        if (!trackFinished(i))
        {
            if (m_trackStates[i].trackDT < dt)
            {
                dt = m_trackStates[i].trackDT;
                retVal = i;
            }
        }
    }

    return retVal;
}

void Player::processEvent(const Event& evt)
{
    if (evt.isMeta())
    {
        if (evt.isMeta(Event::Tempo))
        {
            m_tempo = getTempo(evt);
        }
        return;
    }

    XY_ASSERT(m_output, "Output not initialised");
    m_output->sendMessage(evt.data(), evt.size());
}

void Player::initTrackStates()
{
    if (m_file)
    {
        m_trackStates.clear();
        for (auto i = 0u; i < m_file->getTrackCount(); ++i)
        {
            m_trackStates.emplace_back(0, (*m_file)[i][0].getDt());
        }
    }
}

void Player::updateTrackState(std::uint32_t trackID, std::uint32_t dt)
{
    for (auto i = 0u; i < m_trackStates.size(); ++i)
    {
        if (!trackFinished(i))
        {
            if (i == trackID)
            {
                XY_ASSERT(m_file, "No file loaded");
                auto n = ++m_trackStates[i].trackPosition;
                m_trackStates[i].trackDT = (*m_file)[i][n].getDt();
            }
            else
            {
                m_trackStates[i].trackDT -= dt;
            }
        }
    }
}

std::uint32_t Player::getTempo(const Event& evt)
{
    if (Detail::platformIsLittleEndian())
    {
        return ((evt[4] << 16) | (evt[3] << 8) | evt[2]);
    }
    else
    {
        return ((evt[2] << 16) | (evt[3] << 8) | evt[4]);
    }
}

std::int64_t Player::getMicrosec(std::uint32_t dt, std::uint32_t tempo, std::uint16_t division)
{
    return static_cast<std::int64_t>(tempo / static_cast<std::uint32_t>(division)) * dt;
}

bool Player::finished() const
{
    for (auto i = 0u; i < m_trackStates.size(); ++i)
    {
        if (!trackFinished(i))
        {
            return false;
        }
    }
    return true;
}

void Player::process()
{
    while (!finished() && m_playing)
    {
        auto trackNumber = trackPending();
        auto evtNumber = m_trackStates[trackNumber].trackPosition;
        auto dt = m_trackStates[trackNumber].trackDT;
        auto microsec = getMicrosec(dt, m_tempo, m_file->getTimeDivision()) / 4; //hmm why is this 4 times too long?

        while ((m_heartbeatTime + microsec) >= 10000)
        {
            auto part = 10000 - m_heartbeatTime;
            m_heartbeatTime = 0;
            microsec -= part;

            sf::sleep(sf::microseconds(part));
            m_currentPosition += sf::microseconds(part);
        }
        sf::sleep(sf::microseconds(microsec));
        m_heartbeatTime += microsec;
        m_currentPosition += sf::microseconds(microsec);

        processEvent((*m_file)[trackNumber][evtNumber]);
        updateTrackState(trackNumber, dt);
    }
    m_playing = false;
    m_file->m_playing = false;
}