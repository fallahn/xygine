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

#include <PlatformPlayerController.hpp>

#include <xygine/Assert.hpp>
#include <xygine/Entity.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>

using namespace Plat;

namespace
{
    const float moveForce = 800.f;
    const float maxVelocity = moveForce * moveForce;
    const float rotationSpeed = 800.f;
    const float faceRotation = 45.f;

    enum Animation
    {
        Run  = 0,
        Idle = 1,
        Jump = 2
    };
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_body          (nullptr),
    m_model         (nullptr),
    m_lastInput     (0),
    m_faceLeft      (false),
    m_faceRight     (false)
{

}

//public
void PlayerController::entityUpdate(xy::Entity&, float dt)
{
    float rotation = m_model->getRotation(xy::Model::Axis::Z);
    if (m_faceLeft && rotation > -faceRotation)
    {
        m_model->rotate(xy::Model::Axis::Z, -rotationSpeed * dt * (1.f - (rotation / -faceRotation)));
    }
    else
    {
        m_faceLeft = false;
    }

    if (m_faceRight && rotation < faceRotation)
    {
        m_model->rotate(xy::Model::Axis::Z, rotationSpeed * dt * (1.f - (rotation / faceRotation)));
    }
    else
    {
        m_faceRight = false;
    }
    
    REPORT("Z Rotation", std::to_string(rotation));

    //const float speedRatio = std::max(0.f, xy::Util::Vector::lengthSquared(m_body->getLinearVelocity()) / maxVelocity);
    //REPORT("Velocity Ratio", std::to_string(speedRatio));
    //set animation speed based on ratio
    //m_model->setPlaybackRate((speedRatio > 0) ? speedRatio : 1.f);
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_body = entity.getComponent<xy::Physics::RigidBody>();
    XY_ASSERT(m_body, "Rigid body not found");

    m_model = entity.getComponent<xy::Model>();
    XY_ASSERT(m_model, "Model not found!");
    m_model->playAnimation(1, 0.1f);
}

void PlayerController::applyInput(sf::Uint8 input)
{
    //update physics
    float velocity = 0.f;
    if (input & Left)
    {
        velocity = -moveForce;
        m_faceLeft = true;
        if (m_model->getCurrentAnimation() == Animation::Idle)
        {
            m_model->playAnimation(Animation::Run, 0.1f);
        }
    }

    if (input & Right)
    {
        velocity = moveForce;
        m_faceRight = true;
        if (m_model->getCurrentAnimation() == Animation::Idle)
        {
            m_model->playAnimation(Animation::Run, 0.1f);
        }
    }

    
    const auto currVelocity = m_body->getLinearVelocity();
    const float velChange = velocity - currVelocity.x;
    const float force = m_body->getMass() * velChange;
    m_body->applyLinearImpulse({ force, 0.f }, m_body->getLocalCentre());

    if ((input & Jump) &&
        ((m_lastInput & Jump) == 0))
    {
        const float impulse = m_body->getMass() * 1000.f;
        m_body->applyLinearImpulse({ 0.f, -impulse }, m_body->getLocalCentre());

        //flap wings
        m_model->playAnimation(Animation::Jump, 0.2f);
    }

    //idle if still
    if (input == 0 && (m_model->getCurrentAnimation() == Animation::Run || xy::Util::Vector::lengthSquared(currVelocity) == 0))
    {
        m_model->playAnimation(Animation::Idle, 0.1f);
    }

    m_lastInput = input;
}

//private