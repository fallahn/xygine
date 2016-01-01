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

#include <xygine/physics/AffectorAreaForce.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Util.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

AreaForceAffector::AreaForceAffector(const sf::Vector2f& force, float torque, bool wake)
    : m_force       (force),
    m_torque        (torque),
    m_wake          (wake),
    m_linearDrag    (0.f),
    m_angularDrag   (0.f),
    m_useMask       (false)
{

}

void AreaForceAffector::apply(RigidBody* body)
{   
    XY_ASSERT(body, "body is nullptr");
    body->applyForceToCentre(m_force - (body->getLinearVelocity() * m_linearDrag), m_wake);
    body->applyTorque(m_torque - (body->getAngularVelocity() * m_angularDrag), m_wake);
}

void AreaForceAffector::setLinearDrag(float drag)
{
    XY_ASSERT(drag >= 0.f && drag <= 1.f, "drag must be in range 0-1");
    m_linearDrag = drag;
}

void AreaForceAffector::setAngularDrag(float drag)
{
    XY_ASSERT(drag >= 0.f && drag <= 1.f, "drag must be in range 0-1");
    m_angularDrag = drag;
}