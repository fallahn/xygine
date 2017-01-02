/*********************************************************************
Matt Marchant 2014 - 2017
http://trederia.blogspot.com

xygine - Zlib license.

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

//custom component which destroys its parent entity after a random amount of time

#ifndef TIMED_DESTRUCTION_HPP_
#define TIMED_DESTRUCTION_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Entity.hpp>

#include <SFML/System/Clock.hpp>

class TimedDestruction final : public xy::Component
{
public:
    TimedDestruction(xy::MessageBus& mb)
        : Component(mb, this)
    {
        m_timeout = static_cast<float>(xy::Util::Random::value(5, 10));
    }

    xy::Component::Type type() const override { return xy::Component::Type::Script; }
    void entityUpdate(xy::Entity& ent, float) override
    {
        if (m_clock.getElapsedTime().asSeconds() > m_timeout)
        {
            ent.destroy();
        }
    }

private:
    float m_timeout;
    sf::Clock m_clock;
};

#endif //TIMED_DESTRUCTION_HPP_
