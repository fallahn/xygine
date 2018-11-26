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
#include "PowerupSystem.hpp"
#include "Explosion.hpp"
#include "AnimationController.hpp"
#include "CommandIDs.hpp"
#include "NPCSystem.hpp"
#include "MessageIDs.hpp"
#include "DynamiteSystem.hpp"
#include "Server.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
//#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/network/NetHost.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Wavetable.hpp>
#include <xyginext/util/Random.hpp>

namespace
{
    const float MinVelocity = 25.f; //min len sqr
    const float PushAcceleration = 10.f;
    const float LethalVelocity = 100000.f; //vel sqr before box becomes lethal
    const float RespawnTime = 5.f;
    const float NpcMaxFallVelocity = 20000.f; //NPCs falling faster than this break creates
}

CrateSystem::CrateSystem(xy::MessageBus& mb, xy::NetHost& nh)
    : xy::System(mb, typeid(CrateSystem)),
    m_host          (nh),
    m_respawnCount  (0)
{
    requireComponent<Crate>();
    requireComponent<xy::Transform>();

    m_waveTable = xy::Util::Wavetable::sine(8.f, 1.2f);
}

//public
void CrateSystem::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::MapMessage)
    {
        const auto& data = msg.getData<MapEvent>();
        if (data.type == MapEvent::MapChangeStarted)
        {
            //clear respawn list so they aren't carried on to next map
            m_respawnCount = 0;
        }
    }
}

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
            crate.velocity *= 0.89f; //friction
            groundCollision(entity);
            break;
        case Crate::Falling:
            crate.velocity.y += Gravity * dt;
            airCollision(entity);
            break;
        case Crate::Breaking:
            //another collision marked it as breaking
            destroy(entity);
            continue;
        case Crate::Carried:

            continue; //don't update velocity
        }

        auto& tx = entity.getComponent<xy::Transform>();
        tx.move(crate.velocity * dt);

        //check velocity and put to either full stop or make lethal
        float l2 = xy::Util::Vector::lengthSquared(crate.velocity);
        if (l2 < MinVelocity)
        {
            crate.velocity = {};
        }

        crate.lethal = (l2 > LethalVelocity);

        //check if crate explosive and do occasional shake if not moving
        if (crate.explosive && l2 == 0)
        {
            crate.shake.shakeTime -= dt;
            if (crate.shake.shakeTime < 0)
            {
                if (!crate.shake.shaking)
                {
                    crate.shake.shaking = true;
                    crate.shake.startPosition = tx.getPosition();
                    crate.shake.shakeTime = Crate::ShakeTime;
                }
                else
                {
                    crate.shake.shaking = false;
                    crate.shake.shakeTime = Crate::PauseTime;
                }
            }

            if (crate.shake.shaking)
            {
                tx.setPosition(crate.shake.startPosition.x, crate.shake.startPosition.y);
                tx.move(m_waveTable[crate.shake.shakeIndex], 0.f);
                crate.shake.shakeIndex = (crate.shake.shakeIndex + 1) % m_waveTable.size();
            }
        }
    }

    //update the respawn queue
    for (auto i = 0u; i < m_respawnCount; ++i)
    {
        m_respawnQueue[i].first -= dt;

        if (m_respawnQueue[i].first < 0)
        {
            spawn(m_respawnQueue[i].second);

            //swap last item in and resize count
            m_respawnCount--;
            m_respawnQueue[i] = m_respawnQueue[m_respawnCount];
            i--; //retest i because it's a different object
        }
    }  
}

//private
void CrateSystem::groundCollision(xy::Entity entity)
{
    const auto& collision = entity.getComponent<CollisionComponent>();
    auto hitboxCount = collision.getHitboxCount();
    const auto& hitboxes = collision.getHitboxes();

    auto& crate = entity.getComponent<Crate>();
    if (crate.state == Crate::Breaking)
    {
        //something else has modified the state so we no longer require
        //collision testing on this crate because it's already dooooomed
        return;
    }

    for (auto i = 0u; i < hitboxCount; ++i)
    {
        auto collisionCount = hitboxes[i].getCollisionCount();
        const auto& manifolds = hitboxes[i].getManifolds();

        if (hitboxes[i].getType() == CollisionType::Crate)
        {
            for (auto j = 0u; j < collisionCount; ++j)
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
                        if (!crate.lethal) //only take ownership if not moving (or at least slow enough to not kill)
                        {
                            crate.lastOwner = manifolds[j].otherEntity.getComponent<Player>().playerNumber;
                        }
                    }
                    break;
                case CollisionType::Powerup:
                    if (manifolds[j].otherEntity.hasComponent<Powerup>())
                    {
                        const auto& powerup = manifolds[j].otherEntity.getComponent<Powerup>();
                        if (powerup.state == Powerup::State::Active)
                        {
                            crate.lastOwner = powerup.owner;
                            destroy(entity);
                            return;
                        }
                    }
                    break;
                case CollisionType::NPC:
                    if (manifolds[j].otherEntity.hasComponent<NPC>())
                    {
                        auto l2 = xy::Util::Vector::lengthSquared(manifolds[j].otherEntity.getComponent<NPC>().velocity) * 2.f;
                        if (l2 > NpcMaxFallVelocity)
                        {
                            destroy(entity);
                            return;
                        }  
                    }
                    break;
                }

                if (/*crate.lethal || */manifolds[j].penetration > (CrateBounds.width / 2.f))
                {
                    destroy(entity);
                    return;
                }
            }
        }
        else if (hitboxes[i].getType() == CollisionType::Foot)
        {
            auto collisions = collisionCount;

            //remove any contacts which shouldn't stop it from falling
            for (auto j = 0u; j < collisionCount; ++j)
            {
                switch (manifolds[j].otherType)
                {
                default:break;
                case CollisionType::Powerup:
                    collisions--;
                    break;
                }
            }
            
            if (collisions == 0)
            {
                crate.groundContact = false;
                crate.state = Crate::Falling;
                crate.velocity.x *= 0.3f;
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
    if (crate.state == Crate::Breaking)
    {
        //something else has modified the state so we no longer require
        //collision testing on this crate because it's already dooooomed
        return;
    }

    for (auto i = 0u; i < hitboxCount; ++i)
    {
        auto collisionCount = hitboxes[i].getCollisionCount();
        const auto& manifolds = hitboxes[i].getManifolds();

        if (hitboxes[i].getType() == CollisionType::Crate)
        {
            for (auto j = 0u; j < collisionCount; ++j)
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
                    if ((crate.velocity.y) > std::abs(crate.velocity.x * 2.f)
                        && manifolds[j].normal.y < 0)
                    {
                        destroy(entity);
                    }
                    return;
                }
                else if (manifolds[j].otherType == CollisionType::Powerup)
                {
                    //ignore
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
            auto contactCount = collisionCount;
            for (auto j = 0u; j < collisionCount; ++j)
            {
                auto other = hitboxes[i].getManifolds()[j].otherType;
                switch (other)
                {
                default: break;
                case CollisionType::Player:
                case CollisionType::NPC:
                case CollisionType::Powerup:
                    contactCount--;
                    break;
                }
            }
            
            crate.groundContact = (contactCount != 0);
        }
    }
}

void CrateSystem::destroy(xy::Entity entity)
{
    if (entity.destroyed()) return;
    
    const auto& tx = entity.getComponent<xy::Transform>();  

    //broadcast to client
    ActorEvent evt;
    evt.actor = entity.getComponent<Actor>();
    evt.x = tx.getPosition().x;
    evt.y = tx.getPosition().y;
    evt.type = ActorEvent::Died;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

    const auto& crate = entity.getComponent<Crate>();

    auto spawnExplosion = [&, evt](sf::Uint8 owner, sf::Vector2f position) mutable
    {
        auto expEnt = getScene()->createEntity();
        expEnt.addComponent<Explosion>().owner = owner;
        expEnt.addComponent<xy::Transform>().setPosition(position);
        expEnt.getComponent<xy::Transform>().setOrigin(ExplosionOrigin);
        expEnt.addComponent<Actor>().id = expEnt.getIndex();
        expEnt.getComponent<Actor>().type = ActorID::Explosion;
        expEnt.addComponent<CollisionComponent>().addHitbox(ExplosionBounds, CollisionType::Explosion);
        expEnt.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Explosion);
        expEnt.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::ExplosionMask);
        expEnt.addComponent<xy::QuadTreeItem>().setArea(ExplosionBounds);
        expEnt.addComponent<AnimationController>();
        expEnt.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;

        //broadcast to clients
        evt.actor = expEnt.getComponent<Actor>();
        evt.x = position.x;
        evt.y = position.y;
        evt.type = ActorEvent::Spawned;
        evt.serverTime = GameServer::getServerTime();

        m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
    };

    if (crate.explosive)
    {
        //spawn explosion
        spawnExplosion(entity.getComponent<Crate>().lastOwner, { evt.x, evt.y });
    }
    else
    {
        //small chance might drop dynamite
        if (xy::Util::Random::value(0, 3) == 0)
        {
            auto dynEnt = getScene()->createEntity();
            dynEnt.addComponent<xy::Transform>().setPosition(evt.x, evt.y);
            dynEnt.getComponent<xy::Transform>().setOrigin(CrateOrigin);
            dynEnt.addComponent<Actor>().id = dynEnt.getIndex();
            dynEnt.getComponent<Actor>().type = ActorID::Dynamite;
            dynEnt.addComponent<AnimationController>();
            dynEnt.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;
            dynEnt.addComponent<Dynamite>().callback = spawnExplosion;
            dynEnt.getComponent<Dynamite>().velocity = { static_cast<float>(xy::Util::Random::value(-1, 1)) * 500.f, -300.f };
            dynEnt.getComponent<Dynamite>().lifetime += xy::Util::Random::value(0.1f, 0.5f);
            dynEnt.addComponent<CollisionComponent>().addHitbox(DynamiteBounds, CollisionType::Dynamite);
            dynEnt.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Dynamite);
            dynEnt.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::DynamiteMask);
            dynEnt.addComponent<xy::QuadTreeItem>().setArea(DynamiteBounds);

            //broadcast to clients
            evt.actor = dynEnt.getComponent<Actor>();
            evt.type = ActorEvent::Spawned;
            evt.serverTime = GameServer::getServerTime();

            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
        }
    }

    if (crate.respawn)
    {
        m_respawnQueue[m_respawnCount++] = std::make_pair(RespawnTime, crate);
    }

    getScene()->destroyEntity(entity);
}

void CrateSystem::spawn(Crate crate)
{
    auto entity = getScene()->createEntity();
    entity.addComponent<xy::Transform>().setPosition(crate.spawnPosition);
    entity.getComponent<xy::Transform>().setOrigin(CrateOrigin);
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = ActorID::Crate;

    entity.addComponent<xy::QuadTreeItem>().setArea(CrateBounds);

    entity.addComponent<AnimationController>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;

    crate.velocity = {};
    crate.lastOwner = 3;
    crate.lethal = false;
    crate.state = Crate::Falling;
    crate.groundContact = false;
    entity.addComponent<Crate>() = crate;

    entity.addComponent<CollisionComponent>().addHitbox(CrateBounds, CollisionType::Crate);
    entity.getComponent<CollisionComponent>().addHitbox(CrateFoot, CollisionType::Foot);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Crate);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::CrateMask);

    //let clients know
    ActorEvent evt;
    evt.actor = entity.getComponent<Actor>();
    evt.x = crate.spawnPosition.x;
    evt.y = crate.spawnPosition.y;
    evt.type = ActorEvent::Spawned;
    evt.serverTime = GameServer::getServerTime();

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
}

void CrateSystem::onEntityAdded(xy::Entity entity)
{
    entity.getComponent<Crate>().shake.shakeIndex = xy::Util::Random::value(0, m_waveTable.size() - 1);
    entity.getComponent<Crate>().shake.shakeTime = xy::Util::Random::value(4.f, Crate::PauseTime + 4.f);

    if (entity.getComponent<Crate>().explosive)
    {
        entity.getComponent<AnimationController>().nextAnimation = AnimationController::Walk;
    }
}
