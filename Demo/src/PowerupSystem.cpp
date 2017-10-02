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

#include "PowerupSystem.hpp"
#include "Hitbox.hpp"
#include "MapData.hpp"
#include "AnimationController.hpp"
#include "ClientServerShared.hpp"
#include "PacketIDs.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/network/NetHost.hpp>

PowerupSystem::PowerupSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(PowerupSystem)),
    m_host(host)
{
    requireComponent<xy::Transform>();
    requireComponent<Powerup>();
    requireComponent<CollisionComponent>();
    requireComponent<AnimationController>();
    requireComponent<Actor>();
}


//public
void PowerupSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto entity : entities)
    {
        const auto& powerup = entity.getComponent<Powerup>();
        switch (powerup.type)
        {
        default:break;
        case Powerup::Type::Lightning:
            handleCollision(entity, dt);
            processLightning(entity, dt);
            break;
        case Powerup::Type::Fire:
            handleCollision(entity, dt);
            processFire(entity, dt);
            break;
        case Powerup::Type::Water:
            handleCollision(entity, dt);
            processWater(entity, dt);
            break;
        }
    }


    //TODO check scene conditions and spawn powerups as necessary

    //temp for testing
    static float timer = 0.f;
    timer += dt;
    if (timer > 2.f)
    {
        timer = 0.f;
        spawn(ActorID::LightningOne, 0);
        spawn(ActorID::LightningTwo, 1);
    }
}

//private
void PowerupSystem::processLightning(xy::Entity entity, float dt)
{
    auto& powerup = entity.getComponent<Powerup>();

    switch (powerup.state)
    {
    default: break;
    case Powerup::State::Idle:
        processIdle(entity, dt);
        break;
    case Powerup::State::Active:

        break;
    case Powerup::State::Dying:

        break;
    }

    //TODO set animation state
}

void PowerupSystem::processFire(xy::Entity entity, float dt)
{
    auto& powerup = entity.getComponent<Powerup>();

    switch (powerup.state)
    {
    default: break;
    case Powerup::State::Idle:
        processIdle(entity, dt);
        break;
    case Powerup::State::Active:

        break;
    case Powerup::State::Dying:

        break;
    }
}

void PowerupSystem::processWater(xy::Entity entity, float dt)
{
    auto& powerup = entity.getComponent<Powerup>();

    switch (powerup.state)
    {
    default: break;
    case Powerup::State::Idle:
        processIdle(entity, dt);
        break;
    case Powerup::State::Active:

        break;
    case Powerup::State::Dying:

        break;
    }
}

void PowerupSystem::processIdle(xy::Entity entity, float dt)
{   
    auto& tx = entity.getComponent<xy::Transform>();
    tx.move(0.f, BubbleVerticalVelocity * dt);

    auto& powerup = entity.getComponent<Powerup>();
    powerup.lifetime -= dt;
    if (powerup.lifetime < 0)
    {
        //destroy
        getScene()->destroyEntity(entity);

        ActorEvent evt;
        evt.actor = entity.getComponent<Actor>();
        evt.type = ActorEvent::Died;
        evt.x = tx.getPosition().x;
        evt.y = tx.getPosition().y;

        m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
    }
}

void PowerupSystem::handleCollision(xy::Entity entity, float dt)
{

}

void PowerupSystem::spawn(sf::Int32 actorID, sf::Uint8 player)
{
    sf::FloatRect bounds = { -BubbleSize / 2.f, -BubbleSize / 2.f, BubbleSize, BubbleSize };
    
    sf::Vector2f spawnPos = (player == 0) ? powerupOneSpawn : powerupTwoSpawn;
    

    //spawn one for each player every time
    auto entity = getScene()->createEntity();
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = actorID;

    entity.addComponent<xy::Transform>().setPosition(spawnPos);
    entity.addComponent<CollisionComponent>().addHitbox(bounds, CollisionType::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PowerupMask);

    entity.addComponent<Powerup>().owner = player;
    entity.getComponent<Powerup>().type = Powerup::Type::Lightning;
    entity.getComponent<Powerup>().velocity.x = (player == 0) ? -1.f : 1.f;

    entity.addComponent<AnimationController>();
    entity.addComponent<xy::QuadTreeItem>().setArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;

    //broadcast to clients
    ActorEvent evt;
    evt.actor = entity.getComponent<Actor>();
    evt.x = spawnPos.x;
    evt.y = spawnPos.y;
    evt.type = ActorEvent::Spawned;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
}