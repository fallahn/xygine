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
#include "MapData.hpp"
#include "Hitbox.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/core/App.hpp>

namespace
{
    const float speed = 400.f;
    const float maxVelocity = 800.f;
    const float gravity = 2200.f;
    const float initialJumpVelocity = 900.f;
    const float minJumpVelocity = -initialJumpVelocity * 0.35f; //see http://info.sonicretro.org/SPG:Jumping#Jump_Velocity
    const float teleportDist = (15.f * 64.f) - 10.f; //TODO hook this in with map properties somehow
}

PlayerSystem::PlayerSystem(xy::MessageBus& mb, bool server)
    : xy::System(mb, typeid(PlayerSystem)),
    m_isServer(server)
{
    requireComponent<Player>();
    requireComponent<xy::Transform>();
    requireComponent<CollisionComponent>();
}

//public
void PlayerSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& player = entity.getComponent<Player>();
        auto& tx = entity.getComponent<xy::Transform>();
        const auto& hitboxes = entity.getComponent<CollisionComponent>().getHitboxes();
        auto count = entity.getComponent<CollisionComponent>().getHitboxCount();

        //check for collision and resolve
        for (auto i = 0u; i < count; ++i)
        {
            const auto& hitbox = hitboxes[i];
            if(hitbox.getType() == CollisionType::Player)
            {
                for (auto i = 0u; i < hitbox.getCollisionCount(); ++i)
                {
                    const auto& man = hitbox.getManifolds()[i];
                    switch (man.otherType)
                    {
                    default: break;
                    case CollisionType::Platform:
                        //only collide when moving downwards (one way plat)
                        if (man.normal.y < 0 && player.velocity > 0)
                        {
                            player.velocity = 0.f;
                            player.state = Player::State::Walking;

                            tx.move(man.normal * man.penetration);
                            break;
                        }
                        break;
                    case CollisionType::Solid:
                        //always repel
                        tx.move(man.normal * man.penetration);
                        if (man.normal.y < 0 && player.velocity > 0)
                        {
                            player.velocity = 0.f;
                            player.state = Player::State::Walking;
                        }
                        else if (man.normal.y > 0)
                        {
                            player.velocity = -player.velocity * 0.25f;
                        }
                        break;
                    case CollisionType::Teleport:
                        if (man.normal.y < 0)
                        {
                            //move up
                            tx.move(0.f, -teleportDist);
                        }
                        break;
                    }
                }
            }
            else if (hitbox.getType() == CollisionType::Foot)
            {
                //foot sensor
                auto count = hitbox.getCollisionCount();
                if (count == 0)
                {
                    player.state = Player::State::Jumping; //start falling when nothing underneath
                }
                else
                {
                    for (auto i = 0u; i < count; ++i)
                    {
                        const auto& man = hitbox.getManifolds()[i];
                        switch (man.otherType)
                        {
                        default: break;
                        case CollisionType::Solid: break;
                        case CollisionType::Platform:
                            //if(player.velocity > 0) player.velocity = 0.2f;
                            break;
                        case CollisionType::Teleport:
                            //if moving up move to bottom
                            if (man.normal.y > 0)
                            {
                                tx.move(0.f, teleportDist);
                            }
                            break;
                        }
                    }
                }
            }
        }


        //current input actually points to next empty slot.
        std::size_t idx = (player.currentInput + player.history.size() - 1) % player.history.size();

        //parse any outstanding inputs
        while (player.lastUpdatedInput != idx)
        {   
            //TODO surely we need to update collision for every input?

            auto delta = getDelta(player.history, player.lastUpdatedInput);
            tx.move(speed * parseInput(player.history[player.lastUpdatedInput].mask) * delta);
            player.lastUpdatedInput = (player.lastUpdatedInput + 1) % player.history.size();

            if (player.state != Player::State::Jumping)
            {
                if (player.history[player.lastUpdatedInput].mask & InputFlag::Up
                    && player.canJump)
                {
                    player.state = Player::State::Jumping;
                    player.velocity = -initialJumpVelocity;
                    player.canJump = false;
                }
            }
            else
            {
                //variable jump height
                if ((player.history[player.lastUpdatedInput].mask & InputFlag::Up) == 0
                    && player.velocity < minJumpVelocity)
                {
                    player.velocity = minJumpVelocity;
                }
                
                tx.move({ 0.f, player.velocity * delta });
                player.velocity += gravity * delta;
                player.velocity = std::min(player.velocity, maxVelocity);
            }

            //let go of jump so enable jumping again
            if ((player.history[player.lastUpdatedInput].mask & InputFlag::Up) == 0)
            {
                player.canJump = true;
            }
        }
    }
}

void PlayerSystem::reconcile(const ActorState& state, xy::Entity entity)
{
    //DPRINT("Reconcile to: ", std::to_string(x) + ", " + std::to_string(y));

    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();

    tx.setPosition(state.x, state.y);
    player.state = state.playerState;
    player.velocity = state.playerVelocity;

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
            tx.move(speed * parseInput(player.history[idx].mask) * delta);
            idx = (idx + 1) % player.history.size();

            if (player.state != Player::State::Jumping)
            {
                if (player.history[idx].mask & InputFlag::Up)
                {
                    player.state = Player::State::Jumping;
                    player.velocity = -initialJumpVelocity;
                }
            }
            else
            //if(player.state == Player::State::Jumping)
            {
                if ((player.history[idx].mask & InputFlag::Up) == 0
                    && player.velocity < minJumpVelocity)
                {
                    player.velocity = minJumpVelocity;
                }                
                
                tx.move({ 0.f, player.velocity * delta });
                player.velocity += gravity * delta;
                player.velocity = std::min(player.velocity, maxVelocity);
            }
        }
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
    if (xy::Util::Vector::lengthSquared(motion) > 1)
    {
        motion = xy::Util::Vector::normalise(motion);
    }

    return motion;
}

float PlayerSystem::getDelta(const History& history, std::size_t idx)
{
    auto prevInput = (idx + history.size() - 1) % history.size();
    auto delta = history[idx].timestamp - history[prevInput].timestamp;

    return static_cast<float>(delta) / 1000000.f;
}
