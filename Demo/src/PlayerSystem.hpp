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

#ifndef DEMO_PLAYER_SYSTEM_HPP_
#define DEMO_PLAYER_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <array>

struct ClientState;
struct Input final
{
    sf::Uint16 mask = 0;
    sf::Int64 timestamp = 0;
};

using History = std::array<Input, 120u>;

struct Player final
{
    History history;
    std::size_t currentInput = 0;
    std::size_t lastUpdatedInput = history.size() - 1;
    sf::Uint8 playerNumber = 0;
    enum class State : sf::Uint8
    {
        Walking, Jumping, Dying
    }state = State::Walking;
    sf::Vector2f velocity; //only the Y velocity is actually used in movement - the x value is used to track how much initial velocity to spawn bubble with
    bool canJump = true;
    bool canLand = false; //only for 1 way platforms
    bool canShoot = true;
    enum class Direction : sf::Uint8 
    {
        Left, Right
    }direction = Direction::Right;

    sf::Vector2f spawnPosition;
    float timer = 2.f; //times invulnerability when spawning, and time to respawn
};

class PlayerSystem final : public xy::System
{
public:
    explicit PlayerSystem(xy::MessageBus&, bool = false);

    void process(float) override;

    void reconcile(const ClientState&, xy::Entity);

private:

    bool m_isServer;

    sf::Vector2f parseInput(sf::Uint16);
    float getDelta(const History&, std::size_t);

    void resolveCollision(xy::Entity);
};

#endif //DEMO_PLAYER_SYSTEM_HPP_