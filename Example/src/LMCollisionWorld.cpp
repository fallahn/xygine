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

#include <LMCollisionWorld.hpp>

using namespace lm;

CollisionWorld::CollisionWorld() {}

//public
CollisionComponent::Ptr CollisionWorld::addComponent(xy::MessageBus& mb, sf::FloatRect area, CollisionComponent::ID id)
{
    auto cc = xy::Component::create<CollisionComponent>(mb, area, id);
    if (id == CollisionComponent::ID::Player || id == CollisionComponent::ID::Bullet)
    {
        m_colliders.push_back(cc.get());
    }
    else
    {
        m_collidees.push_back(cc.get());
    }
    return std::move(cc);
}

void CollisionWorld::update()
{
    m_colliders.erase(std::remove_if(m_colliders.begin(), m_colliders.end(), 
        [](const CollisionComponent* cp)
    {
        return cp->destroyed(); 
    }), m_colliders.end());

    m_collidees.erase(std::remove_if(m_collidees.begin(), m_collidees.end(),
        [](const CollisionComponent* cp)
    {
        return cp->destroyed();
    }), m_collidees.end());


    for (auto ca : m_colliders)
    {
        for (auto cb : m_collidees)
        {
            if (ca->globalBounds().intersects(cb->globalBounds()))
            {
                ca->addCollider(cb);
                break;
            }
        }
    }
}