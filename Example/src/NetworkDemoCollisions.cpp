/*********************************************************************
Matt Marchant 2014 - 2016
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


    //delete this when multiplayering
    drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(messageBus);
    drawable->getDrawable().setSize({ 20.f, 1080.f });
    drawable->getDrawable().setFillColor(colour);

    entity = xy::Entity::create(messageBus);
    entity->setPosition(1900.f, 0.f);
    entity->addComponent(drawable);

    m_entities.push_back(scene.addEntity(entity, xy::Scene::Layer::BackRear));
}

void CollisionWorld::addEntity(xy::Entity* entity)
{
    m_entities.push_back(entity);
}

void CollisionWorld::removeEntity(xy::Entity* entity)
{
    std::remove(m_entities.begin(), m_entities.end(), entity);
}