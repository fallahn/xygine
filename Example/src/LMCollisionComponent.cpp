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

#include <LMCollisionComponent.hpp>

#include <xygine/Entity.hpp>

using namespace lm;

CollisionComponent::CollisionComponent(xy::MessageBus& mb, sf::FloatRect area, CollisionComponent::ID id)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_localBounds   (area),
    m_id            (id)
{
    resolve = [](CollisionComponent*) {};
}

//public
void CollisionComponent::entityUpdate(xy::Entity& entity, float)
{
    for (auto c : m_collisions)
    {
        resolve(c);
    }
    m_collisions.clear();
}

void CollisionComponent::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

sf::FloatRect CollisionComponent::localBounds() const
{
    return m_localBounds;
}

sf::FloatRect CollisionComponent::globalBounds() const
{
    return m_entity->getWorldTransform().transformRect(m_localBounds);
}

CollisionComponent::ID CollisionComponent::getID() const
{
    return m_id;
}

void CollisionComponent::setCallback(const Callback& cb)
{
    resolve = cb;
}

void CollisionComponent::addCollider(CollisionComponent* c)
{
    if(!c->destroyed()) m_collisions.push_back(c);
}