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

#ifndef DEMO_POWERUP_SYSTEM_HPP_
#define DEMO_POWERUP_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

namespace xy
{
    class NetHost;
}

struct Powerup final
{
    enum class Type
    {
        Lightning, Fire, Water
    }type = Type::Lightning;

    enum class State
    {
        Idle, Active, Dying
    }state = State::Idle;

    sf::Uint8 owner = 0;
    float lifetime = 10.f;
    sf::Vector2f velocity;
};

class PowerupSystem final : public xy::System
{
public:
    PowerupSystem(xy::MessageBus&, xy::NetHost&);

    void process(float) override;

private:
    xy::NetHost& m_host;

    void processLightning(xy::Entity, float);
    void processFire(xy::Entity, float);
    void processWater(xy::Entity, float);
    void processIdle(xy::Entity, float);

    void handleCollision(xy::Entity, float);

    void spawn(sf::Int32 actorID, sf::Uint8 player);
};

#endif //DEMO_POWERUP_SYSTEM_HPP_