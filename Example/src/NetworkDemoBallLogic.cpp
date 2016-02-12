/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <NetworkDemoBallLogic.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>

namespace
{
    const float speed = 200.f;
    const sf::Vector2f playArea(1920.f, 1080.f);
}

BallLogic::BallLogic(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_localBounds   (-10.f, -10.f, 20.f, 20.f),
    m_velocity      (0.f, 1.f)
{

}

//public
void BallLogic::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * speed * dt);

    auto pos = entity.getWorldPosition();
    if (pos.y < 0 || pos.y > playArea.y)
    {
        m_velocity = xy::Util::Vector::reflect(m_velocity, { 0.f, 1.f });
    }
    m_globalBounds = entity.getWorldTransform().transformRect(m_localBounds);
}

sf::FloatRect BallLogic::globalBounds() const
{
    return m_globalBounds;
}