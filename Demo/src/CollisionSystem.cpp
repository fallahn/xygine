/*********************************************************************
(c) Matt Marchant 2017
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

#include "CollisionSystem.hpp"
#include "Hitbox.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/core/App.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

CollisionSystem::CollisionSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(CollisionSystem))
{
    requireComponent<CollisionComponent>();
    requireComponent<xy::Transform>();
}

//public
void CollisionSystem::process(float dt)
{
    m_vertices.clear();
    m_collisions.clear();

    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        const auto& xForm = entity.getComponent<xy::Transform>();
        auto& collisionComponent = entity.getComponent<CollisionComponent>();

        for (auto i = 0u; i < collisionComponent.m_hitboxCount; ++i)
        {
            auto& hitbox = collisionComponent.m_hitboxes[i];
            auto rect = hitbox.getCollisionRect();
            rect = entity.getComponent<xy::Transform>().getTransform().transformRect(rect);

            //draw some debug to make sure the entities are being created in the correct place
            sf::Color colour = (hitbox.m_collisionCount > 0) ? sf::Color::Red : sf::Color::Green;
            //repeat the first and last points with transparent verts
            m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Transparent);
            m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), colour); //TODO use a colour based on collision type?
            m_vertices.emplace_back(sf::Vector2f(rect.left + rect.width, rect.top), colour);
            m_vertices.emplace_back(sf::Vector2f(rect.left + rect.width, rect.top + rect.height), colour);
            m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top + rect.height), colour);
            m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), colour);
            m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Transparent);

            hitbox.m_collisionCount = 0;
        }
        
        //actual collision testing...
        auto globalBounds = xForm.getTransform().transformRect(collisionComponent.getLocalBounds());
        for (const auto& other : entities)
        {
            if (entity != other)
            {
                //only test for collisions first, so we make sure each pair is processed only once
                auto otherBounds = other.getComponent<xy::Transform>().getTransform().transformRect(other.getComponent<CollisionComponent>().getLocalBounds());

                if (globalBounds.intersects(otherBounds))
                {
                    m_collisions.insert(std::minmax(entity, other));
                }
            }
        }     
    }

    //calc manifolds for any collisions and enter into component info
    //DPRINT("Broadphase count", std::to_string(m_collisions.size()));
    for (auto c : m_collisions)
    {
        const auto& txA = c.first.getComponent<xy::Transform>();
        const auto& txB = c.second.getComponent<xy::Transform>();

        auto& ccA = c.first.getComponent<CollisionComponent>();
        auto& ccB = c.second.getComponent<CollisionComponent>();

        //such nested...
        for (auto i = 0; i < ccA.m_hitboxCount; ++i)
        {
            auto& boxA = ccA.m_hitboxes[i];
            auto rectA = txA.getTransform().transformRect(boxA.getCollisionRect());

            for (auto j = 0; j < ccB.m_hitboxCount; ++j)
            {
                auto& boxB = ccB.m_hitboxes[j];

                sf::FloatRect overlap;
                rectA.intersects(txB.getTransform().transformRect(boxB.getCollisionRect()), overlap);
                sf::Vector2f normal = (txB.getPosition() - txB.getOrigin()) - (txA.getPosition() - txA.getOrigin());

                Manifold manifold;
                if (overlap.width < overlap.height)
                {
                    manifold.normal.x = (normal.x < 0) ? 1.f : -1.f;
                    manifold.penetration = overlap.width;
                }
                else
                {
                    manifold.normal.y = (normal.y < 0) ? 1.f : -1.f;
                    manifold.penetration = overlap.height;
                }
                manifold.otherType = boxB.getType();

                if (boxA.m_collisionCount < Hitbox::MaxCollisions)
                {
                    boxA.m_manifolds[boxA.m_collisionCount++] = manifold;
                }

                manifold.normal = -manifold.normal;
                manifold.otherType = boxA.getType();
                if (boxB.m_collisionCount < Hitbox::MaxCollisions)
                {
                    boxB.m_manifolds[boxB.m_collisionCount++] = manifold;
                }
            }
        }
    }
}

//private
void CollisionSystem::onEntityAdded(xy::Entity entity)
{

}

void CollisionSystem::onEntityRemoved(xy::Entity entity)
{

}

void CollisionSystem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::LinesStrip, states);
}