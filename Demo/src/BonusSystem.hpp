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

#ifndef DEMO_BONUS_SYSTEM_HPP_
#define DEMO_BONUS_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Vector2.hpp>

#include <array>

namespace xy
{
    class NetHost;
}

struct Bonus final
{
    enum Value
    {
        B = 0x1,
        O = 0x2,
        N = 0x4,
        U = 0x8,
        S = 0x10,
        BONUS = B | O | N | U | S
    }value = B;

    static constexpr float MaxLifeTime = 7.f;
    float lifetime = MaxLifeTime;
    sf::Vector2f velocity;

    static constexpr std::array<Value, 5> valueMap = {B,O,N,U,S};
};

class BonusSystem final : public xy::System
{
public:
    BonusSystem(xy::MessageBus&, xy::NetHost&);

    void process(float) override;

    void setEnabled(bool enable) { m_enabled = enable; }

private:

    xy::NetHost& m_host;
    std::size_t m_currentSpawnTime;
    float m_spawnTimer;

    bool m_enabled;

    void doCollision(xy::Entity);
    void kill(xy::Entity);
    void spawn(float, float);
};

#endif //DEMO_BONUS_SYSTEM_HPP_