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

#include "BonusSystem.hpp"
#include "CollisionSystem.hpp"
#include "PlayerSystem.hpp"
#include "MapData.hpp"
#include "CommandIDs.hpp"
#include "ClientServerShared.hpp"
#include "PacketIDs.hpp"
#include "AnimationController.hpp"
#include "MessageIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/network/NetHost.hpp>
#include <xyginext/util/Random.hpp>

#include <array>

namespace
{
    std::array<float, 5u> spawnTimes = { 11.f, 14.f, 13.f, 17.f, 14.f };
}

std::array<Bonus::Value, 5u> Bonus::valueMap  = {B,O,N,U,S}; //this should be const but g++ complains

BonusSystem::BonusSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(BonusSystem)),
    m_host              (host),
    m_currentSpawnTime  (0),
    m_spawnTimer        (0.f),
    m_enabled           (false)
{
    requireComponent<Bonus>();
    requireComponent<xy::Transform>();
}


//public
void BonusSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto entity : entities)
    {
        auto& bonus = entity.getComponent<Bonus>();
        auto& tx = entity.getComponent<xy::Transform>();

        doCollision(entity);

        tx.move(bonus.velocity * dt * (bonus.lifetime / Bonus::MaxLifeTime));

        bonus.lifetime -= dt;

        if (!MapBounds.contains(tx.getPosition())
            || bonus.lifetime < 0)
        {
            kill(entity);
        }
    }

    //check time and spawn next
    m_spawnTimer += dt;
    if (m_enabled && m_spawnTimer > spawnTimes[m_currentSpawnTime])
    {
        m_spawnTimer = 0.f;
        m_currentSpawnTime = (m_currentSpawnTime + 1) % spawnTimes.size();

        auto pos = (xy::Util::Random::value(0, 1) == 0) ? PowerupOneSpawn : PowerupTwoSpawn;
        if (xy::Util::Random::value(0, 1) == 1)
        {
            pos.y -= TopSpawn;
        }

        spawn(pos.x, pos.y);
    }
}

void BonusSystem::doCollision(xy::Entity entity)
{
    auto& bonus = entity.getComponent<Bonus>();
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
            case CollisionType::Platform:
                tx.move(man.normal * man.penetration);

                if (man.normal.y != 0 && bonus.velocity.x == 0)
                {
                    bonus.velocity.x =
                    (xy::Util::Random::value(0, 1) == 0) ?
                    -BubbleVerticalVelocity : BubbleVerticalVelocity;
                }
                else if (man.normal.x != 0)
                {
                    bonus.velocity.x = 0.f;
                }

                break;
            case CollisionType::Player:
            {
                //award to player and pop
                auto otherEnt = man.otherEntity;
                otherEnt.getComponent<Player>().bonusFlags |= bonus.value;

                auto* msg = postMessage<ItemEvent>(MessageID::ItemMessage);
                msg->actorID = ActorID::Bonus;
                msg->player = man.otherEntity;

                kill(entity);
            }
                break;
            }
        }
    }
}

void BonusSystem::kill(xy::Entity entity)
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

void BonusSystem::spawn(float x, float y)
{
    auto value = xy::Util::Random::value(0, 4);
    
    auto scene = getScene();
    auto entity = scene->createEntity();
    entity.addComponent<xy::Transform>().setPosition(x, y);
    entity.getComponent<xy::Transform>().setOrigin(BubbleOrigin);
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = ActorID::Bonus;
    entity.addComponent<Bonus>().value = Bonus::valueMap[value];
    entity.getComponent<Bonus>().velocity.y = (y > xy::DefaultSceneSize.y / 2.f) ? BubbleVerticalVelocity : -BubbleVerticalVelocity;
    
    entity.addComponent<CollisionComponent>().addHitbox(BubbleBounds, CollisionType::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Bonus);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Solid | CollisionFlags::Player | CollisionFlags::Platform);
    entity.addComponent<xy::QuadTreeItem>().setArea(BubbleBounds);

    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem; //so we can destroy at whim
    entity.addComponent<AnimationController>().nextAnimation = static_cast<AnimationController::Animation>(value);


    //broadcast to clients
    ActorEvent evt;
    evt.actor.id = entity.getIndex();
    evt.actor.type = entity.getComponent<Actor>().type;
    evt.x = x;
    evt.y = y;
    evt.type = ActorEvent::Spawned;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
}