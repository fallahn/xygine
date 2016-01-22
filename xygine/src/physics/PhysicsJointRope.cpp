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

#include <xygine/physics/JointRope.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

RopeJoint::RopeJoint(const RigidBody& rbA, const sf::Vector2f& worldAnchorPosA, const sf::Vector2f& worldAnchorPosB)
{
    setRigidBodyA(&rbA);

    m_worldAnchorA = World::sfToBoxVec(worldAnchorPosA);
    m_worldAnchorB = World::sfToBoxVec(worldAnchorPosB);

    m_definition.maxLength = (m_worldAnchorB - m_worldAnchorA).Length();
}

//public
void RopeJoint::canCollide(bool collide)
{
    if (getJointAs<b2RopeJoint>() == nullptr)
    {
        m_definition.collideConnected = collide;
    }
}

bool RopeJoint::canCollide() const
{
    return m_definition.collideConnected;
}

void RopeJoint::setMaximumLength(float length)
{
    if (getJointAs<b2RopeJoint>() == nullptr)
    {
        m_definition.maxLength = World::sfToBoxFloat(length);
    }
}
float RopeJoint::getMaximumLength() const
{
    return World::boxToSfFloat(m_definition.maxLength);
}

//private
b2JointDef* RopeJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.bodyA = getRigidBodyA()->m_body;
    m_definition.bodyB = getRigidBodyB()->m_body;

    m_definition.localAnchorA = m_definition.bodyA->GetLocalPoint(m_worldAnchorA);
    m_definition.localAnchorB = m_definition.bodyB->GetLocalPoint(m_worldAnchorB);

    return static_cast<b2JointDef*>(&m_definition);
}