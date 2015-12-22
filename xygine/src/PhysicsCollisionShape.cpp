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

#include <xygine/physics/CollisionShape.hpp>
#include <xygine/physics/Contact.hpp>
#include <xygine/physics/World.hpp>
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

using namespace std::placeholders;

CollisionShape::CollisionShape()
    : m_fixture(nullptr)
{

}

//public
void CollisionShape::setFriction(float friction)
{
    XY_ASSERT(friction >= 0, "Friction value must be a positive value");

    m_fixtureDef.friction = friction;
    if (m_fixture)
    {
        m_fixture->SetFriction(friction);
    }
}

void CollisionShape::setRestitution(float restitution)
{
    XY_ASSERT(restitution >= 0 && restitution <= 1, "Restitution value should be between 0 and 1");

    m_fixtureDef.restitution = restitution;
    if (m_fixture)
    {
        m_fixture->SetRestitution(restitution);
    }
}

void CollisionShape::setDensity(float density)
{
    XY_ASSERT(density >= 0, "Density value must be a positive value");

    m_fixtureDef.density = density;
    if (m_fixture)
    {
        m_fixture->SetDensity(density);
        m_fixture->GetBody()->ResetMassData();
    }
}

void CollisionShape::setIsSensor(bool sensor)
{
    m_fixtureDef.isSensor = sensor;
    if (m_fixture)
    {
        m_fixture->SetSensor(sensor);
    }
}

void CollisionShape::addFilter(CollisionFilter filter)
{
    m_fixtureDef.filter.categoryBits = filter.categoryFlags;
    m_fixtureDef.filter.groupIndex = filter.groupIndex;
    m_fixtureDef.filter.maskBits = filter.maskFlags;

    if (m_fixture)
    {
        m_fixture->SetFilterData(m_fixtureDef.filter);
    }
}

void CollisionShape::destroy()
{
    if (m_fixture)
    {
        m_fixture->GetBody()->DestroyFixture(m_fixture);
        destructionCallback(this);
    }
}

void CollisionShape::addAffector(const ConstantForceAffector& fa)
{
    m_constForceAffectors.push_back(fa);
}

void CollisionShape::addAffector(const AreaForceAffector& fa)
{
    m_areaAffectors.push_back(fa);
}
//private
void CollisionShape::beginContactCallback(Contact& contact)
{
    if (!contact.touching()) return;

    CollisionShape* shapeA = contact.getCollisionShapeA();
    CollisionShape* shapeB = contact.getCollisionShapeB();
    if (shapeA != shapeB)
    {
        if (shapeA == this)
        {
            //do stuff to this


            //do stuff to other            
            for (auto& a : m_areaAffectors)
            {
                //TODO check masks for affectors
                a(static_cast<RigidBody*>(shapeB->m_fixture->GetBody()->GetUserData()));
            }
        }
        else if (shapeB == this)
        {
            //do stuff to this


            //do stuff to other            
            for (auto& a : m_areaAffectors)
            {
                //TODO check masks for affectors
                a(static_cast<RigidBody*>(shapeA->m_fixture->GetBody()->GetUserData()));
            }
        }
    }
}

void CollisionShape::preSolveContactCallback(Contact& contact)
{

}

void CollisionShape::registerCallbacks()
{
    if (!m_areaAffectors.empty())
    {
        World::ContactCallback cb = std::bind(&CollisionShape::beginContactCallback, this, _1);
        World::m_instance->addContactBeginCallback(cb);
    }
}