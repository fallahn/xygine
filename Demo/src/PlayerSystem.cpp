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

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/core/App.hpp>

#include <xyginext/ecs/components/Text.hpp>

namespace
{
    const float speed = 280.f;
    const float initialJumpVelocity = 840.f;
    const float minJumpVelocity = -initialJumpVelocity * 0.25f; //see http://info.sonicretro.org/SPG:Jumping#Jump_Velocit
    const float dyingTime = 2.f;
    const float invincibleTime = 2.f;
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
        resolveCollision(entity);

        auto& player = entity.getComponent<Player>();
        auto& tx = entity.getComponent<xy::Transform>();

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
            auto delta = getDelta(player.history, player.lastUpdatedInput);
            auto currentMask = player.history[player.lastUpdatedInput].mask;

            //let go of jump so enable jumping again
            if ((currentMask & InputFlag::Up) == 0)
            {
                player.canJump = true;
            }

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

                    auto* msg = postMessage<PlayerEvent>(MessageID::PlayerMessage);
                    msg->type = PlayerEvent::Jumped;
                    msg->entity = entity;
                }
            }
            else if(player.state == Player::State::Jumping)
            {
                //variable jump height
                if ((currentMask & InputFlag::Up) == 0
                    && player.velocity.y < minJumpVelocity)
                {
                    player.velocity.y = minJumpVelocity;
                }
                
                tx.move({ 0.f, player.velocity.y * delta });
                player.velocity.y += gravity * delta;
                player.velocity.y = std::min(player.velocity.y, maxVelocity);
            }
            else
            {
                //dying
                player.velocity.y += gravity * delta;
                player.velocity.y = std::min(player.velocity.y, maxVelocity);
                tx.move({ 0.f, player.velocity.y * delta });
                player.canShoot = false;

                if (player.timer < 0) //respawn
                {
                    tx.setPosition(player.spawnPosition);
                    player.state = Player::State::Walking;
                    player.direction = 
                        (player.spawnPosition.x < (MapBounds.width / 2.f)) ? Player::Direction::Right : Player::Direction::Left;

                    player.timer = invincibleTime;

                    //TODO raise message
                }
            }
            
            //move with input if not dying
            if (player.state != Player::State::Dying)
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
        else
        {
            animController.nextAnimation = AnimationController::Die;
        }
    }
}

void PlayerSystem::reconcile(const ClientState& state, xy::Entity entity)
{
    //DPRINT("Reconcile to: ", std::to_string(x) + ", " + std::to_string(y));

    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();

    tx.setPosition(state.x, state.y);
    player.state = state.playerState;
    player.velocity.y = state.playerVelocity;
    player.timer = state.playerTimer;

    //find the oldest timestamp not used by server
    auto ip = std::find_if(player.history.rbegin(), player.history.rend(),
        [&state](const Input& input)
    {
        return (state.clientTime == input.timestamp);
    });

    //and reparse inputs
    if (ip != player.history.rend())
    {
        std::size_t idx =  std::distance(player.history.begin(), ip.base()) - 1;
        auto end = (player.currentInput + player.history.size() - 1) % player.history.size();

        while (idx != end) //currentInput points to the next free slot in history
        {
            float delta = getDelta(player.history, idx);
            
            //let go of jump so enable jumping again
            if ((player.history[idx].mask & InputFlag::Up) == 0)
            {
                player.canJump = true;
            }

            player.timer -= delta;
            if (player.state == Player::State::Walking)
            {
                if (player.history[idx].mask & InputFlag::Up
                    && player.canJump)
                {
                    player.state = Player::State::Jumping;
                    player.velocity.y = -initialJumpVelocity;
                }
            }
            else if(player.state == Player::State::Jumping)
            {
                if ((player.history[idx].mask & InputFlag::Up) == 0
                    && player.velocity.y < minJumpVelocity)
                {
                    player.velocity.y = minJumpVelocity;
                }                
                
                tx.move({ 0.f, player.velocity.y * delta });
                player.velocity.y += gravity * delta;
                player.velocity.y = std::min(player.velocity.y, maxVelocity);
            }
            else
            {
                //dying
                player.velocity.y += gravity * delta;
                player.velocity.y = std::min(player.velocity.y, maxVelocity);
                tx.move({ 0.f, player.velocity.y * delta });             
            }


            if (player.state != Player::State::Dying)
            {
                auto motion = parseInput(player.history[idx].mask);
                tx.move(speed * motion * delta);
                player.velocity.x = speed * motion.x;
                if (motion.x > 0)
                {
                    player.direction = Player::Direction::Right;
                }
                else if (motion.x < 0)
                {
                    player.direction = Player::Direction::Left;
                }
            }
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

    //normalise if not along one axis
    /*if (xy::Util::Vector::lengthSquared(motion) > 1)
    {
        motion = xy::Util::Vector::normalise(motion);
    }*/

    return motion;
}

float PlayerSystem::getDelta(const History& history, std::size_t idx)
{
    auto prevInput = (idx + history.size() - 1) % history.size();
    auto delta = history[idx].timestamp - history[prevInput].timestamp;

    return static_cast<float>(delta) / 1000000.f;
}

void PlayerSystem::resolveCollision(xy::Entity entity)
{
    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();
    const auto& hitboxes = entity.getComponent<CollisionComponent>().getHitboxes();
    auto count = entity.getComponent<CollisionComponent>().getHitboxCount();

    //check for collision and resolve
    for (auto i = 0u; i < count; ++i)
    {
        const auto& hitbox = hitboxes[i];
        if (hitbox.getType() == CollisionType::Player)
        {
            auto collisionCount = hitbox.getCollisionCount();
            if (collisionCount == 0)
            {
                player.canLand = true; //only land on a platform once we've been in freefall
            }

            for (auto i = 0u; i < collisionCount; ++i)
            {
                const auto& man = hitbox.getManifolds()[i];
                switch (man.otherType)
                {
                default: break;
                //case CollisionType::Bubble:
                case CollisionType::Platform:
                    //only collide when moving downwards (one way plat)
                    if (man.normal.y < 0 && player.canLand)
                    {
                        player.velocity.y = 0.f;
                        if (player.state == Player::State::Jumping)
                        {
                            //don't switch if we're dying
                            player.state = Player::State::Walking;
                        }

                        tx.move(man.normal * man.penetration);
                        break;
                    }
                    player.canLand = false;
                    break;
                case CollisionType::Solid:
                    //always repel
                    tx.move(man.normal * man.penetration);
                    if (man.normal.y < 0 && player.velocity.y > 0)
                    {
                        player.velocity.y = 0.f;
                        if (player.state == Player::State::Jumping)
                        {
                            player.state = Player::State::Walking;
                        }
                    }
                    else if (man.normal.y > 0)
                    {
                        player.velocity = -player.velocity * 0.25f;
                    }
                    break;
                case CollisionType::Teleport:
                    if (tx.getPosition().y > xy::DefaultSceneSize.y / 2.f)
                    {
                        //move up
                        tx.move(0.f, -TeleportDistance);
                    }
                    else
                    {
                        tx.move(0.f, TeleportDistance);
                    }
                    break;
                case CollisionType::NPC:
                    if (player.state != Player::State::Dying
                        && player.timer < 0)
                    {
                        const auto& npc = man.otherEntity.getComponent<NPC>();
                        if (npc.state != NPC::State::Bubble && npc.state != NPC::State::Dying)
                        {
                            player.state = Player::State::Dying;
                            player.timer = dyingTime;

                            entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;

                            //TODO raise dead message

                            return;
                        }
                    }
                    break;
                }
            }
        }
        else if (hitbox.getType() == CollisionType::Foot
            && player.state != Player::State::Dying)
        {
            //foot sensor
            auto count = hitbox.getCollisionCount();
            if (count == 0)
            {
                player.state = Player::State::Jumping; //start falling when nothing underneath
            }
        }
    }
}
