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
#include <xyginext/util/Random.hpp>

namespace
{
    const float flameLifetime = 0.6f; //loosely based on the fact that animation has 7 frames at 12fps
    const sf::Vector2f flameOffset(32.f, 0.f);
    const float flameSpreadTime = 0.5f; //age at which a flame spawns another

    std::array<float, 6u> spawnTimes = { 9.f, 12.f, 10.5f, 9.5f, 12.5f, 11.f };
}

PowerupSystem::PowerupSystem(xy::MessageBus& mb, xy::NetHost& host)
    : xy::System(mb, typeid(PowerupSystem)),
    m_host          (host),
    m_spawnFlags    (255),
    m_waterTime     (6.f),
    m_flameTime     (15.f),
    m_lightningTime (22.f),
    m_nextSpawnTime (0)
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
        auto& powerup = entity.getComponent<Powerup>();
        switch (powerup.type)
        {
        default:break;
        case Powerup::Type::Lightning:
            defaultCollision(entity, dt);
            processLightning(entity, dt);
            break;
        case Powerup::Type::Flame:
            //fire collision in handled in processFire()
            processFire(entity, dt);
            break;
        case Powerup::Type::Water:
            //water collision is handled in processWater()
            processWater(entity, dt);
            break;
        }

        if (powerup.state == Powerup::State::Active)
        {
            //check to see if still in map area
            if (!MapBounds.contains(entity.getComponent<xy::Transform>().getPosition()))
            {
                powerup.state = Powerup::State::Dying;
                powerup.lifetime = 1.f;
                entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
            }
        }
    }
    //DPRINT("count", std::to_string(entities.size()));

    //check time and spawn if enabled
    if (m_flameClock.getElapsedTime().asSeconds() > m_flameTime)
    {
        m_flameClock.restart();
        if (m_spawnFlags & SpawnFlags::Flame)
        {
            spawn(ActorID::FlameOne, 0);
            spawn(ActorID::FlameTwo, 1);
            m_flameTime = spawnTimes[m_nextSpawnTime];
            m_nextSpawnTime = (m_nextSpawnTime + 1) % spawnTimes.size();
        }
    }

    if (m_waterClock.getElapsedTime().asSeconds() > m_waterTime)
    {
        m_waterClock.restart();
        if (m_spawnFlags & SpawnFlags::Water)
        {
            spawn(ActorID::WaterOne, 0);
            spawn(ActorID::WaterTwo, 1);
            m_waterTime = spawnTimes[m_nextSpawnTime];
            m_nextSpawnTime = (m_nextSpawnTime + 1) % spawnTimes.size();
        }
    }

    if (m_lightningClock.getElapsedTime().asSeconds() > m_lightningTime)
    {
        m_lightningClock.restart();
        if (m_spawnFlags & SpawnFlags::Lightning)
        {
            spawn(ActorID::LightningOne, 0);
            spawn(ActorID::LightningTwo, 1);
            m_lightningTime = spawnTimes[m_nextSpawnTime];
            m_nextSpawnTime = (m_nextSpawnTime + 1) % spawnTimes.size();
        }
    }

    //temp for testing
    /*static float timer = 0.f;
    timer += dt;
    if (timer > 5.f)
    {
        timer = 0.f;
        spawn(ActorID::LightningOne, 0);
    }*/
}

void PowerupSystem::setSpawnFlags(sf::Uint8 flags)
{
    m_spawnFlags = flags;
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
        entity.getComponent<xy::Transform>().move(powerup.velocity.x * BubbleVerticalVelocity * 6.f * dt, 0.f);
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
        defaultCollision(entity, dt);
        processIdle(entity, dt);
        break;
    case Powerup::State::Active:
        fireCollision(entity);
        powerup.velocity.y += Gravity * dt;
        powerup.velocity.y = std::min(powerup.velocity.y, MaxVelocity);
        entity.getComponent<xy::Transform>().move(0.f, powerup.velocity.y * dt);
        //incase falls out of map
        powerup.lifetime -= dt;
        if (powerup.lifetime < 0)
        {
            despawn(entity);
        }

        break;
    case Powerup::State::Dying:
    {
        fireCollision(entity);

        powerup.velocity.y += Gravity * dt;
        powerup.velocity.y = std::min(powerup.velocity.y, MaxVelocity);
        entity.getComponent<xy::Transform>().move(0.f, powerup.velocity.y * dt);

        float oldTime = powerup.lifetime;
        powerup.lifetime -= dt;

        if (oldTime > flameSpreadTime
            && powerup.lifetime < flameSpreadTime)
        {
            //spawn a new flame
            if (powerup.generation < Powerup::MaxFlameGenerations)
            {
                switch (powerup.spread)
                {
                default:break;
                case Powerup::SpreadDirection::Left:
                    spawnFlame(entity.getComponent<xy::Transform>().getPosition() - flameOffset, powerup.owner, powerup.spread, powerup.generation + 1);
                    break;
                case Powerup::SpreadDirection::Right:
                    spawnFlame(entity.getComponent<xy::Transform>().getPosition() + flameOffset, powerup.owner, powerup.spread, powerup.generation + 1);
                    break;
                case Powerup::SpreadDirection::Both:
                    spawnFlame(entity.getComponent<xy::Transform>().getPosition() - flameOffset, powerup.owner, Powerup::SpreadDirection::Left, powerup.generation + 1);
                    spawnFlame(entity.getComponent<xy::Transform>().getPosition() + flameOffset, powerup.owner, Powerup::SpreadDirection::Right, powerup.generation + 1);
                    break;
                }
            }
        }
        else if (powerup.lifetime < 0.f)
        {
            despawn(entity);
        }
    }
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
        defaultCollision(entity, dt);
        processIdle(entity, dt);
        break;
    case Powerup::State::Active:
        despawn(entity);
        break;
    case Powerup::State::Dying:

        break;
    }
}

void PowerupSystem::processIdle(xy::Entity entity, float dt)
{   
    auto& powerup = entity.getComponent<Powerup>();
    auto& tx = entity.getComponent<xy::Transform>();
    tx.move(0.f, powerup.velocity.y * dt);

    powerup.lifetime -= dt;
    if (powerup.lifetime < 0)
    {
        despawn(entity);
    }
}

void PowerupSystem::defaultCollision(xy::Entity entity, float dt)
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
                        powerup.lifetime = 5.f;
                        entity.getComponent<AnimationController>().nextAnimation = AnimationController::Walk;
                    }
                }
                break;
            case CollisionType::Platform:
                //if not active move along x axis
                if (powerup.state == Powerup::State::Idle)
                {
                    tx.move(man.normal * man.penetration);
                    tx.move(powerup.velocity.x * -BubbleVerticalVelocity * dt, 0.f);
                }
                break;
            case CollisionType::Solid:
                //if not active move along x axis
                //otherwise die
                if (powerup.state == Powerup::State::Idle)
                {
                    tx.move(man.normal * man.penetration);
                    tx.move(powerup.velocity.x * -BubbleVerticalVelocity * dt, 0.f);
                }
                /*else if (powerup.state == Powerup::State::Active)
                {
                    tx.move(man.normal * man.penetration);
                    powerup.state = Powerup::State::Dying;
                    powerup.lifetime = 1.f;
                    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
                }*/
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

void PowerupSystem::fireCollision(xy::Entity entity)
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
            case CollisionType::Platform:
            case CollisionType::Solid:
                tx.move(man.penetration * man.normal);
                if (powerup.state == Powerup::State::Active
                    && man.normal.y < 0) //so only when collision below
                {                 
                    powerup.state = Powerup::State::Dying;
                    powerup.lifetime = flameLifetime;
                    entity.getComponent<AnimationController>().nextAnimation = AnimationController::Die;
                }
                break;
            case CollisionType::NPC:
                if (powerup.state == Powerup::State::Dying)
                {
                    getScene()->getSystem<NPCSystem>().despawn(man.otherEntity, powerup.owner);
                }
                break;
                //TODO should it kill own player too?
            }
        }
    }
}

void PowerupSystem::spawn(sf::Int32 actorID, sf::Uint8 player)
{
    bool top = (xy::Util::Random::value(0, 1) == 0);
    
    sf::FloatRect bounds = { 0.f, 0.f, BubbleSize, BubbleSize };  
    sf::Vector2f spawnPos = (player == 0) ? PowerupOneSpawn : PowerupTwoSpawn;
    if (top) spawnPos.y -= TopSpawn;

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
    entity.getComponent<Powerup>().velocity.y = (top) ? -BubbleVerticalVelocity : BubbleVerticalVelocity;

    entity.addComponent<AnimationController>().nextAnimation = AnimationController::Idle;
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

void PowerupSystem::spawnFlame(sf::Vector2f position, sf::Uint8 player, Powerup::SpreadDirection direction, sf::Uint8 generation)
{
    //doesn't matter which actor id because flame anim the same on both
    sf::FloatRect bounds = { 0.f, 0.f, BubbleSize, BubbleSize };

    auto entity = getScene()->createEntity();
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = ActorID::FlameOne;

    entity.addComponent<xy::Transform>().setPosition(position);
    entity.getComponent<xy::Transform>().setOrigin(BubbleSize / 2.f, BubbleSize / 2.f);
    entity.addComponent<CollisionComponent>().addHitbox(bounds, CollisionType::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Powerup);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PowerupMask);

    entity.addComponent<Powerup>().owner = player;
    entity.getComponent<Powerup>().type = Powerup::Type::Flame;
    entity.getComponent<Powerup>().state = Powerup::State::Dying;
    entity.getComponent<Powerup>().lifetime = flameLifetime;
    entity.getComponent<Powerup>().spread = direction;
    entity.getComponent<Powerup>().generation = generation;

    entity.addComponent<AnimationController>().nextAnimation = AnimationController::Shoot;
    entity.addComponent<xy::QuadTreeItem>().setArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;


    //broadcast to clients
    ActorEvent evt;
    evt.actor = entity.getComponent<Actor>();
    evt.x = position.x;
    evt.y = position.y;
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