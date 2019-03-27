/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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

#include "CollisionSystem.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/BroadPhaseComponent.hpp>
#include <xyginext/ecs/systems/DynamicTreeSystem.hpp>
#include <xyginext/ecs/Scene.hpp>

CollisionSystem::CollisionSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(CollisionSystem))
{
    requireComponent<xy::Transform>();
    requireComponent<xy::BroadphaseComponent>();
    requireComponent<Collider>();
}

void CollisionSystem::process(float)
{
    m_collisions.clear();

    //do a broadphase pass on our list of entities
    //if they are a dynamic collider such as the ball or paddle
    auto& entities = getEntities();
    for (auto entity : entities)
    {
        auto& collision = entity.getComponent<Collider>();
        if (collision.dynamic)
        {
            //query the dynamic tree for nearby entities
            auto bounds = entity.getComponent<xy::Transform>().getTransform().transformRect(entity.getComponent<xy::BroadphaseComponent>().getArea());
            auto others = getScene()->getSystem<xy::DynamicTreeSystem>().query(bounds);

            //check each of the results to see if their AABBs overlap
            for (auto other : others)
            {
                if (other != entity)
                {
                    auto otherBounds = other.getComponent<xy::Transform>().getTransform().transformRect(other.getComponent<xy::BroadphaseComponent>().getArea());

                    //and add it to the collision list
                    if (bounds.intersects(otherBounds))
                    {
                        m_collisions.insert(std::minmax(entity, other));
                    }
                }
            }

        }
    }

    //iterate the list of collisions, correct the overlap
    //and call any callbacks if they exist (narrow phase)
    for (auto& c : m_collisions)
    {
        auto itemA = c.first;
        auto itemB = c.second;

        auto& txA = itemA.getComponent<xy::Transform>();
        auto& txB = itemB.getComponent<xy::Transform>();

        const auto& collisionA = itemA.getComponent<Collider>();
        const auto& collisionB = itemB.getComponent<Collider>();

        //manifold calculation
        auto boundsA = txA.getTransform().transformRect(itemA.getComponent<xy::BroadphaseComponent>().getArea());
        auto boundsB = txB.getTransform().transformRect(itemB.getComponent<xy::BroadphaseComponent>().getArea());
        
        sf::FloatRect intersection;
        boundsA.intersects(boundsB, intersection);

        sf::Vector2f normal = txB.getPosition() - txA.getPosition();

        Manifold manifold;
        if (intersection.width < intersection.height)
        {
            manifold.normal.x = (normal.x < 0) ? 1.f : -1.f;
            manifold.penetration = intersection.width;
        }
        else
        {
            manifold.normal.y = (normal.y < 0) ? 1.f : -1.f;
            manifold.penetration = intersection.height;
        }

        //only correct position if dynamic objects
        if (collisionA.dynamic)
        {
            txA.move(manifold.normal * manifold.penetration);   
        }

        if (collisionA.callback)
        {
            collisionA.callback(itemA, itemB, manifold);
        }

        //flip the normal for the other item
        manifold.normal = -manifold.normal;

        if (collisionB.dynamic)
        {
            txB.move(manifold.normal * manifold.penetration);
        }

        if (collisionB.callback)
        {
            collisionB.callback(itemB, itemA, manifold);
        }
    }
}