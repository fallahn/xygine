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

#include <xygine/physics/JointDistance.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

DistanceJoint::DistanceJoint(const RigidBody& rbA, const sf::Vector2f& worldPosA, const sf::Vector2f& worldPosB)
{
    m_anchorA = World::sfToBoxVec(worldPosA);
    m_anchorB = World::sfToBoxVec(worldPosB);

    setRigidBodyA(&rbA);
}

//public
float DistanceJoint::getLength() const
{
    auto joint = getJointAs<b2DistanceJoint>();
    return (joint) ? World::boxToSfFloat(joint->GetLength()) : World::boxToSfFloat(m_definition.length);
}

void DistanceJoint::setFrequency(float freq)
{
    XY_ASSERT(freq > 0, "Frequency must be greater than 0");
    m_definition.frequencyHz = freq;

    auto joint = getJointAs<b2DistanceJoint>();
    if (joint)
    {
        joint->SetFrequency(freq);
    }
}

float DistanceJoint::getFrequency() const
{
    return m_definition.frequencyHz;
}

void DistanceJoint::setDampingRatio(float ratio)
{
    XY_ASSERT(ratio >= 0, "Damping ration should be positive");
    m_definition.dampingRatio = ratio;
    auto joint = getJointAs<b2DistanceJoint>();
    if (joint)
    {
        joint->SetDampingRatio(ratio);
    }
}

float DistanceJoint::getDampingRatio() const
{
    return m_definition.dampingRatio;
}

//private
const b2JointDef* DistanceJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    
    //apply def settings first
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body, m_anchorA, m_anchorB);

    return static_cast<b2JointDef*>(&m_definition);
}