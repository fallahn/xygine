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
#include "NPCSystem.hpp"
#include "AnimationController.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"
#include "ClientServerShared.hpp"
#include "Hitbox.hpp"
#include "CommandIDs.hpp"
#include "Server.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/util/Random.hpp>

namespace
{
    const float spawnVelocity = 600.f;
}

BubbleSystem::BubbleSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(BubbleSystem)),
    m_host      (host),
    m_enabled   (true)
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
        if (data.type == PlayerEvent::FiredWeapon && m_enabled)
        {
            const auto& player = data.entity.getComponent<Player>();
            auto pos = data.entity.getComponent<xy::Transform>().getPosition();
            pos.y -= PlayerBounds.height / 2.f; //make appear near mouth, not feet
            
            //spawn a bubble
            auto scene = getScene();
            auto entity = scene->createEntity();
            entity.addComponent<xy::Transform>().setPosition(pos);
            entity.getComponent<xy::Transform>().setOrigin(BubbleOrigin);
            entity.addComponent<Actor>().id = entity.getIndex();
            entity.getComponent<Actor>().type = (player.playerNumber == 0) ?  ActorID::BubbleOne : ActorID::BubbleTwo;
            entity.addComponent<Bubble>().player = player.playerNumber;
            //add player current velocity to spawn velocity
            entity.getComponent<Bubble>().velocity.x = (player.sync.direction == Player::Direction::Right) ? spawnVelocity : -spawnVelocity;
            entity.getComponent<Bubble>().velocity.x += player.sync.velocity.x;
            entity.addComponent<CollisionComponent>().addHitbox(BubbleBounds, CollisionType::Bubble);
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Bubble);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Solid | CollisionFlags::NPC | CollisionFlags::HardBounds);
            entity.addComponent<xy::QuadTreeItem>().setArea(BubbleBounds);

            entity.addComponent<AnimationController>();
            entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem; //so we can destroy at whim

            //broadcast to clients
            ActorEvent evt;
            evt.actor = entity.getComponent<Actor>();
            evt.x = pos.x;
            evt.y = pos.y;
            evt.type = ActorEvent::Spawned;
            evt.serverTime = GameServer::getServerTime();

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

        if (!entity.destroyed()) //collision may have killed the entity
        {
            auto& bubble = entity.getComponent<Bubble>();
            auto& tx = entity.getComponent<xy::Transform>();
            tx.move(bubble.velocity * dt);

            bubble.lifetime -= dt;
            if (bubble.state == Bubble::Spawning)
            {
                bubble.spawntime -= dt;
                bubble.velocity.x *= 0.98f;
                if (bubble.spawntime < 0)
                {
                    bubble.state = Bubble::Normal;
                    bubble.velocity.x = 0.f;
                    bubble.velocity.y = BubbleVerticalVelocity;

                    std::uint32_t flags = CollisionFlags::Solid | CollisionFlags::Player | CollisionFlags::NPC | CollisionFlags::HardBounds;
                    entity.getComponent<CollisionComponent>().setCollisionMaskBits(flags);

                    CollisionFlagsUpdate update;
                    update.actor = entity.getComponent<Actor>().id;
                    update.newflags = flags;
                    m_host.broadcastPacket(PacketID::CollisionFlag, update, xy::NetFlag::Reliable, 1);
                }
                break;
            }

            if (bubble.lifetime < 0
                || !MapBounds.contains(tx.getPosition()))
            {
                killBubble(entity);
            }
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
            case CollisionType::HardBounds:
                tx.move(man.normal * man.penetration);

                if (bubble.state == Bubble::Spawning)
                {
                    bubble.state = Bubble::Normal;
                    bubble.velocity.x = 0.f;
                    bubble.velocity.y = BubbleVerticalVelocity;

                    std::uint32_t flags = CollisionFlags::Solid | CollisionFlags::Player | CollisionFlags::NPC | CollisionFlags::HardBounds;
                    entity.getComponent<CollisionComponent>().setCollisionMaskBits(flags);

                    CollisionFlagsUpdate update;
                    update.actor = entity.getComponent<Actor>().id;
                    update.newflags = flags;
                    m_host.broadcastPacket(PacketID::CollisionFlag, update, xy::NetFlag::Reliable, 1);
                }

                break;
            case CollisionType::Solid:
                if (bubble.state != Bubble::Spawning)
                {
                    tx.move(man.normal * man.penetration);

                    /*if (man.normal.y != 0 && bubble.velocity.x == 0)
                    {
                        bubble.velocity.x = 
                            (xy::Util::Random::value(0, 1) == 0) ?
                            -BubbleVerticalVelocity : BubbleVerticalVelocity;
                    }
                    else if (man.normal.x != 0)
                    {
                        bubble.velocity.x = 0.f;
                    }*/
                }
                break;
            case CollisionType::Player:
                switch (bubble.state)
                {
                case Bubble::Normal:
                    //player jumps off
                    if (man.normal.y > 0)
                    {                      
                        if (bubble.lifetime > 3.f) bubble.lifetime = 3.f;
                        //bubble.velocity.y *= 0.5f;
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
            case CollisionType::NPC:
            {
                if (man.otherEntity.hasComponent<NPC>())
                {
                    auto otherState = man.otherEntity.getComponent<NPC>().state;
                    if (otherState != NPC::State::Bubble && otherState != NPC::State::Dying
                        && bubble.state == Bubble::Spawning)
                    {
                        //pop!
                        killBubble(entity);
                        return; //only want to hit one entity
                    }
                }
            }
                break;
            }
        }
    }
}

void BubbleSystem::killBubble(xy::Entity entity)
{
    const auto& tx = entity.getComponent<xy::Transform>();

    //broadcast to client
    ActorEvent evt;
    evt.actor.id = entity.getIndex();
    evt.actor.type = entity.getComponent<Actor>().type;
    evt.x = tx.getPosition().x;
    evt.y = tx.getPosition().y;
    evt.type = ActorEvent::Died;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

    getScene()->destroyEntity(entity);
    //TODO raise message
}