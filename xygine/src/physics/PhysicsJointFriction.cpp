/*********************************************************************
© Matt Marchant 2014 - 2017
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

#include <xygine/physics/JointFriction.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

FrictionJoint::FrictionJoint(const RigidBody& rbA, const sf::Vector2f& anchor)
{
    m_anchor = World::sfToBoxVec(anchor);

    setRigidBodyA(&rbA);
}

//public
void FrictionJoint::canCollide(bool collide)
{
    if(getJointAs<b2FrictionJoint>() == nullptr)
        m_definition.collideConnected = collide;
}

bool FrictionJoint::canCollide() const
{
    return m_definition.collideConnected;
}

void FrictionJoint::setMaxFrictionForce(float force)
{
    XY_ASSERT(force >= 0, "Friction should be a positive value");
    m_definition.maxForce = force;
    auto joint = getJointAs<b2FrictionJoint>();
    if (joint)
    {
        joint->SetMaxForce(force);
    }
}

float FrictionJoint::getMaxFrictionForce() const
{
    return m_definition.maxForce;
}

void FrictionJoint::setMaxFrictionTorque(float torque)
{
    XY_ASSERT(torque >= 0, "Friction should be a positive value");
    m_definition.maxTorque = torque;
    auto joint = getJointAs<b2FrictionJoint>();
    if (joint)
    {
        joint->SetMaxTorque(torque);
    }
}

float FrictionJoint::getMaxFrictionTorque() const
{
    return m_definition.maxTorque;
}

//private
const b2JointDef* FrictionJoint::getDefinition()
{
    XY_ASSERT(getRigidBodyA() && getRigidBodyB(), "Don't forget to set your Bodies!");
    m_definition.Initialize(getRigidBodyA()->m_body, getRigidBodyB()->m_body, m_anchor);
    return static_cast<b2JointDef*>(&m_definition);
}