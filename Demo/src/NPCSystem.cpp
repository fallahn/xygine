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

#include "NPCSystem.hpp"
#include "ActorSystem.hpp"
#include "BubbleSystem.hpp"
#include "AnimationController.hpp"
#include "Hitbox.hpp"
#include "ClientServerShared.hpp"
#include "PacketIDs.hpp"
#include "MessageIDs.hpp"
#include "CommandIDs.hpp"
#include "PowerupSystem.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/util/Random.hpp>

namespace
{
    const float WhirlybobSpeed = 100.f;
    const float ClocksySpeed = 86.f;
    const float GooblySpeed = 300.f;
    const float angryMultiplier = 2.f;
    const float initialJumpVelocity = 940.f;

    const std::array<float, 10> thinkTimes = { 20.f, 16.f, 12.f, 31.f, 15.4f, 14.9f, 25.f, 12.7f, 13.3f, 18.f };
    const float BubbleTime = 6.f;
    const float DieTime = 1.5f;
}

NPCSystem::NPCSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System        (mb, typeid(NPCSystem)),
    m_host              (host),
    m_currentThinkTime  (0)
{
    requireComponent<NPC>();
    requireComponent<Actor>();
    requireComponent<CollisionComponent>();
    requireComponent<xy::Transform>();
    requireComponent<AnimationController>();
}

//public
void NPCSystem::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::NpcMessage)
    {
        const auto& data = msg.getData<NpcEvent>();
        if (data.type == NpcEvent::Spawned)
        {
            auto entity = getScene()->getEntity(data.entityID);
            
            if (entity.getComponent<Actor>().type == ActorID::Goobly)
            {
                //send to clients
                ActorEvent evt;
                evt.actor = entity.getComponent<Actor>();
                evt.type = ActorEvent::Spawned;
                evt.x = entity.getComponent<xy::Transform>().getPosition().x;
                evt.y = entity.getComponent<xy::Transform>().getPosition().y;

                m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
            }            
        }
    }
}

void NPCSystem::process(float dt)
{    
    auto entities = getEntities();
    for (auto& entity : entities)
    {
        switch(entity.getComponent<NPC>().state)
        {
        case NPC::State::Bubble:
            updateBubbleState(entity, dt);
            break;
        case NPC::State::Dying:
            updateDyingState(entity, dt);
            break;
        default:
            switch (entity.getComponent<Actor>().type)
            {
            default: break;
            case ActorID::Clocksy:
                updateClocksy(entity, dt);
                break;
            case ActorID::Whirlybob:
                updateWhirlybob(entity, dt);
                break;
            case ActorID::Goobly:
                updateGoobly(entity, dt);
                break;
            }
        }
    }
}

//private
void NPCSystem::updateWhirlybob(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    auto& collision = entity.getComponent<CollisionComponent>();
    auto& hitboxes = collision.getHitboxes();

    for (auto i = 0u; i < collision.getHitboxCount(); ++i)
    {
        auto& manifolds = hitboxes[i].getManifolds();
        for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
        {
            auto& manifold = manifolds[j];
            switch (manifold.otherType)
            {
            default: break;
            case CollisionType::Solid:
            case CollisionType::Platform:
                tx.move(manifold.normal * manifold.penetration);
                npc.velocity = xy::Util::Vector::reflect(npc.velocity, manifold.normal);

                break;
            case CollisionType::Bubble:
                //switch to bubble state if bubble in spawn state
            {
                if (manifold.otherEntity.hasComponent<Bubble>())
                {
                    auto& bubble = manifold.otherEntity.getComponent<Bubble>();
                    if (bubble.state == Bubble::Spawning)
                    {
                        npc.lastVelocity = npc.velocity;

                        npc.state = NPC::State::Bubble;
                        npc.velocity.y = BubbleVerticalVelocity;
                        npc.thinkTimer = BubbleTime;
                        npc.bubbleOwner = bubble.player;
                        entity.getComponent<AnimationController>().direction = 1.f;
                        entity.getComponent<AnimationController>().nextAnimation =
                            (npc.bubbleOwner == 0) ? AnimationController::TrappedOne : AnimationController::TrappedTwo;

                        return;
                    }
                }
            }
                break;
            case CollisionType::Teleport:
                if (manifold.normal.y < 1)
                {
                    tx.move(0.f, -(TeleportDistance - NPCSize));
                }
                else
                {
                    tx.move(0.f, (TeleportDistance - NPCSize));
                }
                break;
            }
        }
    }

    //if (npc.state == NPC::State::Normal)
    {
        auto vel = npc.velocity * WhirlybobSpeed * dt;
        if (npc.angry) vel *= angryMultiplier;
        tx.move(vel);
        entity.getComponent<AnimationController>().nextAnimation = AnimationController::Idle;
    }
}

void NPCSystem::updateClocksy(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    float xMotion = tx.getPosition().x; //used in animation, see below

    const auto& collision = entity.getComponent<CollisionComponent>();
    const auto& hitboxes = collision.getHitboxes();

    for (auto i = 0u; i < collision.getHitboxCount(); ++i)
    {    
        if (hitboxes[i].getType() == CollisionType::NPC)
        {
            auto& manifolds = hitboxes[i].getManifolds();
            auto collisionCount = hitboxes[i].getCollisionCount();

            if(collisionCount == 0)
            {
                npc.canLand = true;
            }

            for (auto j = 0u; j < collisionCount; ++j)
            {
                auto& manifold = manifolds[j];
                switch (manifold.otherType)
                {
                default: break;
                case CollisionType::Platform:
                    //collide when falling downwards
                    if (manifold.normal.y < 0 && npc.canLand)
                    {
                        npc.velocity.y = 0.f;
                        if(xy::Util::Random::value(0, 1) == 0) npc.velocity.x = -npc.velocity.x;
                        npc.state = NPC::State::Normal;

                        tx.move(manifold.normal * manifold.penetration);
                        break;
                    }
                    npc.canLand = false;
                    break;
                case CollisionType::Solid:               
                    tx.move(manifold.normal * manifold.penetration);

                    if (npc.state == NPC::State::Jumping)
                    {
                        if (npc.velocity.y > 0)
                        {
                            //moving down
                            npc.state = NPC::State::Normal;
                            npc.velocity.y = 0.f;                            
                        }
                        else //bonk head
                        {
                            npc.velocity.y *= 0.25f;
                        }
                    }
                    npc.velocity = xy::Util::Vector::reflect(npc.velocity, manifold.normal);
                    break;
                case CollisionType::Bubble:
                    //switch to bubble state if bubble in spawn state
                {
                    if (manifold.otherEntity.hasComponent<Bubble>())
                    {
                        const auto& bubble = manifold.otherEntity.getComponent<Bubble>();
                        if (bubble.state == Bubble::Spawning)
                        {
                            npc.lastVelocity = npc.velocity; //so we can  restore if bubble pops

                            npc.state = NPC::State::Bubble;
                            npc.velocity.y = BubbleVerticalVelocity;
                            npc.thinkTimer = BubbleTime;
                            npc.bubbleOwner = bubble.player;
                            entity.getComponent<AnimationController>().direction = 1.f;
                            entity.getComponent<AnimationController>().nextAnimation =
                                (npc.bubbleOwner == 0) ? AnimationController::TrappedOne : AnimationController::TrappedTwo;
                            return;
                        }
                    }
                }
                    break;
                case CollisionType::Teleport:
                    tx.move(0.f, -TeleportDistance);
                    break;
                }
            }
        }
        else //footbox
        {
            if (hitboxes[i].getCollisionCount() == 0)
            {
                //foots in the air so we're falling
                npc.state = NPC::State::Jumping;
            }
        }  
    }

    npc.thinkTimer -= dt;

    switch (npc.state)
    {
    default: break;
    case NPC::State::Normal:
    {
        auto vel = npc.velocity * ClocksySpeed * dt;
        if (npc.angry) vel *= angryMultiplier;
        tx.move(vel);

        if (npc.thinkTimer < 0)
        {
            npc.thinkTimer = thinkTimes[m_currentThinkTime] * 0.065f;
            m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();
            npc.state = NPC::State::Thinking;
        }
    }
        break;
    case NPC::State::Jumping:
        npc.velocity.y += Gravity * dt;
        tx.move(npc.velocity * dt);
        break;

    case NPC::State::Thinking:
        if (npc.thinkTimer < 0)
        {
            npc.thinkTimer = thinkTimes[m_currentThinkTime];
            m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();
            
            if (xy::Util::Random::value(0,1) % 2 == 0)
            {
                npc.state = NPC::State::Normal;
                npc.velocity.x = -npc.velocity.x;
            }
            else
            {
                //do a jump
                npc.velocity.y = -initialJumpVelocity;
                npc.state = NPC::State::Jumping;
                tx.move(npc.velocity * dt);
            }
        }
        break;
    }

    //update animation state   
    auto& animController = entity.getComponent<AnimationController>();
    animController.direction = -npc.velocity.x;
    if (npc.state == NPC::State::Jumping)
    {
        animController.nextAnimation = (npc.velocity.y > 0) ? AnimationController::JumpDown : AnimationController::JumpUp;
    }
    else
    {
        xMotion = tx.getPosition().x - xMotion;
        animController.nextAnimation = (xMotion == 0) ? AnimationController::Idle : AnimationController::Walk;
    }
}

void NPCSystem::updateGoobly(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    auto& collision = entity.getComponent<CollisionComponent>();
    auto& hitboxes = collision.getHitboxes();

    for (auto i = 0u; i < collision.getHitboxCount(); ++i)
    {
        auto& manifolds = hitboxes[i].getManifolds();
        for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
        {
            auto& manifold = manifolds[j];
            switch (manifold.otherType)
            {
            default: break;
            case CollisionType::Solid:
            //case CollisionType::Platform:
                tx.move(manifold.normal * manifold.penetration);
                npc.velocity = xy::Util::Vector::reflect(npc.velocity, manifold.normal);

                break;
            case CollisionType::Player:
            {
                auto player = manifold.otherEntity.getComponent<Player>();
                if (player.state != Player::State::Dead && player.state != Player::State::Dying)
                {
                    if (player.timer > 0)
                    {
                        //player is invincible so kill goobly
                        getScene()->destroyEntity(entity);

                        //broadcast to client
                        ActorEvent evt;
                        evt.actor.id = entity.getIndex();
                        evt.actor.type = entity.getComponent<Actor>().type;
                        evt.x = tx.getPosition().x;
                        evt.y = tx.getPosition().y;
                        evt.type = ActorEvent::Died;

                        m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

                        //raise message
                        auto* msg = postMessage<SceneEvent>(MessageID::SceneMessage);
                        msg->actorID = evt.actor.type;
                        msg->type = SceneEvent::ActorRemoved;
                        msg->entity = entity;
                        msg->x = evt.x;
                        msg->y = evt.y;
                        break;
                    }
                }
            }
                break;
            case CollisionType::Teleport:
                if (manifold.normal.y < 1)
                {
                    tx.move(0.f, -(TeleportDistance - NPCSize));
                }
                else
                {
                    tx.move(0.f, (TeleportDistance - NPCSize));
                }
                break;
            }
        }
    }


    //do thinks
    npc.thinkTimer -= dt;
    if (npc.state == NPC::State::Normal)
    {
        auto vel = npc.velocity * dt;
        //if (npc.angry) vel *= angryMultiplier;
        tx.move(vel);
        entity.getComponent<AnimationController>().nextAnimation = AnimationController::Idle;

        if (npc.thinkTimer < 0)
        {
            npc.state = NPC::State::Thinking;
            npc.thinkTimer = thinkTimes[m_currentThinkTime] * 0.08f;
            m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();
        }
    }
    else if (npc.state == NPC::State::Thinking)
    {
        //get a new velocity from direction to player

        if (npc.thinkTimer < 0)
        {
            npc.state = NPC::State::Normal;
            npc.thinkTimer = thinkTimes[m_currentThinkTime] * 0.055f;
            m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();

            //get target position and move on either x or y axis
            if (npc.target.hasComponent<xy::Transform>())
            {
                auto dir = npc.target.getComponent<xy::Transform>().getPosition() - tx.getPosition();
                if (xy::Util::Random::value(0, 1) == 0)
                {
                    npc.velocity.x = dir.x / npc.thinkTimer;
                    npc.velocity.y = 0.f;
                }
                else
                {
                    npc.velocity.x = dir.x > 0 ? 0.01f : -0.01f;
                    npc.velocity.y = dir.y / npc.thinkTimer;
                }
            }
        }
    }

    //update animation state   
    auto& animController = entity.getComponent<AnimationController>();
    if (npc.velocity.x < 0) animController.direction = 1.f;
    else if (npc.velocity.x > 0) animController.direction = -1.f;
}

void NPCSystem::updateBubbleState(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    const auto& collision = entity.getComponent<CollisionComponent>();
    const auto& hitboxes = collision.getHitboxes();

    for (auto i = 0u; i < collision.getHitboxCount(); ++i)
    {
        auto& manifolds = hitboxes[i].getManifolds();
        for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
        {
            auto& manifold = manifolds[j];
            switch (manifold.otherType)
            {
            default: break;
            case CollisionType::Solid:
            case CollisionType::Platform:
                tx.move(manifold.normal * manifold.penetration);
                break;
            case CollisionType::Player:
                //kill ent
            {
                const auto& player = manifold.otherEntity.getComponent<Player>();
                if (player.playerNumber == npc.bubbleOwner && player.state == Player::State::Jumping)
                {
                    despawn(entity, player.playerNumber);
                }
            }
                return;
            case CollisionType::Teleport:
                if (manifold.normal.y < 1)
                {
                    tx.move(0.f, -TeleportDistance);
                }
                else
                {
                    tx.move(0.f, TeleportDistance);
                }
                break;
            }
        }
    }

    tx.move(npc.velocity * dt);

    npc.thinkTimer -= dt;
    if (npc.thinkTimer < 0)
    {
        //bubble wasn't burst in time, release NPC (angry mode)
        npc.state = NPC::State::Normal;
        npc.velocity = npc.lastVelocity;
        npc.thinkTimer = thinkTimes[m_currentThinkTime];
        npc.angry = true;
        
        //broadcast anger
        /*auto* msg = postMessage<NpcEvent>(MessageID::NpcMessage);
        msg->type = NpcEvent::GotAngry;
        msg->entityID = entity.getIndex();*/

        ActorEvent evt;
        evt.type = ActorEvent::GotAngry;
        evt.actor = entity.getComponent<Actor>();
        m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

        m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();
    }
}

void NPCSystem::updateDyingState(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    const auto& collision = entity.getComponent<CollisionComponent>();
    const auto& hitboxes = collision.getHitboxes();

    for (auto i = 0u; i < collision.getHitboxCount(); ++i)
    {
        auto& manifolds = hitboxes[i].getManifolds();
        for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
        {
            auto& manifold = manifolds[j];

            if (npc.thinkTimer > 0)
            {
                if (manifold.otherType == CollisionType::Solid)
                {
                    tx.move(manifold.normal * manifold.penetration);
                    npc.velocity = xy::Util::Vector::reflect(npc.velocity, manifold.normal);
                    npc.velocity *= 0.8f;
                }
                else if (manifold.otherType == CollisionType::Teleport)
                {
                    tx.move(0.f, -TeleportDistance);
                }
            }
            else
            {
                //we can settle
                if (((manifolds[j].otherType == CollisionType::Solid) || (manifolds[j].otherType == CollisionType::Platform))
                    && manifolds[j].normal.y < 0)
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

                    //raise message
                    auto* msg = postMessage<SceneEvent>(MessageID::SceneMessage);
                    msg->actorID = evt.actor.type;
                    msg->type = SceneEvent::ActorRemoved;
                    msg->entity = entity;
                    msg->x = evt.x;
                    msg->y = evt.y;
                    break;
                }
            }   
        }
    }

    npc.thinkTimer -= dt;
    tx.move(npc.velocity * dt);
    if (npc.velocity.y < MaxVelocity)
    {
        npc.velocity.y += Gravity * dt;
    }
}

void NPCSystem::onEntityAdded(xy::Entity /*entity*/)
{
    /*entity.getComponent<NPC>().thinkTimer = thinkTimes[m_currentThinkTime];

    m_currentThinkTime = (m_currentThinkTime + 1) % thinkTimes.size();*/
}

void NPCSystem::despawn(xy::Entity entity, sf::Uint8 playerNumber)
{
    XY_ASSERT(entity.hasComponent<NPC>(), "Not an NPC");

    if (entity.getComponent<Actor>().type == ActorID::Goobly) return;
    
    auto& npc = entity.getComponent<NPC>();
    npc.state = NPC::State::Dying;
    npc.thinkTimer = DieTime;

    auto position = entity.getComponent<xy::Transform>().getPosition();
    npc.velocity.x = (position.x > MapBounds.width / 2.f) ? -90.f : 90.f;
    npc.velocity.y = (position.y > MapBounds.height / 2.f) ? -300.f : -230.f;

    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::NPCMask & ~(CollisionFlags::Player | CollisionFlags::Powerup));

    //raise message
    auto* msg = postMessage<NpcEvent>(MessageID::NpcMessage);
    msg->type = NpcEvent::Died;
    msg->entityID = entity.getIndex();
    msg->playerID = playerNumber;
}