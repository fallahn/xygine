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
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>

using namespace lm;

namespace
{
    const sf::Vector2f gravity(0.f, 1.f);
    const sf::Vector2f thrustX(3.6f, 0.f);
    const sf::Vector2f thrustUp(0.f, -3.2f);
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component(mb, this)
{

}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
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
    m_velocity.x *= 0.9999f;

    //move ship
    entity.move(m_velocity * dt);

    //TEMP kill when out of bounds - this
    //will eventually be handled by collision
    auto pos = entity.getWorldPosition();
    if (pos.x < 386 || pos.x> 1534 ||
        pos.y < -20 || pos.y > 1080.f)
    {
        entity.destroy();
        auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
        msg->type = LMEvent::PlayerDied;
    }
}

void PlayerController::setInput(sf::Uint8 input)
{
    m_inputFlags = input;
}