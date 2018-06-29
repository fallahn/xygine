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

#include "MapData.hpp"

#include <SFML/System/Vector2.hpp>

#include <xyginext/ecs/System.hpp>

#include <array>
#include <vector>

namespace xy
{
    class NetHost;
}

struct Crate final
{
    enum
    {
        Ground, Falling, Breaking, Carried
    }state = Falling;
    bool explosive = false;
    sf::Vector2f velocity;
    bool groundContact = false;
    bool lethal = false;
    sf::Uint8 lastOwner = 3;

    sf::Vector2f spawnPosition;
    bool respawn = false;

    struct Shake final
    {
        sf::Vector2f startPosition;
        float shakeTime = 0.f;
        bool shaking = false;
        std::size_t shakeIndex = 0;
    }shake;

    static constexpr float ShakeTime = 0.7f;
    static constexpr float PauseTime = 8.f;

    enum Flags
    {
        Explosive = 0x1,
        Respawn = 0x2
    };
};

class CrateSystem final : public xy::System
{
public:
    CrateSystem(xy::MessageBus&, xy::NetHost&);

    void handleMessage(const xy::Message&) override;

    void process(float) override;

private:

    xy::NetHost& m_host;

    std::vector<float> m_waveTable;

    std::array<std::pair<float, Crate>, MaxCrates> m_respawnQueue;
    std::size_t m_respawnCount;

    void groundCollision(xy::Entity);
    void airCollision(xy::Entity);
    void destroy(xy::Entity);

    void spawn(Crate);

    void onEntityAdded(xy::Entity) override;
};
