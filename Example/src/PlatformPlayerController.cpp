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

#include <PlatformPlayerController.hpp>

#include <xygine/Assert.hpp>
#include <xygine/Entity.hpp>
#include <xygine/physics/RigidBody.hpp>

using namespace Plat;

namespace
{
    const float moveForce = 600.f;
    const float moveImpulse = 420.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_body          (nullptr),
    m_lastInput     (0)
{

}

//public
void PlayerController::entityUpdate(xy::Entity&, float)
{

}

void PlayerController::onStart(xy::Entity& entity)
{
    m_body = entity.getComponent<xy::Physics::RigidBody>();
    XY_ASSERT(m_body, "Rigid body not found");
}

void PlayerController::applyInput(sf::Uint8 input)
{
    if (input & Left)
    {
        m_body->applyForceToCentre({ -moveForce, 0.f });
        if ((m_lastInput & Left) == 0)
        {
            m_body->applyLinearImpulse({ -moveImpulse, 0.f }, m_body->getLocalCentre());
        }
    }

    if (input & Right)
    {
        m_body->applyForceToCentre({ moveForce, 0.f });
        if ((m_lastInput & Right) == 0)
        {
            m_body->applyLinearImpulse({ moveImpulse, 0.f }, m_body->getLocalCentre());
        }
    }

    if ((input & Jump) &&
        ((m_lastInput & Jump) == 0))
    {
        const float impulse = m_body->getMass() * 1000.f;
        m_body->applyLinearImpulse({ 0.f, -impulse }, m_body->getLocalCentre());
    }
    m_lastInput = input;
}

//private