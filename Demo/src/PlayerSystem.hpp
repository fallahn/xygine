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

#include "Hitbox.hpp"

#include <xyginext/ecs/System.hpp>

#include <array>

struct ClientState;
struct Input final
{
    std::uint16_t mask = 0;
    std::int64_t timestamp = 0;
};

struct HistoryState final
{
    Input input;
    CollisionComponent collision;
};

using History = std::array<HistoryState, 120u>;

struct Player final
{
    History history;
    std::size_t currentInput = 0;
    std::size_t lastUpdatedInput = history.size() - 1;
    std::uint8_t playerNumber = 0;
    sf::Vector2f spawnPosition;

    enum class State : std::uint8_t
    {
        Walking, Jumping, Dying, Dead, Disabled //disable the player during map transitions
    };

    enum class Direction :std::uint8_t
    {
        Left, Right
    };

    enum
    {
        JumpFlag = 0x1,
        ShootFlag = 0x2,
        BubbleFlag = 0x4,
        HatFlag = 0x8
    };

    //player info which requires net sync
    struct SyncState final
    {
        State state = State::Jumping;
        Direction direction = Direction::Right;

        std::uint8_t flags = 0;

        sf::Vector2f velocity; //only the Y velocity is actually used in movement - the x value is used to track how much initial velocity to spawn bubble with
        std::uint8_t canLand = 0; //only for 1 way platforms

        float timer = 2.f; //times invulnerability when spawning, and time to respawn
        std::uint8_t lives = 3;
        std::uint8_t bonusFlags = 0;
    }sync;
};

struct Manifold;
class PlayerSystem final : public xy::System
{
public:
    explicit PlayerSystem(xy::MessageBus&, bool = false);

    void process(float) override;

    void reconcile(const ClientState&, xy::Entity);

private:

    bool m_isServer;

    sf::Vector2f parseInput(std::uint16_t);
    float getDelta(const History&, std::size_t);

    void processInput(std::uint16_t, float, xy::Entity);

    void resolveCollision(xy::Entity);

    void collisionWalking(xy::Entity);
    void collisionJumping(xy::Entity);
    void collisionDying(xy::Entity);

    bool npcCollision(xy::Entity, const Manifold&); //returns true if this caused a state change
    bool explosionCollision(xy::Entity, const Manifold&); //as above
};
