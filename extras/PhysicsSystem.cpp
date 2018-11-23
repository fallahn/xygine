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

#include <xyginext/ecs/components/Transform.hpp>

PhysicsSystem::PhysicsSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(PhysicsSystem)),
    m_space     (nullptr)
{
    requireComponent<PhysicsObject>();
    requireComponent<xy::Transform>();

    auto gravity = cpv(0.f, -99.f);

    m_space = cpSpaceNew();
    cpSpaceSetGravity(m_space, gravity);
    cpSpaceSetSleepTimeThreshold(m_space, 1.0);
}

PhysicsSystem::~PhysicsSystem()
{
    if (m_space)
    {
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

void PhysicsSystem::onEntityRemoved(xy::Entity entity)
{
    removeObject(entity.getComponent<PhysicsObject>());
}