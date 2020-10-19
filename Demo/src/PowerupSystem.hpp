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

#pragma once

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Clock.hpp>

namespace xy
{
    class NetHost;
}

struct Powerup final
{
    enum class Type
    {
        Lightning, Flame
    }type = Type::Lightning;

    enum class State
    {
        Idle, Active, Dying
    }state = State::Idle;

    std::uint8_t owner = 0;
    float lifetime = 10.f;
    sf::Vector2f velocity;

    //used in flame and water types to spread around
    std::uint8_t generation = 0;
    static constexpr std::uint8_t MaxFlameGenerations = 4;
    static constexpr std::uint8_t MaxWaterGenerations = 8;

    enum class SpreadDirection
    {
        Both, Left, Right
    }spread = SpreadDirection::Both;
};

class PowerupSystem final : public xy::System
{
public:
    PowerupSystem(xy::MessageBus&, xy::NetHost&);

    void process(float) override;

    enum SpawnFlags
    {
        Flame = 0x1,
        Lightning = 0x2,
        Water = 0x4
    };
    void setSpawnFlags(std::uint8_t);

private:
    xy::NetHost& m_host;
    std::uint8_t m_spawnFlags;

    sf::Clock m_flameClock;
    sf::Clock m_lightningClock;
    float m_flameTime;
    float m_lightningTime;
    std::size_t m_nextSpawnTime;

    void processLightning(xy::Entity, float);
    void processFire(xy::Entity, float);
    void processIdle(xy::Entity, float);

    void defaultCollision(xy::Entity, float);
    void fireCollision(xy::Entity);

    void spawn(std::int32_t actorID, std::uint8_t player);
    void spawnFlame(sf::Vector2f position, std::uint8_t player, Powerup::SpreadDirection, std::uint8_t generation);
    void despawn(xy::Entity);
};
