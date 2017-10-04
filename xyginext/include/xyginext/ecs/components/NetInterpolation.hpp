/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#ifndef XY_NET_INTERPOLATE_HPP_
#define XY_NET_INTERPOLATE_HPP_

#include <xyginext/Config.hpp>
#include <SFML/Config.hpp>

namespace xy
{
    /*!
    \brief Interpolates positions received from a server.
    When receiving infrequent (say 100ms or so) position updates from
    a remote server entities can have their position interpolated via
    this component. The component, when coupled with an InterpolationSystem
    will travel towards the given target position using the given timestamp
    to linearly interpolate movement. This component is not limited to
    networked entities, and can be used anywhere linear interpolation of
    movement is desired.
    */
    class XY_EXPORT_API NetInterpolate final
    {
    public:
        NetInterpolate();

        /*!
        \brief Sets the target position and timestamp.
        The timestamp is used in conjunction with the previous timestamp
        to decide how quickly motion should be integrated between positions.
        The timestamp would usually be in server time, and arrive in the packet
        data with the destination postion, in milliseconds.
        \param pos Target destination
        \param timestamp Time at which this position should be reached, relative
        to the previous position.
        */
        void setTarget(sf::Vector2f pos, sf::Int32 timestamp);

    private:
        sf::Vector2f m_targetPosition;
        float m_targetTimestamp;

        float m_elapsedTime;
        float m_timeDifference;

        float m_previousTimestamp;
        sf::Vector2f m_previousPosition;

        friend class InterpolationSystem;
    };
}

#endif //XY_NET_INTERPOLATE_HPP_