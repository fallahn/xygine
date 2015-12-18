/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/physics/JointMotor.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/World.hpp>

using namespace xy;
using namespace xy::Physics;

MotorJoint::MotorJoint(const RigidBody& rbA)
{
    setRigidBodyA(&rbA);
}

//public
void MotorJoint::canCollide(bool collide)
{
    if (getJointAs<b2MotorJoint>() == nullptr)
    {
        m_definition.collideConnected = collide;
    }
}

bool MotorJoint::canCollide() const
{
    return m_definition.collideConnected;
}

void MotorJoint::setTargetLinearOffset(const sf::Vector2f& offset)
{
    XY_ASSERT(getJointAs<b2MotorJoint>(), "Not enough bodies attached");
    getJointAs<b2MotorJoint>()->SetLinearOffset(World::sfToBoxVec(offset));
}

sf::Vector2f MotorJoint::getTargetLinearOffset() const
{
    XY_ASSERT(getJointAs<b2MotorJoint>(), "Not enough bodies attached");
    return World::boxToSfVec(getJointAs<b2MotorJoint>()->GetLinearOffset());
}

void MotorJoint::setTargetAngularOffset(float angle)
{
    XY_ASSERT(getJointAs<b2MotorJoint>(), "Not enough bodies attached");
    getJointAs<b2MotorJoint>()->SetAngularOffset(World::sfToBoxAngle(angle));
}

float MotorJoint::getTargetAngularOffset() const
{
    XY_ASSERT(getJointAs<b2MotorJoint>(), "Not enough bodies attached");
    return World::boxToSfAngle(getJointAs<b2MotorJoint>()->GetAngularOffset());
}

void MotorJoint::setMaxForce(float force)
{
    XY_ASSERT(force >= 0, "Force must be a positive value");
    m_definition.maxForce = force;
    auto joint = getJointAs<b2MotorJoint>();
    if (joint)
    {
        joint->SetMaxForce(force);
    }
}

float MotorJoint::getMaxForce() const
{
    return m_definition.maxForce;
}

void MotorJoint::setMaxTorque(float torque)
{
    XY_ASSERT(torque >= 0, "Torque must be a positive value");
    m_definition.maxTorque = torque;
    auto joint = getJointAs<b2MotorJoint>();
    if (joint)
    {
        joint->SetMaxTorque(torque);
    }
}

float MotorJoint::getMaxTorque() const
{
    return m_definition.maxTorque;
}

void MotorJoint::setCorrectionFactor(float factor)
{
    XY_ASSERT(factor >= 0 && factor <= 1, "Correction factor must be range 0-1");
    m_definition.correctionFactor = factor;
    auto joint = getJointAs<b2MotorJoint>();
    if (joint)
    {
        joint->SetCorrectionFactor(factor);
    }
}

float MotorJoint::getCorrectionFactor() const
{
    return m_definition.correctionFactor;
}

//private
const b2JointDef* MotorJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body);
    return static_cast<b2JointDef*>(&m_definition);
}