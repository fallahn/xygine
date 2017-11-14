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

#include "CrateSystem.hpp"
#include "ClientServerShared.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"
#include "PlayerSystem.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/network/NetHost.hpp>
#include <xyginext/util/Vector.hpp>

namespace
{
    const float MinVelocity = 25.f; //min len sqr
    const float PushAcceleration = 14.f;
    const float LethalVelocity = 100.f;
}

CrateSystem::CrateSystem(xy::MessageBus& mb, xy::NetHost& nh)
    : xy::System(mb, typeid(CrateSystem)),
    m_host(nh)
{
    requireComponent<Crate>();
    requireComponent<xy::Transform>();
}

//public
void CrateSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& crate = entity.getComponent<Crate>();
        switch (crate.state)
        {
        default:break;
        case Crate::Ground:
            crate.velocity *= 0.93f; //friction
            groundCollision(entity);
            break;
        case Crate::Falling:
            crate.velocity.y += Gravity * dt;
            airCollision(entity);
            break;
        case Crate::Breaking:
            //TODO probably moot state, we'll just spawn some particles instead
            break;
        }

        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(crate.velocity * dt);

        float l2 = xy::Util::Vector::lengthSquared(crate.velocity);
        if (l2 < MinVelocity)
        {
            crate.velocity = {};
        }
        crate.lethal = (l2 > LethalVelocity);
    }
}

//private
void CrateSystem::groundCollision(xy::Entity entity)
{
    const auto& collision = entity.getComponent<CollisionComponent>();
    auto hitboxCount = collision.getHitboxCount();
    const auto& hitboxes = collision.getHitboxes();

    auto& crate = entity.getComponent<Crate>();

    for (auto i = 0u; i < hitboxCount; ++i)
    {
        auto collisionCount = hitboxes[i].getCollisionCount();
        const auto& manifolds = hitboxes[i].getManifolds();

        if (hitboxes[i].getType() == CollisionType::Crate)
        {
            for (auto j = 0; j < collisionCount; ++j)
            {
                switch (manifolds[j].otherType)
                {
                default:break;
                case CollisionType::Platform:
                case CollisionType::Solid:
                case CollisionType::Crate:
                    entity.getComponent<xy::Transform>().move(manifolds[j].normal * manifolds[j].penetration);
                    if (manifolds[j].normal.x != 0)
                    {
                        crate.velocity.x = -crate.velocity.x;
                    }
                    break;
                case CollisionType::Player:
                    if (manifolds[j].normal.x != 0)
                    {
                        entity.getComponent<xy::Transform>().move(manifolds[j].normal * manifolds[j].penetration);
                        crate.velocity += manifolds[j].normal * manifolds[j].penetration * PushAcceleration;
                        crate.lastOwner = manifolds[j].otherEntity.getComponent<Player>().playerNumber;
                    }
                    break;
                }

                if (/*crate.lethal || */manifolds[j].penetration > (CrateBounds.width / 2.f))
                {
                    //destroy the crate - TODO check if should explode
                    destroy(entity);
                    return;
                }
            }
        }
        else if (hitboxes[i].getType() == CollisionType::Foot)
        {
            if (collisionCount == 0)
            {
                crate.groundContact = false;
                crate.state = Crate::Falling;
                return;
            }
            
        }
    }
}

void CrateSystem::airCollision(xy::Entity entity)
{
    const auto& collision = entity.getComponent<CollisionComponent>();
    auto hitboxCount = collision.getHitboxCount();
    const auto& hitboxes = collision.getHitboxes();

    auto& crate = entity.getComponent<Crate>();

    for (auto i = 0u; i < hitboxCount; ++i)
    {
        auto collisionCount = hitboxes[i].getCollisionCount();
        const auto& manifolds = hitboxes[i].getManifolds();

        if (hitboxes[i].getType() == CollisionType::Crate)
        {
            for (auto j = 0; j < collisionCount; ++j)
            {
                if (manifolds[j].otherType == CollisionType::Teleport)
                {
                    if (manifolds[j].normal.y < 0)
                    {
                        //move up
                        entity.getComponent<xy::Transform>().move(0.f, -TeleportDistance);
                    }
                    else
                    {
                        //move down
                        entity.getComponent<xy::Transform>().move(0.f, TeleportDistance);
                    }
                }
                else if (manifolds[j].otherType == CollisionType::NPC
                    || manifolds[j].otherType == CollisionType::Player)
                {
                    //SQUISH
                    destroy(entity);
                    return;
                }
                else
                {
                    entity.getComponent<xy::Transform>().move(manifolds[j].normal * manifolds[j].penetration);
                    if (manifolds[j].normal.y < 1 && crate.groundContact)
                    {
                        crate.state = Crate::Ground;
                        crate.velocity.y = 0.f;
                        return;
                    }
                }
            }
        }
        else if (hitboxes[i].getType() == CollisionType::Foot)
        {
            crate.groundContact = (collisionCount != 0);
        }
    }
}

void CrateSystem::destroy(xy::Entity entity)
{
    const auto& tx = entity.getComponent<xy::Transform>();
    getScene()->destroyEntity(entity);

    //broadcast to client
    ActorEvent evt;
    evt.actor.id = entity.getIndex();
    evt.actor.type = entity.getComponent<Actor>().type;
    evt.x = tx.getPosition().x;
    evt.y = tx.getPosition().y;
    evt.type = ActorEvent::Died;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
}