/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <NetworkDemoCollisions.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <algorithm>

CollisionWorld::CollisionWorld(xy::Scene& scene, xy::MessageBus& messageBus, sf::Color colour)
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(messageBus);
    drawable->getDrawable().setSize({ 1920.f, 20.f });
    drawable->getDrawable().setFillColor(colour);

    auto entity = xy::Entity::create(messageBus);
    auto dc = entity->addComponent(drawable);

    m_entities.push_back(scene.addEntity(entity, xy::Scene::Layer::BackRear));

    drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(messageBus);
    drawable->getDrawable() = dc->getDrawable();

    entity = xy::Entity::create(messageBus);
    entity->setPosition(0.f, 1060.f);
    entity->addComponent(drawable);

    m_entities.push_back(scene.addEntity(entity, xy::Scene::Layer::BackRear));

    auto netDrawable = xy::Component::create<xy::SfDrawableComponent<Net>>(messageBus);
    entity = xy::Entity::create(messageBus);
    entity->setPosition(950.f, 0.f);
    entity->addComponent(netDrawable);
    scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void CollisionWorld::addEntity(xy::Entity* entity)
{
    m_entities.push_back(entity);
}

void CollisionWorld::removeEntity(xy::Entity* entity)
{
    std::remove(m_entities.begin(), m_entities.end(), entity);
}

//drawable for net
namespace 
{
    const float netWidth = 20.f;
    const float netHeight = 40.f;
    const sf::Uint8 partCount = static_cast<sf::Uint8>(1080.f / netHeight);
}
CollisionWorld::Net::Net()
{
    sf::Color colour;
    for (auto i = 0; i < partCount; ++i)
    {
       colour = (colour.r == 0) ? sf::Color::White : sf::Color::Black;
       sf::Vertex v = { {0.f, netHeight * i}, colour };
       m_vertexArray.push_back(v);
       v.position.x += netWidth;
       m_vertexArray.push_back(v);
       v.position.y += netHeight;
       m_vertexArray.push_back(v);
       v.position.x -= netWidth;
       m_vertexArray.push_back(v);
    }
}

void CollisionWorld::Net::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertexArray.data(), m_vertexArray.size(), sf::Quads, states);
}