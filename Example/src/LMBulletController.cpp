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

#include <LMBulletController.hpp>
#include <LMCollisionComponent.hpp>

#include <xygine/Entity.hpp>

using namespace lm;

namespace
{
    const sf::Vector2f velocity(0.f, -600.f);
}

BulletController::BulletController(xy::MessageBus& mb)
    : xy::Component(mb, this),
    m_entity(nullptr)
{

}

//public
void BulletController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(velocity * dt);
}

void BulletController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void BulletController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {    
    case CollisionComponent::ID::Player:
    case CollisionComponent::ID::Mothership:
        //do nothing
        break;
    default: 
        //destroy becasue we hit something
        m_entity->destroy();

        //TODO raise message so we can do some particle effects

        break;
    }
}