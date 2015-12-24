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

#include <xygine/physics/AffectorPointForce.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Util.hpp>

using namespace xy;
using namespace xy::Physics;

PointForceAffector::PointForceAffector(float magnitude, bool wake)
    : m_targetPoint     (Centroid::CollisionShape),
    m_sourcePoint       (Centroid::CollisionShape),
    m_magnitude         (magnitude),
    m_wake              (wake),
    m_linearDrag        (0.f),
    m_angularDrag       (0.f),
    m_useCollisionMask  (false)
{}

void PointForceAffector::apply(RigidBody* body)
{
    XY_ASSERT(body, "body is nullptr");
    body->applyForceToCentre(m_force - (body->getLinearVelocity() * m_linearDrag), m_wake);
}

void PointForceAffector::setLinearDrag(float drag)
{
    XY_ASSERT(drag >= 0.f && drag <= 1.f, "drag must be in range 0-1");
    m_linearDrag = drag;
}

void PointForceAffector::setAngularDrag(float drag)
{
    XY_ASSERT(drag >= 0.f && drag <= 1.f, "drag must be in range 0-1");
    m_angularDrag = drag;
}

//private
void PointForceAffector::calcForce(CollisionShape* source, CollisionShape* dest)
{
    XY_ASSERT(source && dest, "shape must not be nullptr");

    //dest - source
    sf::Vector2f destPos;
    if (m_targetPoint == Centroid::CollisionShape)
    {
        destPos = World::boxToSfVec(dest->m_fixture->GetAABB(0).GetCenter() + dest->m_fixture->GetBody()->GetWorldCenter());
    }
    else
    {
        destPos = static_cast<RigidBody*>(dest->m_fixture->GetBody()->GetUserData())->getWorldCentre();
    }

    sf::Vector2f sourcePos;
    if (m_sourcePoint == Centroid::CollisionShape)
    {
        sourcePos = World::boxToSfVec(source->m_fixture->GetAABB(0).GetCenter() + source->m_fixture->GetBody()->GetWorldCenter());
    }
    else
    {
        sourcePos = static_cast<RigidBody*>(source->m_fixture->GetBody()->GetUserData())->getWorldCentre();
    }

    m_force = Util::Vector::normalise(destPos - sourcePos) * m_magnitude;
}