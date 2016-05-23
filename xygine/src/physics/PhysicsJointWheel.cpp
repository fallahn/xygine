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

#include <xygine/physics/JointWheel.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

WheelJoint::WheelJoint(const RigidBody& rbA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& axis)
{
    m_anchor = World::sfToBoxVec(worldAnchorPos);
    m_axis = World::sfToBoxVec(axis);

    setRigidBodyA(&rbA);
}

//public
void WheelJoint::canCollide(bool collide)
{
    if (getJointAs<b2WheelJoint>() == nullptr)
    {
        m_definition.collideConnected = collide;
    }
}

bool WheelJoint::canCollide() const
{
    return m_definition.collideConnected;
}

float WheelJoint::getTranslation() const
{
    XY_ASSERT(getJointAs<b2WheelJoint>(), "Joint not yet added to rigidbody");
    return World::boxToSfFloat(getJointAs<b2WheelJoint>()->GetJointTranslation());
}

float WheelJoint::getSpeed() const
{
    XY_ASSERT(getJointAs<b2WheelJoint>(), "Joint not yet added to rigidbody");
    return World::boxToSfFloat(getJointAs<b2WheelJoint>()->GetJointAngularSpeed());
}

void WheelJoint::motorEnabled(bool enabled)
{
    m_definition.enableMotor = enabled;
    auto joint = getJointAs<b2WheelJoint>();
    if (joint)
    {
        joint->EnableMotor(enabled);
    }
}

bool WheelJoint::motorEnabled() const
{
    return m_definition.enableMotor;
}

void WheelJoint::setMotorSpeed(float speed)
{
    m_definition.motorSpeed = World::sfToBoxAngle(speed);
    auto joint = getJointAs<b2WheelJoint>();
    if (joint)
    {
        joint->SetMotorSpeed(m_definition.motorSpeed);
    }
}

float WheelJoint::getMotorSpeed() const
{
    return World::boxToSfAngle(m_definition.motorSpeed);
}

void WheelJoint::setMaxMotorTorque(float torque)
{
    m_definition.maxMotorTorque = torque;
    
    auto joint = getJointAs<b2WheelJoint>();
    if (joint)
    {
        joint->SetMaxMotorTorque(torque);
    }
}

float WheelJoint::getMaxMotorTorque() const
{
    return m_definition.maxMotorTorque;
}

void WheelJoint::setSpringFrequency(float frequency)
{
    XY_ASSERT(frequency >= 0, "Spring frequency must be a positive value");
    m_definition.frequencyHz = frequency;

    auto joint = getJointAs<b2WheelJoint>();
    if (joint)
    {
        joint->SetSpringFrequencyHz(frequency);
    }
}

float WheelJoint::getSpringFrequency() const
{
    return m_definition.frequencyHz;
}

void WheelJoint::setSpringDampingRatio(float ratio)
{
    XY_ASSERT(ratio >= 0, "Damping ratio must be greater than zero");
    m_definition.dampingRatio = ratio;

    auto joint = getJointAs<b2WheelJoint>();
    if (joint)
    {
        joint->SetSpringDampingRatio(ratio);
    }
}

float WheelJoint::getSpringDampingRatio() const
{
    return m_definition.dampingRatio;
}

//private
const b2JointDef* WheelJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body, m_anchor, m_axis);
    return static_cast<b2JointDef*>(&m_definition);
}