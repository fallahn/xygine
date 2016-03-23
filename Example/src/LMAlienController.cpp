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

#include <LMAlienController.hpp>
#include <LMCollisionComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>

using namespace lm;

namespace
{
    const float maxVelocity = 100.f;
}

AlienController::AlienController(xy::MessageBus& mb, const sf::FloatRect& playArea)
    : xy::Component (mb, this),
    m_playArea      (playArea),
    m_speed         (xy::Util::Random::value(0.f, maxVelocity)),
    m_entity        (nullptr)
{
    m_velocity.x = xy::Util::Random::value(-maxVelocity, maxVelocity);
    m_velocity.y = xy::Util::Random::value(-(maxVelocity / 2.f), (maxVelocity / 2.f));
    m_velocity = xy::Util::Vector::normalise(m_velocity);
}

//public
void AlienController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * m_speed * dt);

    auto position = entity.getPosition();
    if (position.y < m_playArea.top)
    {
        m_velocity = xy::Util::Vector::reflect(m_velocity, { 0.f, 1.f });
    }
    else if (position.y > m_playArea.top + m_playArea.height)
    {
        m_velocity = xy::Util::Vector::reflect(m_velocity, { 0.f, -1.f });
    }

    if (position.x < m_playArea.left)
    {
        position.x += m_playArea.width;
        entity.setPosition(position);
    }
    else if (position.x > m_playArea.left + m_playArea.width)
    {
        position.x -= m_playArea.width;
        entity.setPosition(position);
    }
}

void AlienController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void AlienController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {
    default: break;
    case CollisionComponent::ID::Bullet:
    //case CollisionComponent::ID::Player:
    {
        auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
        msg->type = LMEvent::AlienDied;
        m_entity->destroy();
    }
        break;
    }
}

//private