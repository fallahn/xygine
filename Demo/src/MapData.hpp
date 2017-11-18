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

#ifndef DEMO_GAME_MAP_DATA_HPP_
#define DEMO_GAME_MAP_DATA_HPP_

#include "PlayerSystem.hpp"

#include <SFML/Config.hpp>

namespace ActorID
{
    enum
    {
        None = -1,
        PlayerOne, PlayerTwo,
        Whirlybob, Clocksy, Goobly, Balldock, Squatmo,
        BubbleOne, BubbleTwo,
        FruitSmall, FruitLarge,
        FlameOne, FlameTwo,
        LightningOne, LightningTwo,
        Bonus,
        PrincessOne, PrincessTwo,
        TowerOne, TowerTwo,
        MagicHat, Crate, Explosion
    };
}

struct Velocity final
{
    float x = 0.f;
    float y = 0.f;
};

//entities such as players or NPC
//projectiles and bonuses
struct Actor final
{
    sf::Int16 type = -1;
    sf::Int16 id = -1;
};

//this is sent to a client when it has connected successfully
static constexpr sf::Uint8 MaxChars = 11;
static constexpr sf::Uint8 MaxNPCs = 12;
static constexpr sf::Uint8 MaxCrates = 6;
struct MapData final
{
    char mapName[MaxChars]{};
    char mapSha[41]{}; //sha1 is always 40 chars long
    Actor NPCs[MaxNPCs]{};
    sf::Int8 NPCCount = 0;
    Actor crates[MaxCrates]{};
    sf::Int8 crateCount = 0;
    sf::Uint8 colourQuad = 0;
};

//the actor ID, spawn position and Player number
//of a client, as assigned by the server
struct ClientData final
{
    Actor actor;
    float spawnX = 0.f;
    float spawnY = 0.f;
    sf::Uint32 peerID = 0;
    sf::Uint8 playerNumber = 0;
};

//actor and its position
//sent as part of an update, or containing initial positions
struct ActorState
{
    float x = 0.f;
    float y = 0.f;
    Actor actor;
    sf::Int32 serverTime = 0;
    float animationDirection = 1.f;
    sf::Int32 animationID = 0;
};
//client state for client side reconciliation
struct ClientState final : public ActorState
{
    sf::Int64 clientTime = 0;
    Player::SyncState sync;
    //Player::State playerState = Player::State::Walking;
    //float playerVelocity = 0.f;
    //float playerTimer = 0.f;
    //sf::Uint8 playerCanLand = 0;
    //sf::Uint8 playerLives = 3;
    //sf::Uint8 boolFlags;
};

//actor events for spawn/despawn etc
struct ActorEvent final : public ActorState
{
    enum
    {
        Spawned, Died, GotAngry
    }type;
};

//update to the server from the client's input
struct InputUpdate final
{
    sf::Uint16 input = 0; //input mask
    sf::Int64 clientTime = 0; //client timestamp this input was received
    sf::Uint8 playerNumber = 0; //player one or two
};

namespace InputFlag
{
    enum
    {
        Up = 0x1,
        Down = 0x2,
        Left = 0x4,
        Right = 0x8,
        Jump = 0x10,
        Shoot = 0x20
    };
}

//player inventory update
struct InventoryUpdate final
{
    sf::Uint8 playerID = 0;
    sf::Uint32 score = 0;
    sf::Uint32 amount = 0;
    sf::Uint8 lives = 0;
    sf::Uint8 bonusFlags = 0;
};

struct CollisionFlagsUpdate final
{
    sf::Int16 actor = -1;
    sf::Uint32 newflags = 0;
};


#endif //DEMO_GAME_MAP_DATA_HPP_