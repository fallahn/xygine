#include "xyginext/audio/midi/Track.hpp"

using namespace xy;
using namespace xy::Midi;

Event& Track::addEvent()
{
    emplace_back();
    return back();
}

std::string Track::getName() const
{
    std::string retVal;

    for (auto i = 0u; i < size(); ++i)
    {
        const auto& evt = at(i);
        if (evt.isMeta())
        {
            if (evt.at(1) == Event::TrackName)
            {
                if (!retVal.empty())
                {
                    retVal += ", ";
                }
                retVal += evt.getText();
            }
        }
    }
    return retVal;
}