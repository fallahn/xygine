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

#include <LMPlayerController.hpp>
#include <LMCollisionComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Rectangle.hpp>
#include <xygine/Reports.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f gravity(0.f, 1.5f);
    const sf::Vector2f thrustX(3.6f, 0.f);
    const sf::Vector2f thrustUp(0.f, -3.8f);
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_inputFlags    (0),
    m_entity        (nullptr),
    m_carrying      (false),
    m_pickupTime    (0.f)
{
    m_velocity.y = 2.f;

    updateState = std::bind(&PlayerController::flyingState, this, _1, _2);
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    updateState(entity, dt);
    REPORT("Current Speed", std::to_string(xy::Util::Vector::lengthSquared(m_velocity)));
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void PlayerController::setInput(sf::Uint8 input)
{
    m_inputFlags = input;
}

void PlayerController::destroy()
{
    Component::destroy();
    auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
    msg->type = LMEvent::PlayerDied;
}

void PlayerController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {
    case CollisionComponent::ID::Alien:
        //m_entity->destroy();
        break;
    case CollisionComponent::ID::Bounds:
    {
        auto manifold = getManifold(cc->globalBounds());
        sf::Vector2f normal(manifold.x, manifold.y);

        m_entity->move(normal * manifold.z);
        m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
        m_velocity *= 0.65f; //some damping
    }
        break;
    case CollisionComponent::ID::Mothership:
        //if carrying drop human, raise message
        if (m_carrying && xy::Util::Vector::lengthSquared(m_velocity) < 25000)
        {
            //we want to be moving slowly enough, and fully contained in mothership area
            if (xy::Util::Rectangle::contains(cc->globalBounds(), m_entity->globalBounds()))
            {
                m_carrying = false;
                auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
                msg->type = LMEvent::HumanRescued;
            }
        }
        break;
    case CollisionComponent::ID::Tower:
    {
        auto manifold = getManifold(cc->globalBounds());
        if (manifold.y != 0)
        {
            //we're on top TODO measure velocity an assplode if too fast
            auto normal = sf::Vector2f(manifold.x, manifold.y);
            m_entity->move(normal * manifold.z);
            m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
            m_velocity.y = -0.1f; //anti-jiggle

            if (!m_carrying)
            {
                //stop and pickup
                m_pickupTime = 5.f;
                updateState = std::bind(&PlayerController::landedState, this, _1, _2);
            }
        }
        else
        {
            //crashded :(
            m_entity->destroy();
        }
    }
        break;
    default: break;
    }
}

sf::Vector3f PlayerController::getManifold(const sf::FloatRect& worldRect)
{
    sf::FloatRect overlap;
    sf::FloatRect playerBounds = m_entity->getComponent<CollisionComponent>()->globalBounds();

    //we know we intersect, but we want the overlap
    worldRect.intersects(playerBounds, overlap);
    auto collisionNormal = sf::Vector2f(worldRect.left + (worldRect.width / 2.f), worldRect.top + (worldRect.height / 2.f)) - m_entity->getPosition();

    sf::Vector3f manifold;

    if (overlap.width < overlap.height)
    {
        manifold.x = (collisionNormal.x < 0) ? 1.f : -1.f;
        manifold.z = overlap.width;
    }
    else
    {
        manifold.y = (collisionNormal.y < 0) ? 1.f : -1.f;
        manifold.z = overlap.height;
    }

    return manifold;
}

void PlayerController::flyingState(xy::Entity& entity, float dt)
{
    //apply gravity every frame
    m_velocity += gravity;

    //check input and apply forces
    if (m_inputFlags & LMInputFlags::SteerRight)
    {
        m_velocity += thrustX;
    }
    if (m_inputFlags & LMInputFlags::SteerLeft)
    {
        m_velocity -= thrustX;
    }
    if (m_inputFlags & LMInputFlags::Thrust)
    {
        m_velocity += thrustUp;
    }


    //apply drag
    m_velocity.x *= 0.999f;

    //move ship
    entity.move(m_velocity * dt);
}

void PlayerController::landedState(xy::Entity& entity, float dt)
{
    m_pickupTime -= dt;
    if (m_pickupTime < 0)
    {
        m_velocity = { 0.f, -82.5f };
        updateState = std::bind(&PlayerController::flyingState, this, _1, _2);
        m_carrying = true;
    }
}