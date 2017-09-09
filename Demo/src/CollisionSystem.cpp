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

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

CollisionSystem::CollisionSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(CollisionSystem))
{
    requireComponent<Hitbox>();
    requireComponent<xy::Transform>();
}

//public
void CollisionSystem::process(float dt)
{
    m_vertices.clear();

    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        //draw some debug to make sure the entities are being created in the correct place
        auto rect = entity.getComponent<Hitbox>().getCollisionRect();
        rect = entity.getComponent<xy::Transform>().getWorldTransform().transformRect(rect);

        //repeat the first and last points with transparent verts
        m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Transparent);
        m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Green); //TODO use a colour based on collision type?
        m_vertices.emplace_back(sf::Vector2f(rect.left + rect.width, rect.top), sf::Color::Green);
        m_vertices.emplace_back(sf::Vector2f(rect.left + rect.width, rect.top + rect.height), sf::Color::Green);
        m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top + rect.height), sf::Color::Green);
        m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Green);
        m_vertices.emplace_back(sf::Vector2f(rect.left, rect.top), sf::Color::Transparent);

        //TODO actual collision testing...
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