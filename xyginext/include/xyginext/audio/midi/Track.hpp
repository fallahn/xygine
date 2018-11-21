#pragma once

#include "Event.hpp"

namespace xy
{
    namespace Midi
    {
        class Track final : private std::vector<Event>
        {
        public:
            Event& addEvent();
            std::string getName() const;

            using std::vector<Event>::size;
            using std::vector<Event>::operator[];

        private:
        };
    }
}
