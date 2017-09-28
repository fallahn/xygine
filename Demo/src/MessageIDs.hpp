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

#ifndef DEMO_MESSAGE_IDS_HPP_
#define DEMO_MESSAGE_IDS_HPP_

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
        MapMessage
    };
}

struct PlayerEvent final
{
    enum
    {
        Spawned,
        FiredWeapon,
        Jumped,
        Died
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
    sf::Int32 actorID = -1;
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
    sf::Int32 entityID = 0;
    sf::Uint8 playerID = 0; //who gets points?
};

//when an item such as fruit or bonus collected
struct ItemEvent final
{
    sf::Uint8 playerID = 0;
    sf::Int32 actorID = -1;
};

struct AnimationEvent final
{
    sf::Int32 newAnim = -1;
    sf::Int32 oldAnim = -1;
    xy::Entity entity;
    float x = 0.f;
    float y = 0.f;
};

struct MapEvent final
{
    enum
    {
        AnimationComplete
    }type;
};

#endif //DEMO_MESSAGE_IDS_HPP_