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

#include <xygine/physics/AffectorConstantForce.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

ConstantForceAffector::ConstantForceAffector(const sf::Vector2f& force, float torque, bool wake)
    : m_force   (force),
    m_torque    (torque),
    m_wake      (wake)
{

}

//public
void ConstantForceAffector::operator()(RigidBody* body)
{
    XY_ASSERT(body, "can't apply a force to null bodies");
    body->applyForceToCentre(m_force, m_wake);
    body->applyTorque(m_torque, m_wake);
}

void ConstantForceAffector::setForce(const sf::Vector2f& force)
{
    m_force = force;
}

const sf::Vector2f& ConstantForceAffector::getForce() const
{
    return m_force;
}

void ConstantForceAffector::setTorque(float torque)
{
    m_torque = torque;
}

float ConstantForceAffector::getTorque() const
{
    return m_torque;
}

void ConstantForceAffector::setWake(bool wake)
{
    m_wake = wake;
}

bool ConstantForceAffector::getWake() const
{
    return m_wake;
}