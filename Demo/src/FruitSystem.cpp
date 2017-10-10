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

#include "FruitSystem.hpp"
#include "Hitbox.hpp"
#include "MessageIDs.hpp"
#include "MapData.hpp"
#include "ClientServerShared.hpp"
#include "PacketIDs.hpp"
#include "AnimationController.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/network/NetHost.hpp>
#include <xyginext/util/Vector.hpp>

FruitSystem::FruitSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(FruitSystem)),
    m_host(host)
{
    requireComponent<CollisionComponent>();
    requireComponent<xy::Transform>();
    requireComponent<Fruit>();
}

//public
void FruitSystem::handleMessage(const xy::Message& msg)
{
    //if NPC died spawn a frooit
    if (msg.id == MessageID::SceneMessage)
    {
        const auto& data = msg.getData<SceneEvent>();
        if (data.type == SceneEvent::ActorRemoved)
        {
            switch (data.actorID)
            {
            default:break;
            case ActorID::Clocksy:
            case ActorID::Whirlybob:
            case ActorID::Balldock:
            case ActorID::Squatmo:
            {
                sf::Vector2f initialVelocity;
                initialVelocity.x = (data.x > MapBounds.width / 2.f) ? -50.f : 50.f;
                initialVelocity.y = (data.y > MapBounds.height / 2.f) ? -180.f : -100.f;
                
                //spawn a fruit
                auto scene = getScene();
                auto entity = scene->createEntity();
                entity.addComponent<xy::Transform>().setPosition(data.x, data.y);
                entity.getComponent<xy::Transform>().setOrigin(SmallFoodOrigin);
                entity.addComponent<Actor>().id = entity.getIndex();
                entity.getComponent<Actor>().type = ActorID::FruitSmall;
                entity.addComponent<Fruit>().velocity = initialVelocity;
                entity.addComponent<CollisionComponent>().addHitbox(SmallFoodBounds, CollisionType::Fruit);
                entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Fruit);
                entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::FruitMask);
                entity.addComponent<xy::QuadTreeItem>().setArea(SmallFoodBounds);

                entity.addComponent<AnimationController>();
                entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem; //so we can destroy at whim

                //broadcast to clients
                ActorEvent evt;
                evt.actor.id = entity.getIndex();
                evt.actor.type = entity.getComponent<Actor>().type;
                evt.x = data.x;
                evt.y = data.y;
                evt.type = ActorEvent::Spawned;

                m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
            }
                break;
            }
        }
    }
}

void FruitSystem::process(float dt)
{
    auto entities = getEntities();
    for (auto entity : entities)
    {
        auto& tx = entity.getComponent<xy::Transform>();
        auto& fruit = entity.getComponent<Fruit>();

        //collision
        const auto& collision = entity.getComponent<CollisionComponent>();
        const auto& hitboxes = collision.getHitboxes();

        for (auto i = 0u; i < collision.getHitboxCount(); ++i)
        {
            auto& manifolds = hitboxes[i].getManifolds();
            for (auto j = 0u; j < hitboxes[i].getCollisionCount(); ++j)
            {
                switch (fruit.state)
                {
                default: break;
                case Fruit::Colliding:
                    if (((manifolds[j].otherType == CollisionType::Solid) || (manifolds[j].otherType == CollisionType::Platform))
                        && manifolds[j].normal.y < 0)
                    {
                        tx.move(manifolds[j].normal * manifolds[j].penetration);
                        fruit.velocity = {};
                        fruit.state = Fruit::Idle;
                        break;
                    }
                    //else fall through
                case Fruit::Spawning:
                    if (manifolds[j].otherType == CollisionType::Solid)
                    {
                        tx.move(manifolds[j].normal * manifolds[j].penetration);
                        fruit.velocity = xy::Util::Vector::reflect(fruit.velocity, manifolds[j].normal);
                        fruit.velocity *= 0.9f;
                    }
                    else if (manifolds[j].otherType == CollisionType::Teleport)
                    {
                        tx.move(0.f, -TeleportDistance);
                    }
                    break;
                case Fruit::Idle:
                    if (manifolds[j].otherType == CollisionType::Player)
                    {
                        //despawn.
                        getScene()->destroyEntity(entity);

                        //broadcast to clients
                        ActorEvent evt;
                        evt.actor.id = entity.getIndex();
                        evt.actor.type = entity.getComponent<Actor>().type;
                        evt.x = tx.getPosition().x;
                        evt.y = tx.getPosition().y;
                        evt.type = ActorEvent::Died;

                        m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);

                        auto* msg = postMessage<ItemEvent>(MessageID::ItemMessage);
                        msg->playerID = manifolds[j].otherEntity.getComponent<Player>().playerNumber;
                        msg->actorID = (fruit.size == Fruit::Small) ? ActorID::FruitSmall : ActorID::FruitLarge;
                    }
                    break;
                }
            }
        }
      
        //movement
        if (fruit.state != Fruit::Idle)
        {
            fruit.spawnTime -= dt;
            if (fruit.spawnTime < 0)
            {
                fruit.state = Fruit::Colliding;
            }

            tx.move(fruit.velocity * dt);
            if (fruit.velocity.y < MaxVelocity)
            {
                fruit.velocity.y += Gravity * dt;
            }
        }
        else
        {
            fruit.lifeTime -= dt;
            if (fruit.lifeTime < 0)
            {
                //despawn.
                getScene()->destroyEntity(entity);

                //broadcast to clients
                ActorEvent evt;
                evt.actor.id = entity.getIndex();
                evt.actor.type = entity.getComponent<Actor>().type;
                evt.x = tx.getPosition().x;
                evt.y = tx.getPosition().y;
                evt.type = ActorEvent::Died;

                m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
            }
        }
    }
}
