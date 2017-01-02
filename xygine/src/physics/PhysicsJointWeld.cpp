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

#include <xygine/physics/JointWeld.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

WeldJoint::WeldJoint(const RigidBody& rbA, const sf::Vector2f& worldWeldPos)
{
    setRigidBodyA(&rbA);

    m_anchor = World::sfToBoxVec(worldWeldPos);
}

//public
void WeldJoint::canCollide(bool collide)
{
    if (getJointAs<b2WeldJoint>() == nullptr)
    {
        m_definition.collideConnected = collide;
    }
}

bool WeldJoint::canCollide() const
{
    return m_definition.collideConnected;
}

float WeldJoint::getReferenceAngle() const
{
    return m_definition.referenceAngle;
}

void WeldJoint::setFrequency(float freq)
{
    XY_ASSERT(freq >= 0, "Frequency must be a positive value");
    m_definition.frequencyHz = freq;
    auto joint = getJointAs<b2WeldJoint>();
    if (joint)
    {
        joint->SetFrequency(freq);
    }
}

float WeldJoint::getFrequency() const
{
    return m_definition.frequencyHz;
}

void WeldJoint::setDampingRatio(float ratio)
{
    XY_ASSERT(ratio >= 0, "Ratio must be a positive value");
    auto joint = getJointAs<b2WeldJoint>();
    if(joint)
    {
        joint->SetDampingRatio(ratio);
    }
}

float WeldJoint::getDampingRatio() const
{
    return m_definition.dampingRatio;
}

//private
b2JointDef* WeldJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body, m_anchor);
    return static_cast<b2JointDef*>(&m_definition);
}