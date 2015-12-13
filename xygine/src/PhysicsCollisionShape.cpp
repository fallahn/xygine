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
#include <xygine/Assert.hpp>

using namespace xy;
using namespace xy::Physics;

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