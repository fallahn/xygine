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

#include <SFML/Config.hpp>

#include <xyginext/core/Message.hpp>
#include <xyginext/ecs/Entity.hpp>

namespace MessageID
{
    enum
    {
        PlayerMessage = xy::Message::Count,
        SceneMessage,
        AnimationMessage,
        NpcMessage,
        ItemMessage,
        MapMessage,
        NetworkMessage,
        MenuMessage,
        GameMessage
    };
}

struct PlayerEvent final
{
    enum
    {
        Spawned,
        FiredWeapon,
        Jumped,
        Died,
        GotHat,
        LostHat,
        GotExtraLife
    }type;
    xy::Entity entity;
};

struct SceneEvent final
{
    enum
    {
        ActorSpawned,
        ActorRemoved
    }type;
    xy::Entity entity;
    std::int32_t actorID = -1;
    float x = 0.f;
    float y = 0.f;
};

struct NpcEvent final
{
    enum
    {
        Died,
        Spawned,
        GotAngry
    }type = Died;
    std::int32_t entityID = 0;
    std::uint8_t playerID = 0; //who gets points?
    enum
    {
        Bubble,
        Lightning,
        Flame,
        Crate,
        Explosion,
        OutOfBounds
    };
    std::uint8_t causeOfDeath = Bubble;
};

//when an item such as fruit or bonus collected
struct ItemEvent final
{
    xy::Entity player;
    std::int32_t actorID = -1;
};

struct AnimationEvent final
{
    std::int32_t newAnim = -1;
    std::int32_t oldAnim = -1;
    xy::Entity entity;
    float x = 0.f;
    float y = 0.f;
};

struct MapEvent final
{
    enum
    {
        AnimationComplete,
        HurryUp,
        BonusSwitch,
        MapChangeStarted,
        MapChangeComplete
    }type;
};

struct NetworkEvent final
{
    enum
    {
        Connected, Disconnected
    }type = Connected;
    std::uint8_t playerID = 0;
};

struct MenuEvent final
{
    enum
    {
        HelpButtonClicked,
        QuitGameClicked,
        ContinueGameClicked,
        UnpauseGame,
        KeybindClicked
    }action;
    std::uint8_t index = 0;
    std::uint8_t player = 0;
};

struct GameEvent final
{
    enum
    {
        Restarted
    }action;
    std::uint8_t playerID = 0;
};
