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

#include <RacingDemoPlayer.hpp>
#include <RacingDemoTrack.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Window/Keyboard.hpp>

namespace
{
    const float MAX_SPEED = 12000.f;
    const float acceleration = MAX_SPEED / 5.f;
    const float breaking = -MAX_SPEED;
    const float offroadDecel = -MAX_SPEED / 2.f;
    const float offroadMaxSpeed = MAX_SPEED / 4.f;
    const float centrifugal = 0.35f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : Component(mb, this),
    m_speed(0.f),
    m_offset(0.f),
    m_depth(0.f)
{

}


//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    //check input
    auto speedRatio = m_speed / MAX_SPEED;
    auto deltaSpeed = speedRatio * dt;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        moveLeft(deltaSpeed);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        moveRight(deltaSpeed);
    }

    //TODO get current track segment (from messagebus) and apply centrifugal force
    //adaptMovement((deltaSpeed * speedRatio * playerSegment.getCurve() * centrifugal));

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        accelerate(acceleration * dt);
    }
    else
    {
        accelerate(-acceleration * dt);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        accelerate(breaking * dt);
    }

    //check if car is off road and slow down
    if (m_offset < -1 || m_offset > 1)
    {
        if (m_speed > offroadMaxSpeed)
        {
            accelerate(offroadDecel * dt);
        }
        //TODO check for scenery collision
    }

    //apply limits on speed and offset
    m_speed = std::max(0.f, std::min(m_speed, MAX_SPEED));
    m_offset = std::max(-2.f, std::min(m_offset, 2.f));

    //command track to update position
    xy::Command cmd;
    cmd.category = RacingCommandIDs::TrackEnt;
    cmd.action = [this](xy::Entity& e, float delta)
    {
        e.getComponent<Track>()->updatePosition(m_speed * delta, m_offset);
    };
    entity.getScene()->sendCommand(cmd);

    //TODO update entity drawable
    
}

void PlayerController::setSpeed(float speed)
{
    m_speed = speed;
}

float PlayerController::getSpeed() const
{
    return m_speed;
}

void PlayerController::setOffset(float offset)
{
    m_offset = offset;
}

float PlayerController::getOffset() const
{
    return m_offset;
}

void PlayerController::setDepth(float depth)
{
    m_depth = depth;
}

float PlayerController::getDepth() const
{
    return m_depth;
}

//private
void PlayerController::moveLeft(float amount)
{
    m_offset -= amount;
}

void PlayerController::moveRight(float amount)
{
    m_offset += amount;
}

void PlayerController::adaptMovement(float amount)
{
    moveLeft(amount);
}

void PlayerController::accelerate(float amount)
{
    m_speed += amount;
}