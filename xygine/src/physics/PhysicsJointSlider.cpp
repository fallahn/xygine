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

#include <xygine/physics/JointSlider.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

SliderJoint::SliderJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& direction)
{
    m_anchor = World::sfToBoxVec(worldAnchorPos);
    m_direction = World::sfToBoxVec(direction);

    setRigidBodyA(&bodyA);
}

//public
void SliderJoint::canCollide(bool collide)
{
    if (getJointAs<b2PrismaticJoint>() == nullptr)
    {
        m_definition.collideConnected = collide;
    }
}

bool SliderJoint::canCollide() const
{
    return m_definition.collideConnected;
}

float SliderJoint::getTranslation() const
{
    XY_ASSERT(getJointAs<b2PrismaticJoint>(), "Joint not yet created");
    return World::boxToSfFloat(getJointAs<b2PrismaticJoint>()->GetJointTranslation());
}

float SliderJoint::getSpeed() const
{
    XY_ASSERT(getJointAs<b2PrismaticJoint>(), "Joint not yet created");
    return World::boxToSfFloat(getJointAs<b2PrismaticJoint>()->GetJointSpeed());
}

void SliderJoint::limitEnabled(bool enabled)
{
    m_definition.enableLimit = enabled;
    auto joint = getJointAs<b2PrismaticJoint>();
    if (joint)
    {
        joint->EnableLimit(enabled);
    }
}

bool SliderJoint::limitEnabled() const
{
    return m_definition.enableLimit;
}

float SliderJoint::getLowerLimit() const
{
    return World::boxToSfFloat(m_definition.lowerTranslation);
}

float SliderJoint::getUpperLimit() const
{
    return World::boxToSfFloat(m_definition.upperTranslation);
}

void SliderJoint::setLimits(float upper, float lower)
{
    m_definition.upperTranslation = World::sfToBoxFloat(upper);
    m_definition.lowerTranslation = World::sfToBoxFloat(lower);

    auto joint = getJointAs<b2PrismaticJoint>();
    if (joint)
    {
        joint->SetLimits(m_definition.lowerTranslation, m_definition.upperTranslation);
    }
}

void SliderJoint::motorEnabled(bool enabled)
{
    m_definition.enableMotor = enabled;
    auto joint = getJointAs<b2PrismaticJoint>();
    if (joint)
    {
        joint->EnableMotor(enabled);
    }
}

bool SliderJoint::motorEnabled() const
{
    return m_definition.enableMotor;
}

void SliderJoint::setMotorSpeed(float speed)
{
    m_definition.motorSpeed = World::sfToBoxFloat(speed);
    auto joint = getJointAs<b2PrismaticJoint>();
    if (joint)
    {
        joint->SetMotorSpeed(m_definition.motorSpeed);
    }
}

float SliderJoint::getMotorSpeed() const
{
    return World::boxToSfFloat(m_definition.motorSpeed);
}

void SliderJoint::setMaxMotorForce(float force)
{
    m_definition.maxMotorForce = force;
    auto joint = getJointAs<b2PrismaticJoint>();
    if (joint)
    {
        joint->SetMaxMotorForce(force);
    }
}

float SliderJoint::getMaxMotorForce() const
{
    return m_definition.maxMotorForce;
}

//private
const b2JointDef* SliderJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body, m_anchor, m_direction);
    return static_cast<b2JointDef*>(&m_definition);
}