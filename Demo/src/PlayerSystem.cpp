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

#include "PlayerSystem.hpp"
#include "AnimationController.hpp"
#include "MapData.hpp"
#include "Hitbox.hpp"
#include "MessageIDs.hpp"
#include "ClientServerShared.hpp"
#include "NPCSystem.hpp"
#include "CollisionSystem.hpp"
#include "BubbleSystem.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/core/App.hpp>


namespace
{
    const float speed = 280.f;
    const float initialJumpVelocity = 840.f;
    const float minJumpVelocity = -initialJumpVelocity * 0.25f; //see http://info.sonicretro.org/SPG:Jumping#Jump_Velocit
    const float dyingTime = 2.f;
    const float invincibleTime = 2.f;

    const sf::Uint32 UpMask = CollisionFlags::PlayerMask & ~(CollisionFlags::Bubble/*|CollisionFlags::Platform*/);
    const sf::Uint32 DownMask = CollisionFlags::PlayerMask;

    const sf::Uint32 FootMask = (CollisionType::Platform | CollisionType::Solid | CollisionType::Bubble);

    const sf::Uint8 BodyClear = 0x1;
    const sf::Uint8 FootClear = 0x2;
    const sf::Uint8 PlayerClear = BodyClear | FootClear;
}

PlayerSystem::PlayerSystem(xy::MessageBus& mb, bool server)
    : xy::System(mb, typeid(PlayerSystem)),
    m_isServer(server)
{
    requireComponent<Player>();
    requireComponent<xy::Transform>();
    requireComponent<CollisionComponent>();
    requireComponent<AnimationController>();
}

//public
void PlayerSystem::process(float)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        //resolveCollision(entity);

        auto& player = entity.getComponent<Player>();
        auto& tx = entity.getComponent<xy::Transform>();
        const auto& collision = entity.getComponent<CollisionComponent>();

        float xMotion = tx.getPosition().x; //used for animation, below

        //check  the player hasn't managed to warp out of the gameplay area for some reason
        if (!MapBounds.contains(tx.getPosition()))
        {
            tx.setPosition(player.spawnPosition);
        }


        //current input actually points to next empty slot.
        std::size_t idx = (player.currentInput + player.history.size() - 1) % player.history.size();

        //parse any outstanding inputs
        while (player.lastUpdatedInput != idx)
        {   
            resolveCollision(entity);
            
            //record the collision state used for this calculation
            //this isn't strictly correct during reconciliation. What we really need
            //is to store the state of the collision world at this time so it can be requeried
            player.history[player.lastUpdatedInput].collision = collision;

            auto delta = getDelta(player.history, player.lastUpdatedInput);
            auto currentMask = player.history[player.lastUpdatedInput].input.mask;

            //if shoot was pressed but not last frame, raise message
            //note this is NOT reconciled and ammo actors are entirely server side
            if ((currentMask & InputFlag::Shoot) && player.canShoot)
            {
                auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
                msg->entity = entity;
                msg->type = PlayerEvent::FiredWeapon;

                player.canShoot = false;
            }
            else if ((currentMask & InputFlag::Shoot) == 0
                && player.state != Player::State::Dying)
            {
                player.canShoot = true;
            }

            processInput(currentMask, delta, entity);

            //probably should be server side only?
            if (player.state == Player::State::Dying && player.timer < 0) //respawn
            {
                if (player.lives)
                {
                    tx.setPosition(player.spawnPosition);
                    player.state = Player::State::Walking;
                    player.direction =
                        (player.spawnPosition.x < (MapBounds.width / 2.f)) ? Player::Direction::Right : Player::Direction::Left;

                    player.timer = invincibleTime;
                }
                else
                {
                    player.state = Player::State::Dead;
                    entity.getComponent<CollisionComponent>().setCollisionMaskBits(0); //remove collision
                }
                player.velocity.y = 0.f;
            }

            player.lastUpdatedInput = (player.lastUpdatedInput + 1) % player.history.size();
        }

        //update animation state
        auto& animController = entity.getComponent<AnimationController>();
        animController.direction = (player.direction == Player::Direction::Left) ? 1.f : -1.f;
        if (player.state == Player::State::Jumping)
        {
            animController.nextAnimation = (player.velocity.y > 0) ? AnimationController::JumpDown : AnimationController::JumpUp;
        }
        else if(player.state == Player::State::Walking)
        {
            xMotion = tx.getPosition().x - xMotion;
            animController.nextAnimation = (xMotion == 0) ? AnimationController::Idle : AnimationController::Walk;
        }
        else if(player.state != Player::State::Disabled)
        {
            animController.nextAnimation = AnimationController::Die;
        }

        //TODO map change animation
#ifdef XY_DEBUG
        if (!m_isServer)
        {
            switch (player.state)
            {
            default:
                DPRINT("Player state", std::to_string((int)player.state));
                break;
            case Player::State::Disabled:
                DPRINT("Player state", "Disabled");
                break;
            case Player::State::Dying:
                DPRINT("Player state", "Dying");
                break;
            case Player::State::Jumping:
                DPRINT("Player state", "Jumping");
                break;
            case Player::State::Walking:
                DPRINT("Player state", "Walking");
                break;
            }
        }
#endif

    }
}

void PlayerSystem::reconcile(const ClientState& state, xy::Entity entity)
{
    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();

    tx.setPosition(state.x, state.y);
    player.state = state.playerState;
    player.velocity.y = state.playerVelocity;
    player.timer = state.playerTimer;
    player.canJump = state.playerCanJump;
    player.canLand = state.playerCanLand;
    player.canRideBubble = state.playerCanRideBubble;

    //find the oldest timestamp not used by server
    auto ip = std::find_if(player.history.rbegin(), player.history.rend(),
        [&state](const HistoryState& hs)
    {
        return (state.clientTime == hs.input.timestamp);
    });

    //and reparse inputs
    if (ip != player.history.rend())
    {
        std::size_t idx = std::distance(player.history.begin(), ip.base()) -1;
        auto end = (player.currentInput + player.history.size() - 1) % player.history.size();

        while (idx != end) //currentInput points to the next free slot in history
        {                      
            //restore the collision data from history
            entity.getComponent<CollisionComponent>() = player.history[idx].collision;
            resolveCollision(entity);
            
            float delta = getDelta(player.history, idx);
            auto currentMask = player.history[idx].input.mask;
            
            processInput(currentMask, delta, entity);

            idx = (idx + 1) % player.history.size();
        }
    }

    //update resulting animation
    auto& animController = entity.getComponent<AnimationController>();
    animController.direction = (player.direction == Player::Direction::Left) ? 1.f : -1.f;
    if (player.state == Player::State::Jumping)
    {
        animController.nextAnimation = (player.velocity.y > 0) ? AnimationController::JumpDown : AnimationController::JumpUp;
    }
    else
    {
        float xMotion = tx.getPosition().x - state.x;
        animController.nextAnimation = (xMotion == 0) ? AnimationController::Idle : AnimationController::Walk;
    }
}

//private
sf::Vector2f PlayerSystem::parseInput(sf::Uint16 mask)
{
    sf::Vector2f motion;
    if (mask & InputFlag::Left)
    {
        motion.x = -1.f;
    }
    if (mask & InputFlag::Right)
    {
        motion.x += 1.f;
    }

    return motion;
}

float PlayerSystem::getDelta(const History& history, std::size_t idx)
{
    auto prevInput = (idx + history.size() - 1) % history.size();
    auto delta = history[idx].input.timestamp - history[prevInput].input.timestamp;

    return static_cast<float>(delta) / 1000000.f;
}

void PlayerSystem::processInput(sf::Uint16 currentMask, float delta, xy::Entity entity)
{
    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();
    
    //let go of jump so enable jumping again
    if ((currentMask & InputFlag::Up) == 0)
    {
        player.canJump = true;
    }

    //state specific...
    player.timer -= delta;
    if (player.state == Player::State::Walking)
    {
        if ((currentMask & InputFlag::Up)
            && player.canJump)
        {
            player.state = Player::State::Jumping;
            player.velocity.y = -initialJumpVelocity;
            player.canJump = false;

            entity.getComponent<AnimationController>().nextAnimation = AnimationController::JumpUp;

            //disable platform and bubble collision
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(UpMask);

            auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
            msg->type = PlayerEvent::Jumped;
            msg->entity = entity;
        }
    }
    else if (player.state == Player::State::Jumping)
    {
        //variable jump height
        if ((currentMask & InputFlag::Up) == 0
            && player.velocity.y < minJumpVelocity)
        {
            player.velocity.y = minJumpVelocity;
        }

        tx.move({ 0.f, player.velocity.y * delta });
        player.velocity.y += Gravity * delta;
        player.velocity.y = std::min(player.velocity.y, MaxVelocity);

        //reenable downward collision
        if (player.velocity.y > 0)
        {
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(DownMask);
        }
    }
    else if (player.state == Player::State::Dying)
    {
        //dying
        player.velocity.y += Gravity * delta;
        player.velocity.y = std::min(player.velocity.y, MaxVelocity);
        tx.move({ 0.f, player.velocity.y * delta });
        player.canShoot = false;
    }

    //move with input if not dying
    if (player.state == Player::State::Walking || player.state == Player::State::Jumping)
    {
        auto motion = parseInput(currentMask);
        tx.move(speed * motion * delta);
        player.velocity.x = speed * motion.x; //used to decide how much velocity to add to bubble spawn

        if (motion.x > 0)
        {
            player.direction = Player::Direction::Right;
        }
        else if (motion.x < 0)
        {
            player.direction = Player::Direction::Left;
        }
    }
}

void PlayerSystem::resolveCollision(xy::Entity entity)
{
    auto& player = entity.getComponent<Player>();
    switch (player.state)
    {
    default:break;
    case Player::State::Disabled:
        return;
    case Player::State::Dying:
        collisionDying(entity);
        return;
    case Player::State::Jumping:
        collisionJumping(entity);
        return;
    case Player::State::Walking:
        collisionWalking(entity);
        return;
    }
}

void PlayerSystem::collisionWalking(xy::Entity entity)
{
    const auto& hitboxes = entity.getComponent<CollisionComponent>().getHitboxes();
    auto hitboxCount = entity.getComponent<CollisionComponent>().getHitboxCount();

    auto& tx = entity.getComponent<xy::Transform>();
    auto& player = entity.getComponent<Player>();

    //check for collision and resolve
    for (auto i = 0u; i < hitboxCount; ++i)
    {
        const auto& hitbox = hitboxes[i];
        if (hitbox.getType() == CollisionType::Player)
        {
            auto& manifolds = hitbox.getManifolds();
            auto collisionCount = hitbox.getCollisionCount();
            for (auto j = 0u; j < collisionCount; ++j)
            {
                const auto& man = manifolds[j];
                switch (man.otherType)
                {
                default: break;
                case CollisionType::Bubble:
                    if (!player.canRideBubble)
                    {
                        break;
                    }

                    if (man.otherEntity.hasComponent<Bubble>() &&
                        man.otherEntity.getComponent<Bubble>().state == Bubble::Normal)
                    {
                        if (man.normal.y < 0)
                        {
                            tx.move(man.normal * man.penetration);
                        }
                    }

                    break;
                case CollisionType::Solid:
                case CollisionType::Platform:
                    tx.move(man.normal * man.penetration);
                    break;
                case CollisionType::NPC:
                    npcCollision(entity, man);
                    break;
                }
            }
        }
        else if (hitbox.getType() == CollisionType::Foot)
        {
            if (hitbox.getCollisionCount() == 0)
            {
                entity.getComponent<Player>().state = Player::State::Jumping; //enter freefall
            }
        }
    }
}

void PlayerSystem::collisionJumping(xy::Entity entity)
{
    const auto& hitboxes = entity.getComponent<CollisionComponent>().getHitboxes();
    auto hitboxCount = entity.getComponent<CollisionComponent>().getHitboxCount();

    auto& player = entity.getComponent<Player>();    
    auto& tx = entity.getComponent<xy::Transform>();

    player.canRideBubble = false;

    //check for collision and resolve
    for (auto i = 0u; i < hitboxCount; ++i)
    {
        const auto& hitbox = hitboxes[i];
        if (hitbox.getType() == CollisionType::Player)
        {
            auto manifolds = hitbox.getManifolds();
            auto collisionCount = hitbox.getCollisionCount();

            if (collisionCount == 0)
            {
                player.canLand |= BodyClear;
            }
            else
            {
                //force bubble / plat checks last so we can tell if body really is clear
                std::sort(manifolds.begin(), manifolds.begin() + collisionCount,
                    [](const Manifold& a, const Manifold& b)
                {
                    return a.otherType < b.otherType;
                });
            }

            for (auto j = 0u; j < collisionCount; ++j)
            {
                const auto& man = manifolds[j];
                switch (man.otherType)
                {
                default: 
                    player.canLand |= BodyClear;
                    break;
                case CollisionType::Bubble:
                    player.canRideBubble = true;
                case CollisionType::Platform:
                    /*if (man.normal.x != 0)
                    {
                        tx.move(man.normal * man.penetration);
                    }                    
                    else*/ if ((player.canLand & PlayerClear)&& player.velocity.y > 0 && man.normal.y < 0)
                    {
                        player.state = Player::State::Walking;
                        player.velocity.y = 0.f;
                        tx.move(man.normal * man.penetration);
                        return; //quit when we change state because this function is no longer valid
                    }

                    player.canLand &= ~BodyClear;
                    break;
                case CollisionType::Solid:
                    if (man.normal.y < 0 && player.velocity.y > 0)
                    {
                        player.state = Player::State::Walking;
                        player.velocity.y = 0.f;
                        return;
                    }
                    else if (man.normal.y > 0) //bonk head and fall
                    {
                        player.velocity = -player.velocity * 0.25f;
                    }
                    tx.move(man.normal * man.penetration);
                    //player.canLand &= ~BodyClear;
                    break;
                case CollisionType::Teleport:
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
                    return;
                case CollisionType::NPC:
                    npcCollision(entity, man);
                    break;
                }
            }
        }
        else if (hitbox.getType() == CollisionType::Foot)
        {
            auto manifolds = hitbox.getManifolds();
            auto collisionCount = hitbox.getCollisionCount();

            for (auto j = 0u; j < collisionCount; ++j)
            {
                auto man = manifolds[j];
                if (man.otherType == CollisionType::Platform)
                {
                    if (player.velocity.y < 0)
                    {
                        player.canLand &= ~FootClear;
                    }
                    else
                    {
                        player.canLand |= FootClear;
                    }
                }
                else
                {
                    player.canLand &= ~FootClear;
                }
            }
        }
    }
}

void PlayerSystem::collisionDying(xy::Entity entity)
{
    const auto& hitboxes = entity.getComponent<CollisionComponent>().getHitboxes();
    auto hitboxCount = entity.getComponent<CollisionComponent>().getHitboxCount();

    //check for collision and resolve
    for (auto i = 0u; i < hitboxCount; ++i)
    {
        const auto& hitbox = hitboxes[i];
        if (hitbox.getType() == CollisionType::Player)
        {
            auto& manifolds = hitbox.getManifolds();
            auto collisionCount = hitbox.getCollisionCount();

            for (auto j = 0u; j < collisionCount; ++j)
            {
                const auto& man = manifolds[j];
                if ((man.otherType & (CollisionType::Solid | CollisionType::Platform)))
                {
                    entity.getComponent<xy::Transform>().move(man.normal * man.penetration);
                    entity.getComponent<Player>().velocity.y = 0.f;
                }
            }
        }
    }
}

void PlayerSystem::npcCollision(xy::Entity entity, const Manifold& man)
{
    auto& player = entity.getComponent<Player>();
    if (player.timer < 0 && man.otherEntity.hasComponent<NPC>())
    {
        const auto& npc = man.otherEntity.getComponent<NPC>();
        if (npc.state != NPC::State::Bubble && npc.state != NPC::State::Dying)
        {
            player.state = Player::State::Dying;
            player.timer = dyingTime;

            entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;

            player.lives--;

            //remove collision if player has no lives left
            if (!player.lives)
            {
                entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Solid | CollisionFlags::Platform);

                //and kill any chasing goobly
                xy::Command cmd;
                cmd.targetFlags = CommandID::NPC;
                cmd.action = [&, entity](xy::Entity npcEnt, float)
                {
                    if (npcEnt.getComponent<NPC>().target == entity)
                    {
                        getScene()->destroyEntity(npcEnt);
                    }
                };
                getScene()->getSystem<xy::CommandSystem>().sendCommand(cmd);
            }

            //raise dead message
            auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
            msg->entity = entity;
            msg->type = PlayerEvent::Died;

            return;
        }
    }
}