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
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Reports.hpp>

namespace
{
    float speed = 800.f;
    const sf::Vector2f playArea(1920.f, 1080.f);
    const std::size_t deltaReservation = 240;
}

using namespace NetDemo;

BallLogic::BallLogic(xy::MessageBus& mb)
    : xy::Component (mb, this),
    //m_localBounds   (-10.f, -10.f, 20.f, 20.f),
    m_velocity      (xy::Util::Random::value(-1.f, 1.f), xy::Util::Random::value(-1.f, 1.f)),
    m_stepCount     (0u),
    m_entity        (nullptr)
{
    m_velocity = xy::Util::Vector::normalise(m_velocity);
    m_deltaHistory.reserve(deltaReservation);
}

//public
void BallLogic::entityUpdate(xy::Entity& entity, float dt)
{
    m_stepCount++;
    m_deltaHistory.push_back(dt);
    
    entity.move(m_velocity * speed * dt);

    auto bounds = entity.globalBounds();
    for (const auto e : m_collisionObjects)
    {
        sf::FloatRect collision;
        if (bounds.intersects(e->globalBounds(), collision))
        {
            auto normal = e->getPosition() - entity.getPosition();
            resolveCollision(collision, normal, entity);
            break; //only one collision at a time
        }
    }


    //speed += 10 * dt;
    //REPORT("Speed", std::to_string(speed));

    //check score area, destroy / message if true
    auto position = entity.getPosition();
    if (position.x < 0)
    {
        //player 2 score
        auto msg = sendMessage<PongEvent>(NetMessageId::PongMessage);
        msg->type = PongEvent::PlayerTwoScored;
        killBall(entity);
    }
    else if (position.x > 1920)
    {
        //player 1 score
        auto msg = sendMessage<PongEvent>(NetMessageId::PongMessage);
        msg->type = PongEvent::PlayerOneScored;
        killBall(entity);
    }
}

void BallLogic::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void BallLogic::setCollisionObjects(const std::vector<xy::Entity*>& objs)
{
    m_collisionObjects = objs;
}

sf::Vector2f BallLogic::getPosition() const
{
    XY_ASSERT(m_entity, "Entity is nullptr");
    return m_entity->getPosition();
}

void BallLogic::reconcile(const sf::Vector2f& position, const sf::Vector2f& velocity, sf::Uint32 stepCount)
{
    auto destCount = m_stepCount;
    m_stepCount = stepCount;
    m_velocity = velocity;
    m_entity->setPosition(position);

    while (m_stepCount < destCount)
    {
        entityUpdate(*m_entity, m_deltaHistory[m_stepCount - stepCount]);
    }

    std::vector<float> newVec;
    std::swap(newVec, m_deltaHistory);
    m_deltaHistory.reserve(deltaReservation);
}

//private
void BallLogic::resolveCollision(const sf::FloatRect& intersection, const sf::Vector2f& collisionNormal, xy::Entity& entity)
{
    sf::Vector2f normal;
    float penetration = 0.f;

    if (intersection.width < intersection.height)
    {
        normal.x = (collisionNormal.x < 0) ? 1.f : -1.f;
        penetration = intersection.width;
    }
    else
    {
        normal.y = (collisionNormal.y < 0) ? 1.f : -1.f;
        penetration = intersection.height;
    }

    entity.move(normal * penetration);
    //REPORT("Normal", std::to_string(normal.x) + ", " + std::to_string(normal.y));
    m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
}

void BallLogic::killBall(xy::Entity& entity)
{
    entity.destroy();
    auto msg = sendMessage<PongEvent>(NetMessageId::PongMessage);
    msg->type = PongEvent::BallDestroyed;
}