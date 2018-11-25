/*********************************************************************
(c) Matt Marchant 2017 - 2018
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#include "PhysicsSystem.hpp"

#include <xyginext/core/Log.hpp>

PhysicsObject::PhysicsObject()
    : m_type    (Type::None),
    m_body      (nullptr),
    m_shapes    (),
    m_shapeCount(0),
    m_system    (nullptr)
{
    for (auto& s : m_shapes)
    {
        s = nullptr;
    }
}

PhysicsObject::~PhysicsObject()
{
    if (m_system)
    {
        m_system->removeObject(*this);
    }
}

PhysicsObject::PhysicsObject(PhysicsObject&& other) noexcept
    : m_type    (Type::None),
    m_body      (nullptr),
    m_shapes    (),
    m_shapeCount(0),
    m_system    (nullptr)
{
    m_type = other.m_type;
    other.m_type = Type::None;

    m_body = other.m_body;
    other.m_body = nullptr;

    m_shapeCount = other.m_shapeCount;
    other.m_shapeCount = 0;

    for (auto i = 0u; i < m_shapes.size(); ++i)
    {
        m_shapes[i] = other.m_shapes[i];
        other.m_shapes[i] = nullptr;
    }

    m_system = other.m_system;
    other.m_system = nullptr;
}

PhysicsObject& PhysicsObject::operator=(PhysicsObject&& other) noexcept
{
    if (&other != this)
    {
        m_type = other.m_type;
        other.m_type = Type::None;

        m_body = other.m_body;
        other.m_body = nullptr;

        m_shapeCount = other.m_shapeCount;
        other.m_shapeCount = 0;

        for (auto i = 0u; i < m_shapes.size(); ++i)
        {
            m_shapes[i] = other.m_shapes[i];
            other.m_shapes[i] = nullptr;
        }

        m_system = other.m_system;
        other.m_system = nullptr;
    }

    return *this;
}

//public
bool PhysicsObject::awake() const
{
    return (m_body && !cpBodyIsSleeping(m_body));
}

cpShape* PhysicsObject::addCircleShape(const ShapeProperties& properties, float radius, sf::Vector2f offset)
{
    XY_ASSERT(m_system && m_body, "Component not initialised!");
    XY_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    m_shapes[m_shapeCount] = cpSpaceAddShape(m_system->m_space, cpCircleShapeNew(m_body, Convert::toPhysFloat(radius), Convert::toPhysVec(offset)));
    auto shape = m_shapes[m_shapeCount++];
    applyProperties(properties, shape);

    return shape;
}

cpShape* PhysicsObject::addLineShape(const ShapeProperties& properties, sf::Vector2f start, sf::Vector2f end)
{
    XY_ASSERT(m_system && m_body, "Component not initialised!");
    XY_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    m_shapes[m_shapeCount] = cpSpaceAddShape(m_system->m_space, cpSegmentShapeNew(m_body, Convert::toPhysVec(start), Convert::toPhysVec(end), 0.02));
    auto shape = m_shapes[m_shapeCount++];
    applyProperties(properties, shape);

    return shape;
}

cpShape* PhysicsObject::addPolygonShape(const ShapeProperties& properties, const std::vector<sf::Vector2f>& points)
{
    XY_ASSERT(m_system && m_body, "Component not initialised!");
    XY_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    std::vector<cpVect> converted(points.size());
    for (auto i = 0u; i < points.size(); ++i)
    {
        converted[i] = Convert::toPhysVec(points[i]);
    }

    cpTransform tx;
    tx.a = 1.f;
    tx.b = 1.f;
    tx.c = 1.f;
    tx.d = 1.f;
    tx.tx = 0.f;
    tx.ty = 0.f;

    m_shapes[m_shapeCount] = cpSpaceAddShape(m_system->m_space, cpPolyShapeNew(m_body, points.size(), converted.data(), tx, 0.02));
    auto shape = m_shapes[m_shapeCount++];
    applyProperties(properties, shape);

    return shape;
}

cpShape* PhysicsObject::addBoxShape(const ShapeProperties& properties, sf::Vector2f size)
{
    XY_ASSERT(m_system && m_body, "Component not initialised!");
    XY_ASSERT(m_shapeCount < MaxShapes, "No more shapes available!");

    m_shapes[m_shapeCount] = cpSpaceAddShape(m_system->m_space, cpBoxShapeNew(m_body, Convert::toPhysFloat(size.x), Convert::toPhysFloat(size.y), 0.02));
    auto shape = m_shapes[m_shapeCount++];
    applyProperties(properties, shape);

    return shape;
}

//private
void PhysicsObject::applyProperties(const ShapeProperties& properties, cpShape* shape)
{
    cpShapeSetElasticity(shape, properties.elasticity);
    cpShapeSetMass(shape, properties.mass);
    cpShapeSetSensor(shape, properties.isSensor ? 1 : 0);
    cpShapeSetFriction(shape, properties.friction);

    auto vel = cpv(Convert::toPhysFloat(properties.surfaceVelocity.x), Convert::toPhysFloat(properties.surfaceVelocity.y));
    cpShapeSetSurfaceVelocity(shape, vel);
}