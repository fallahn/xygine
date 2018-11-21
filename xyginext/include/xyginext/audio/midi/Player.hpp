#pragma once

#include "xyginext/Config.hpp"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Thread.hpp>

#include <memory>
#include <atomic>
#include <vector>

class RtMidiOut;
namespace xy
{
    namespace Midi
    {
        class File;
        class Event;
        class XY_EXPORT_API Player final
        {
        public:
            Player();
            ~Player();

            Player(const Player&) = delete;
            Player(Player&&) = delete;
            const Player& operator = (const Player&) = delete;
            Player& operator = (Player&&) = delete;

            void play(const File&);
            void pause();

        private:

            bool m_outputOK;
            std::atomic_bool m_playing;
            std::unique_ptr<RtMidiOut> m_output;

            std::uint32_t m_tempo; //microsec/quarternote
            const File* m_file;
            sf::Time m_currentPosition;

            std::uint64_t m_heartbeatTime;
            sf::Thread m_thread;

            struct State final
            {
                State(std::uint32_t tp, std::uint32_t td)
                    : trackPosition(tp), trackDT(td) {}
                std::uint32_t trackPosition = 0;
                std::uint32_t trackDT = 0;
            };
            std::vector<State> m_trackStates;

            bool trackFinished(std::uint32_t) const;
            std::uint32_t trackPending() const;
            void processEvent(const Event&);
            void initTrackStates();
            void updateTrackState(std::uint32_t, std::uint32_t);
            bool finished() const;
            void process();

            std::uint32_t getTempo(const Event&);
            std::int64_t getMicrosec(std::uint32_t, std::uint32_t, std::uint16_t);
        };
    }
}
