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

#include "HatSystem.hpp"
#include "Hitbox.hpp"
#include "AnimationController.hpp"
#include "ClientServerShared.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "CommandIDs.hpp"
#include "PacketIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <xyginext/network/NetHost.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Random.hpp>

namespace
{
    const float MinHatTime = 30.f;
    const float MaxHatTime = 48.f;
}

HatSystem::HatSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System    (mb, typeid(HatSystem)),
    m_host          (host),
    m_hatActive     (false),
    m_nextHatTime   (xy::Util::Random::value(MinHatTime, MaxHatTime))
{
    requireComponent<MagicHat>();
    requireComponent<xy::Transform>();
    requireComponent<CollisionComponent>();
    requireComponent<AnimationController>();
}

//public
void HatSystem::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::PlayerMessage)
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::LostHat && m_hatActive)
        {
            auto position = data.entity.getComponent<xy::Transform>().getPosition();

            //spawn a hat
            auto scene = getScene();
            auto entity = scene->createEntity();
            entity.addComponent<xy::Transform>().setPosition(position);
            entity.getComponent<xy::Transform>().setOrigin(PlayerOrigin);
            entity.addComponent<Actor>().id = entity.getIndex();
            entity.getComponent<Actor>().type = ActorID::MagicHat;
            entity.addComponent<MagicHat>().velocity.y = xy::Util::Random::value(-660.f, -520.f);
            entity.getComponent<MagicHat>().velocity.x = xy::Util::Random::value(-500.f, 500.f);
            entity.getComponent<MagicHat>().state = MagicHat::Dying;

            entity.addComponent<CollisionComponent>().addHitbox(PlayerBounds, CollisionType::MagicHat);
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::MagicHat);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(
                /*CollisionFlags::Solid | CollisionFlags::Platform | */CollisionFlags::HardBounds/* | CollisionFlags::Player | CollisionFlags::Teleport*/);
            entity.addComponent<xy::QuadTreeItem>().setArea(PlayerBounds);

            entity.addComponent<AnimationController>();
            entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem; //so we can destroy at whim

            //broadcast to clients
            ActorEvent evt;
            evt.actor.id = entity.getIndex();
            evt.actor.type = entity.getComponent<Actor>().type;
            evt.x = position.x;
            evt.y = position.y;
            evt.type = ActorEvent::Spawned;

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
        }
    }
    else if (msg.id == MessageID::MapMessage)
    {
        const auto& data = msg.getData<MapEvent>();
        if (data.type == MapEvent::MapChangeStarted)
        {
            m_hatActive = false; //assumes any active hats are destroyed as a MapItem
        }
    }
}

void HatSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& hat = entity.getComponent<MagicHat>();
        switch (hat.state)
        {
        case MagicHat::Spawning:
            hat.spawnTime -= dt;
            updateSpawning(entity, dt);
            break;
        case MagicHat::Dying:
            updateDying(entity, dt);
            break;
        case MagicHat::Idle:
            updateIdle(entity);
            break;
        }
    }

    if (!m_hatActive)
    {
        //check to see if we should spawn a hat
        m_nextHatTime -= dt;

        if (m_nextHatTime < 0)
        {
            m_nextHatTime = xy::Util::Random::value(MinHatTime, MaxHatTime);
            m_hatActive = true;

            sf::Vector2f position(xy::Util::Random::value(128.f, MapBounds.width - 128.f),
                xy::Util::Random::value(256.f, 512.f));

            //spawn a hat
            auto scene = getScene();
            auto entity = scene->createEntity();
            entity.addComponent<xy::Transform>().setPosition(position);
            entity.getComponent<xy::Transform>().setOrigin(PlayerOrigin);
            entity.addComponent<Actor>().id = entity.getIndex();
            entity.getComponent<Actor>().type = ActorID::MagicHat;
            entity.addComponent<MagicHat>();
            entity.getComponent<MagicHat>().state = MagicHat::Spawning;

            entity.addComponent<CollisionComponent>().addHitbox(PlayerBounds, CollisionType::MagicHat);
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::MagicHat);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(
                CollisionFlags::Solid | CollisionFlags::Platform | CollisionFlags::Teleport | CollisionFlags::Player);
            entity.addComponent<xy::QuadTreeItem>().setArea(PlayerBounds);

            entity.addComponent<AnimationController>();
            entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem; //so we can destroy at whim

            //broadcast to clients
            ActorEvent evt;
            evt.actor.id = entity.getIndex();
            evt.actor.type = entity.getComponent<Actor>().type;
            evt.x = position.x;
            evt.y = position.y;
            evt.type = ActorEvent::Spawned;

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
        }
    }
}

//private
void HatSystem::updateSpawning(xy::Entity entity, float dt)
{
    auto& hat = entity.getComponent<MagicHat>();
    auto& tx = entity.getComponent<xy::Transform>();

    hat.velocity.y = std::min(hat.velocity.y + (Gravity * dt), MaxVelocity);
    tx.move(hat.velocity * dt);

    if (tx.getPosition().y > xy::DefaultSceneSize.y)
    {
        destroy(entity);
        m_hatActive = false;
        return;
    }

    const auto& collision = entity.getComponent<CollisionComponent>();
    for (auto i = 0; i < collision.getHitboxCount(); ++i)
    {
        const auto& hitbox = collision.getHitboxes()[i];
        for (auto j = 0; j < hitbox.getCollisionCount(); ++j)
        {
            const auto& man = hitbox.getManifolds()[j];
            if ((man.otherType == CollisionType::Platform || man.otherType == CollisionType::Solid))
            {
                tx.move(man.normal * man.penetration);
                if (hat.spawnTime < 0 && man.normal.y < 0)
                { 
                    hat.velocity = {};
                    hat.state = MagicHat::Idle;
                    return;
                }
            }
            else if (man.otherType == CollisionType::Teleport)
            {
                if (man.normal.y < 0)
                {
                    //move up
                    tx.move(0.f, -TeleportDistance);
                }
                else
                {
                    //move down
                    tx.move(0.f, TeleportDistance);
                }
            }
        }
    }

    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
}

void HatSystem::updateIdle(xy::Entity entity)
{
    const auto& collision = entity.getComponent<CollisionComponent>();
    for (auto i = 0; i < collision.getHitboxCount(); ++i)
    {
        const auto& hitbox = collision.getHitboxes()[i];
        for (auto j = 0; j < hitbox.getCollisionCount(); ++j)
        {
            const auto& man = hitbox.getManifolds()[j];
            if (man.otherType == CollisionType::Player)
            {
                xy::Command cmd;
                cmd.targetFlags = 
                    (man.otherEntity.getComponent<Player>().playerNumber == 0) 
                    ? CommandID::PlayerOne : CommandID::PlayerTwo;
                cmd.action = [&](xy::Entity playerEnt, float)
                {
                    auto& player = playerEnt.getComponent<Player>();
                    if (!player.hasHat && (player.state == Player::State::Jumping || player.state == Player::State::Walking))
                    {
                        player.hasHat = true;
                        auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
                        msg->type = PlayerEvent::GotHat;
                        msg->entity = playerEnt;
                    }
                };
                getScene()->getSystem<xy::CommandSystem>().sendCommand(cmd);
                
                destroy(entity);
                return;
            }
            else
            {
                entity.getComponent<xy::Transform>().move(man.normal * man.penetration);
            }
        }
    }

    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Dead;
}

void HatSystem::updateDying(xy::Entity entity, float dt)
{
    auto& hat = entity.getComponent<MagicHat>();
    auto& tx = entity.getComponent<xy::Transform>();

    hat.velocity.y = std::min(hat.velocity.y + (Gravity * dt * 0.7f), MaxVelocity);
    tx.move(hat.velocity * dt);

    if (tx.getPosition().y > xy::DefaultSceneSize.y)
    {
        /*destroy(entity);
        m_hatActive = false;*/
        tx.setPosition(tx.getPosition().x, 256.f);
        hat.state = MagicHat::Spawning;
        hat.spawnTime = xy::Util::Random::value(0.3f, 0.5f);
        hat.velocity.x = 0.f;

        entity.getComponent<CollisionComponent>().setCollisionMaskBits(
            CollisionFlags::Solid | CollisionFlags::Platform | CollisionFlags::Teleport | CollisionFlags::Player);

        return;
    }

#ifdef XY_DEBUG
    auto vel = hat.velocity;
    m_host.broadcastPacket(PacketID::DebugCrownVelocity, vel, xy::NetFlag::Unreliable, 0);
#endif

    const auto& collision = entity.getComponent<CollisionComponent>();
    for (auto i = 0; i < collision.getHitboxCount(); ++i)
    {
        const auto& hitbox = collision.getHitboxes()[i];
        for (auto j = 0; j < hitbox.getCollisionCount(); ++j)
        {
            const auto& man = hitbox.getManifolds()[j];
            if (man.otherType == CollisionType::HardBounds)
            {
                tx.move(man.normal * man.penetration);
                hat.velocity.x = -hat.velocity.x;
                return;
            }
        }
    }

    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
}

void HatSystem::destroy(xy::Entity entity)
{
    XY_ASSERT(entity.hasComponent<MagicHat>(), "Not a hat entity");
    
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