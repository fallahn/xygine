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

#ifndef DEMO_CLIENT_SERVER_SHARED_HPP_
#define DEMO_CLIENT_SERVER_SHARED_HPP_

#include "Hitbox.hpp"

#include <xyginext/ecs/Entity.hpp>

#include <SFML/Graphics/Color.hpp>

#include <tmxlite/Object.hpp>

#include <string>

namespace xy
{
    class Scene;
}

//used when loading maps to check valid map properties exist
namespace MapFlags
{
    enum
    {
        Solid = 0x1,
        Platform = 0x2,
        Graphics = 0x4,
        Teleport = 0x8,
        Spawn = 0x10,

        Shared = Solid | Platform | Teleport,
        Server = Shared | Spawn,
        Client = Shared | Graphics
    };
}

namespace CollisionFlags //used in broad phase culling
{
    enum
    {
        Solid = 0x1,
        Platform = 0x2,
        Teleport = 0x4,
        Player = 0x8,
        Bubble = 0x10,
        NPC = 0x20,
        Fruit = 0x40,
        Powerup = 0x80,
        Bonus = 0x100,
        HardBounds = 0x200,
        MagicHat = 0x400,
        Crate = 0x800,

        PlayerMask = Bubble | Platform | Solid | Teleport | NPC | Fruit | Powerup | Bonus | MagicHat | Crate,
        NPCMask = Solid | Player | Bubble | Platform | Teleport | Powerup | HardBounds | Crate,
        FruitMask = Solid | Platform | Player | Teleport,
        PowerupMask = Platform | Solid | Player | NPC | Crate,
        CrateMask = Platform | Solid | Player | NPC | Powerup | Teleport | Crate
    };
}

static const std::string dataDir("demo_game");

static constexpr float BubbleVerticalVelocity = -100.f;
static const sf::FloatRect BubbleBounds(0.f, 0.f, 64.f, 64.f);
static const sf::Vector2f BubbleOrigin(BubbleBounds.width / 2.f, BubbleBounds.height / 2.f);

static const sf::FloatRect WhirlyBobBounds(0.f, 0.f, 64.f, 64.f);
static const sf::Vector2f WhirlyBobOrigin(WhirlyBobBounds.width / 2.f, WhirlyBobBounds.height / 2.f);

static const sf::FloatRect GooblyBounds = WhirlyBobBounds;
static const sf::Vector2f GooblyOrigin = WhirlyBobOrigin;

static const sf::FloatRect BalldockBounds = WhirlyBobBounds;
static const sf::Vector2f BalldockOrigin = WhirlyBobOrigin;
static const sf::FloatRect BalldockFoot(-1.f, 64.f, 66.f, 10.f);

static const sf::FloatRect SquatmoBounds = { 6.f, 0.f, 52.f, 64.f };
static const sf::Vector2f SquatmoOrigin = { SquatmoBounds.width / 2.f, SquatmoBounds.height / 2.f };
static const sf::FloatRect SquatmoFoot(-5.f, 64.f, 54.f, 10.f);

static const sf::FloatRect ClocksyBounds(6.f, 12.f, 52.f, 52.f);
static const sf::FloatRect ClocksyFoot(6.f, 64.f, 52.f, 10.f);
static const sf::Vector2f ClocksyOrigin(32.f, 38.f);

static const sf::FloatRect PlayerBounds = ClocksyBounds;
static const sf::FloatRect PlayerFoot = ClocksyFoot;
static const sf::Vector2f PlayerOrigin(32.f, 64.f);

static const sf::FloatRect SmallFoodBounds(0.f, 0.f, 64.f, 64.f);
static const sf::Vector2f SmallFoodOrigin(32.f, 32.f);

static const sf::FloatRect CrateBounds(2.f, 2.f, 60.f, 62.f);
static const sf::FloatRect CrateFoot = ClocksyFoot;

static constexpr float TeleportDistance = 886.f;
static constexpr float Gravity = 2200.f;
static constexpr float MaxVelocity = 800.f;

static const sf::FloatRect MapBounds(0.f, 0.f, 16.f * 64.f, 17.f * 64.f);

static const sf::Vector2f PlayerOneSpawn(104.f, 920.f);
static const sf::Vector2f PlayerTwoSpawn(920.f, 920.f);
static const float PlayerInvincibleTime = 2.f;
static const sf::Uint8 PlayerStartLives = 3;

static const sf::Vector2f PowerupOneSpawn(352.f, 960.f);
static const sf::Vector2f PowerupTwoSpawn(672.f, 960.f);
static const float TopSpawn = 896.f;

static const sf::Vector2f TowerSpawnOne(-192.f, 1024.f);
static const sf::Vector2f TowerSpawnTwo(MapBounds.width + 128.f, 1024.f);

static const sf::Color BubbleColourOne(255, 212, 0);
static const sf::Color BubbleColourTwo(255, 0, 212);

static const sf::Uint8 MapsToWin = 24; //divisible by 4 for colour cycle

//map loading functions shared between client / server
void createCollisionObject(xy::Scene& scene, const tmx::Object&, CollisionType::ID type);

std::string getSha(const std::string&);

#endif //DEMO_CLIENT_SERVER_SHARED_HPP_