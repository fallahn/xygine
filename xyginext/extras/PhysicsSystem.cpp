/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include <xyginext/ecs/components/Transform.hpp>

PhysicsSystem::PhysicsSystem(xy::MessageBus& mb, sf::Vector2f gravity)
    : xy::System(mb, typeid(PhysicsSystem)),
    m_space     (nullptr)
{
    requireComponent<PhysicsObject>();
    requireComponent<xy::Transform>();

    auto grav = cpv(Convert::toPhysFloat(gravity.x), Convert::toPhysFloat(gravity.y));

    m_space = cpSpaceNew();
    cpSpaceSetGravity(m_space, grav);
    cpSpaceSetSleepTimeThreshold(m_space, 1.0);
}

PhysicsSystem::~PhysicsSystem()
{
    if (m_space)
    {
        for (const auto& c : m_constraints)
        {
            cpSpaceRemoveConstraint(m_space, c.constraint);
        }

        auto& entities = getEntities();
        for (auto entity : entities)
        {
            removeObject(entity.getComponent<PhysicsObject>());
        }
        cpSpaceFree(m_space);
    }
}

//public
void PhysicsSystem::process(float dt)
{
    cpSpaceStep(m_space, dt);

    auto& entities = getEntities();
    for (auto entity : entities)
    {
        auto& tx = entity.getComponent<xy::Transform>();
        const auto& phys = entity.getComponent<PhysicsObject>();

        tx.setPosition(Convert::toWorldVec(cpBodyGetPosition(phys.m_body)));
        tx.setRotation(Convert::toDegrees(cpBodyGetAngle(phys.m_body)));
    }
}

PhysicsObject PhysicsSystem::createObject(sf::Vector2f position, PhysicsObject::Type type)
{
    XY_ASSERT(m_space, "No space created!");
    PhysicsObject obj;
    obj.m_system = this;

    //add body
    switch (type)
    {
    default:
    case PhysicsObject::Type::Dynamic:
        obj.m_body = cpSpaceAddBody(m_space, cpBodyNew(0.0, 0.0));
        obj.m_type = PhysicsObject::Type::Dynamic;
        break;
    case PhysicsObject::Type::Kinematic:
        obj.m_body = cpSpaceAddBody(m_space, cpBodyNewKinematic());
        obj.m_type = PhysicsObject::Type::Kinematic;
        break;
    case PhysicsObject::Type::Static:
        obj.m_body = cpSpaceAddBody(m_space, cpBodyNewStatic());
        obj.m_type = PhysicsObject::Type::Static;
        break;
    }
    cpBodySetPosition(obj.m_body, Convert::toPhysVec(position));
    
    return obj;
}

void PhysicsSystem::removeObject(PhysicsObject& obj)
{
    //remove obj shapes
    for (auto i = 0u; i < obj.m_shapeCount; ++i)
    {
        cpSpaceRemoveShape(m_space, obj.m_shapes[i]);
        cpShapeFree(obj.m_shapes[i]);
        obj.m_shapes[i] = nullptr;
    }

    //remove body
    cpSpaceRemoveBody(m_space, obj.m_body);
    cpBodyFree(obj.m_body);
    obj.m_body = nullptr;

    obj.m_system = nullptr;
}

cpConstraint* PhysicsSystem::addPivotConstraint(xy::Entity objectA, xy::Entity objectB, sf::Vector2f position)
{
    XY_ASSERT(objectA.isValid() && objectB.isValid(), "Invalid object");
    XY_ASSERT(objectA.hasComponent<PhysicsObject>() && objectB.hasComponent<PhysicsObject>(), "Requires physics object component");

    auto* bodyA = objectA.getComponent<PhysicsObject>().getBody();
    auto* bodyB = objectB.getComponent<PhysicsObject>().getBody();

    ConstraintPair cp;
    cp.a = objectA;
    cp.b = objectB;
    cp.constraint = cpSpaceAddConstraint(m_space, cpPivotJointNew(bodyA, bodyB, Convert::toPhysVec(position)));

    m_constraints.push_back(cp);
    return cp.constraint;
}

cpConstraint* PhysicsSystem::addMotorConstraint(xy::Entity objectA, xy::Entity objectB, float rate)
{
    XY_ASSERT(objectA.isValid() && objectB.isValid(), "Invalid object");
    XY_ASSERT(objectA.hasComponent<PhysicsObject>() && objectB.hasComponent<PhysicsObject>(), "Requires physics object component");

    auto* bodyA = objectA.getComponent<PhysicsObject>().getBody();
    auto* bodyB = objectB.getComponent<PhysicsObject>().getBody();

    ConstraintPair cp;
    cp.a = objectA;
    cp.b = objectB;
    cp.constraint = cpSpaceAddConstraint(m_space, cpSimpleMotorNew(bodyA, bodyB, Convert::toRadians(rate)));

    m_constraints.push_back(cp);
    return cp.constraint;
}

void PhysicsSystem::onEntityRemoved(xy::Entity entity)
{
    //remove all constraints which use this entity
    m_constraints.erase(std::remove_if(
    m_constraints.begin(), m_constraints.end(),
        [&,entity](const ConstraintPair& cp)
        {
            if (cp.a == entity
                || cp.b == entity)
            {
                cpSpaceRemoveConstraint(m_space, cp.constraint);
                return true;
            }
            return false;
        }
    ), m_constraints.end());

    removeObject(entity.getComponent<PhysicsObject>());
}