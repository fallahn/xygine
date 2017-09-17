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

#include "BubbleSystem.hpp"
#include "ActorSystem.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"
#include "ClientServerShared.hpp"
#include "Hitbox.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/Scene.hpp>

namespace
{
    const float spawnVelocity = 1100.f;
    const float verticalVelocity = -100.f;
}

BubbleSystem::BubbleSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(BubbleSystem)),
    m_host      (host)
{
    requireComponent<xy::Transform>();
    requireComponent<Actor>();
    requireComponent<Bubble>();
    requireComponent<CollisionComponent>();
}

//public
void BubbleSystem::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::PlayerMessage)
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::FiredWeapon)
        {
            const auto& player = data.entity.getComponent<Player>();
            auto pos = data.entity.getComponent<xy::Transform>().getPosition();
            pos.y -= PlayerSize / 2.f; //make appear near mouth, not feet
            
            //spawn a bubble
            auto scene = getScene();
            auto entity = scene->createEntity();
            entity.addComponent<xy::Transform>().setPosition(pos);
            entity.getComponent<xy::Transform>().setOrigin(BubbleSize / 2.f, BubbleSize / 2.f);
            entity.addComponent<Actor>().id = entity.getIndex();
            entity.getComponent<Actor>().type = (player.playerNumber == 0) ?  ActorID::BubbleOne : ActorID::BubbleTwo;
            entity.addComponent<Bubble>().player = player.playerNumber;
            entity.getComponent<Bubble>().velocity.x = (player.direction == Player::Direction::Right) ? spawnVelocity : -spawnVelocity;
            entity.addComponent<CollisionComponent>().addHitbox({ 0.f, 0.f, BubbleSize, BubbleSize }, CollisionType::Bubble);
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Bubble);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Solid | CollisionFlags::Player);
            entity.addComponent<xy::QuadTreeItem>().setArea({ 0.f, 0.f, BubbleSize, BubbleSize });

            //broadcast to clients
            ActorEvent evt;
            evt.actor.id = entity.getIndex();
            evt.actor.type = entity.getComponent<Actor>().type;
            evt.x = pos.x;
            evt.y = pos.y;
            evt.type = ActorEvent::Spawned;

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
        }
    }
}

void BubbleSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        doCollision(entity);        
        
        auto& bubble = entity.getComponent<Bubble>();
        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(bubble.velocity * dt);
        
        bubble.lifetime -= dt;
        if(bubble.state == Bubble::Spawning)
        {
            bubble.spawntime -= dt;
            if (bubble.spawntime < 0)
            {
                bubble.state = Bubble::Normal;
                bubble.velocity.x *= 0.001f;
                bubble.velocity.y = verticalVelocity;
            }
            break;
        }

        if (bubble.lifetime < 0)
        {
            getScene()->destroyEntity(entity);

            //broadcast to client
            ActorEvent evt;
            evt.actor.id = entity.getIndex();
            evt.actor.type = entity.getComponent<Actor>().type;
            evt.x = tx.getPosition().x;
            evt.y = tx.getPosition().y;
            evt.type = ActorEvent::Died;

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

            //TODO raise message
        }
    }
}

void BubbleSystem::doCollision(xy::Entity entity)
{
    auto& bubble = entity.getComponent<Bubble>();
    auto& tx = entity.getComponent<xy::Transform>();
    const auto& collision = entity.getComponent<CollisionComponent>();

    const auto& hitboxes = collision.getHitboxes();
    for (const auto& hitbox : hitboxes)
    {
        auto count = hitbox.getCollisionCount();
        for (auto i = 0u; i < count; ++i)
        {
            const auto& man = hitbox.getManifolds()[i];
            switch (man.otherType)
            {
            default: break;
            case CollisionType::Solid:
                tx.move(man.normal * man.penetration);
                break;
            case CollisionType::Player:
                switch (bubble.state)
                {
                case Bubble::Normal:
                    //player jumps off
                    if (man.normal.y > 0)
                    {                      
                        if (bubble.lifetime > 1.f) bubble.lifetime = 1.f;
                        bubble.velocity.y *= 0.5f;
                    }
                    //tx.move(man.normal * man.penetration);
                    break;
                case Bubble::Spawning:
                    /*bubble.state = Bubble::Normal;
                    bubble.velocity.x *= 0.001f;
                    bubble.velocity.y = verticalVelocity;*/
                    break;
                default:break;
                }
                break;
            }
        }
    }
}