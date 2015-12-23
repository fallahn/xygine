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
#include <xygine/Assert.hpp>

#include <algorithm>

using namespace xy;
using namespace xy::Physics;

using namespace std::placeholders;

CollisionShape::CollisionShape()
    : m_fixture         (nullptr),
    m_removeCallbacks   (false)
{

}

CollisionShape::~CollisionShape()
{
    if (m_removeCallbacks)
    {
        World::m_instance->removeContactBeginCallback(m_beginCallbackIndex);
        World::m_instance->removeContactEndCallback(m_endCallbackIndex);
    }
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

void CollisionShape::setFilter(CollisionFilter filter)
{
    m_fixtureDef.filter.categoryBits = filter.categoryFlags;
    m_fixtureDef.filter.groupIndex = filter.groupIndex;
    m_fixtureDef.filter.maskBits = filter.maskFlags;

    if (m_fixture)
    {
        m_fixture->SetFilterData(m_fixtureDef.filter);
    }
}

CollisionFilter CollisionShape::getFilter() const
{
    CollisionFilter retVal;
    if (m_fixture)
    {
        auto filter = m_fixture->GetFilterData();
        retVal.categoryFlags = filter.categoryBits;
        retVal.groupIndex = filter.groupIndex;
        retVal.maskFlags = filter.maskBits;
        return retVal;
    }

    retVal.categoryFlags = m_fixtureDef.filter.categoryBits;
    retVal.groupIndex = m_fixtureDef.filter.groupIndex;
    retVal.maskFlags = m_fixtureDef.filter.maskBits;
    return retVal;
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

void CollisionShape::clearAffectors()
{
    m_activeAffectors.clear();
    m_areaAffectors.clear();
    m_constForceAffectors.clear();

    if (m_removeCallbacks)
    {
        World::m_instance->removeContactBeginCallback(m_beginCallbackIndex);
        World::m_instance->removeContactEndCallback(m_endCallbackIndex);
    }
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
            //store affectors for our body


            //store affectors for other body
            for (auto& a : m_areaAffectors)
            {
                if ((a.useCollisionMask() && a.getCollisionMask().passes(shapeB->getFilter()))
                    || !a.useCollisionMask())
                {
                    m_activeAffectors.emplace_back(std::make_pair(&a, static_cast<RigidBody*>(shapeB->m_fixture->GetBody()->GetUserData())));
                }
            }
        }
        else if (shapeB == this)
        {
            //remove affectors for our body


            //remove affectors for other body       
            for (auto& a : m_areaAffectors)
            {
                if ((a.useCollisionMask() && a.getCollisionMask().passes(shapeA->getFilter()))
                    || !a.useCollisionMask())
                {
                    m_activeAffectors.emplace_back(std::make_pair(&a, static_cast<RigidBody*>(shapeA->m_fixture->GetBody()->GetUserData())));
                }
            }
        }
    }
}

void CollisionShape::endContactCallback(Contact& contact)
{
    CollisionShape* shapeA = contact.getCollisionShapeA();
    CollisionShape* shapeB = contact.getCollisionShapeB();
    if (shapeA != shapeB)
    {
        if (shapeA == this)
        {
            //do stuff to ours


            //do stuff to other            
            for (auto& a : m_areaAffectors)
            {
                //hm, is this going to be an epic bottleneck?
                auto pair(std::make_pair(&a, static_cast<RigidBody*>(shapeB->m_fixture->GetBody()->GetUserData())));
                m_activeAffectors.erase(std::remove_if(m_activeAffectors.begin(), m_activeAffectors.end(),
                    [&pair](const AffectorPair& p)
                {
                    return (pair.first == p.first && pair.second == p.second);
                }), m_activeAffectors.end());
            }
        }
        else if (shapeB == this)
        {
            //do stuff to ours


            //do stuff to other            
            for (auto& a : m_areaAffectors)
            {
                auto pair(std::make_pair(&a, static_cast<RigidBody*>(shapeA->m_fixture->GetBody()->GetUserData())));
                m_activeAffectors.erase(std::remove_if(m_activeAffectors.begin(), m_activeAffectors.end(),
                    [&pair](const AffectorPair& p)
                {
                    return (pair.first == p.first && pair.second == p.second);
                }), m_activeAffectors.end());
            }
        }
    }
}

void CollisionShape::preSolveContactCallback(Contact& contact)
{

}

void CollisionShape::registerCallbacks()
{
    if (!m_areaAffectors.empty() /*|| other things !empty()*/)
    {
        World::ContactCallback cb = std::bind(&CollisionShape::beginContactCallback, this, _1);
        m_beginCallbackIndex = World::m_instance->addContactBeginCallback(cb);

        cb = std::bind(&CollisionShape::endContactCallback, this, _1);
        m_endCallbackIndex = World::m_instance->addContactEndCallback(cb);

        m_removeCallbacks = true;
    }
}