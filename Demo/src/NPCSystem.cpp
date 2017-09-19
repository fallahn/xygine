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
#include "Hitbox.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>

namespace
{
    const float WhirlybobSpeed = 100.f;
    const float ClocksySpeed = 86.f;

    const float maxVelocity = 800.f;
    const float gravity = 2200.f;
    const float initialJumpVelocity = 840.f;
}

NPCSystem::NPCSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(NPCSystem))
{
    requireComponent<NPC>();
    requireComponent<Actor>();
    requireComponent<CollisionComponent>();
    requireComponent<xy::Transform>();
}

//public
void NPCSystem::handleMessage(const xy::Message&)
{

}

void NPCSystem::process(float dt)
{
    auto entities = getEntities();
    for (auto& entity : entities)
    {
        switch (entity.getComponent<Actor>().type)
        {
        default: break;
        case ActorID::Clocksy:
            updateClocksy(entity, dt);
            break;
        case ActorID::Whirlybob:
            updateWhirlybob(entity, dt);
            break;
        }
    }
}

//private
void NPCSystem::updateWhirlybob(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    const auto& collision = entity.getComponent<CollisionComponent>();
    const auto& hitboxes = collision.getHitboxes();

    for (auto i = 0; i < collision.getHitboxCount(); ++i)
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
                break;
            }
        }
    }

    if (npc.state == NPC::State::Normal)
    {
        tx.move(npc.velocity * WhirlybobSpeed * dt);
    }
    else
    {
        //we're in bubble state
    }
}

void NPCSystem::updateClocksy(xy::Entity entity, float dt)
{
    auto& tx = entity.getComponent<xy::Transform>();
    auto& npc = entity.getComponent<NPC>();

    const auto& collision = entity.getComponent<CollisionComponent>();
    const auto& hitboxes = collision.getHitboxes();

    for (auto i = 0; i < collision.getHitboxCount(); ++i)
    {
        auto& manifolds = hitboxes[i].getManifolds();
        for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
        {   
            if (hitboxes[i].getType() == CollisionType::NPC)
            {
                auto& manifold = manifolds[j];
                switch (manifold.otherType)
                {
                default: break;
                case CollisionType::Solid:
                case CollisionType::Platform:
                    tx.move(manifold.normal * manifold.penetration);
                    npc.velocity = xy::Util::Vector::reflect(npc.velocity, manifold.normal);
                    npc.velocity.y = 0.f;

                    if (npc.state == NPC::State::Falling)
                    {
                        npc.state = NPC::State::Normal;
                    }

                    break;
                case CollisionType::Bubble:
                    //switch to bubble state if bubble in spawn state
                    break;
                }
            }
            else
            {
                if (hitboxes[i].getCollisionCount() == 0)
                {
                    //foots in the air so we're falling
                    npc.state = NPC::State::Falling;
                }
            }
        }
    }


    switch (npc.state)
    {
    default: break;
    case NPC::State::Normal:
        tx.move(npc.velocity * ClocksySpeed * dt);
        break;
    case NPC::State::Falling:
        npc.velocity.y += gravity * dt;
        tx.move(npc.velocity * dt);
        break;
    }
}