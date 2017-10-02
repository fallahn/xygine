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
#include "NPCSystem.hpp"

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
        case Powerup::Type::Flame:
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
        spawn(ActorID::FlameOne, 0);
        spawn(ActorID::FlameTwo, 1);
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
        entity.getComponent<xy::Transform>().move(powerup.velocity * BubbleVerticalVelocity * 6.f * dt);
        break;
    case Powerup::State::Dying:
        powerup.lifetime -= dt;
        if (powerup.lifetime < 0.f)
        {
            despawn(entity);
        }
        break;
    }
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
        despawn(entity);
    }
}

void PowerupSystem::handleCollision(xy::Entity entity, float dt)
{
    auto& powerup = entity.getComponent<Powerup>();
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
            case CollisionType::Player:
                //check if our player and switch to active mode
                if (powerup.state == Powerup::State::Idle)
                {
                    if (man.otherEntity.getComponent<Player>().playerNumber == powerup.owner)
                    {
                        powerup.state = Powerup::State::Active;
                        entity.getComponent<AnimationController>().nextAnimation = AnimationController::Walk;
                    }
                }
                break;
            case CollisionType::Platform:
                //if not active move along x axis
                if (powerup.state == Powerup::State::Idle)
                {
                    tx.move(man.normal * man.penetration);
                    tx.move(powerup.velocity * -BubbleVerticalVelocity * dt);
                }
                break;
            case CollisionType::Solid:
                //if not active move along x axis
                //otherwise die
                if (powerup.state == Powerup::State::Idle)
                {
                    tx.move(man.normal * man.penetration);
                    tx.move(powerup.velocity * -BubbleVerticalVelocity * dt);
                }
                else if (powerup.state == Powerup::State::Active)
                {
                    tx.move(man.normal * man.penetration);
                    powerup.state = Powerup::State::Dying;
                    powerup.lifetime = 1.f;
                    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
                }
                break;
            case CollisionType::NPC:
                if (powerup.state == Powerup::State::Active)
                {
                    tx.move(man.normal * man.penetration);
                    powerup.state = Powerup::State::Dying;
                    powerup.lifetime = 1.f;
                    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;

                    getScene()->getSystem<NPCSystem>().despawn(man.otherEntity, powerup.owner);
                }
                break;
            }
        }
    }
}

void PowerupSystem::spawn(sf::Int32 actorID, sf::Uint8 player)
{
    sf::FloatRect bounds = { 0.f, 0.f, BubbleSize, BubbleSize };  
    sf::Vector2f spawnPos = (player == 0) ? powerupOneSpawn : powerupTwoSpawn;
    Powerup::Type type = Powerup::Type::Lightning;
    switch (actorID)
    {
        default: break;
    case ActorID::FlameOne:
    case ActorID::FlameTwo:
        type = Powerup::Type::Flame;
        break;
    case ActorID::WaterOne:
    case ActorID::WaterTwo:
        type = Powerup::Type::Water;
        break;
    }
    
    auto entity = getScene()->createEntity();
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = actorID;

    entity.addComponent<xy::Transform>().setPosition(spawnPos);
    entity.getComponent<xy::Transform>().setOrigin(BubbleSize / 2.f, BubbleSize / 2.f);
    entity.addComponent<CollisionComponent>().addHitbox(bounds, CollisionType::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PowerupMask);

    entity.addComponent<Powerup>().owner = player;
    entity.getComponent<Powerup>().type = type;
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

void PowerupSystem::despawn(xy::Entity entity)
{
    const auto& tx = entity.getComponent<xy::Transform>();

    //destroy
    getScene()->destroyEntity(entity);

    ActorEvent evt;
    evt.actor = entity.getComponent<Actor>();
    evt.type = ActorEvent::Died;
    evt.x = tx.getPosition().x;
    evt.y = tx.getPosition().y;

    m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
}