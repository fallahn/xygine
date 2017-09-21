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

#include <tmxlite/Object.hpp>

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

namespace CollisionFlags
{
    enum
    {
        Solid = 0x1,
        Platform = 0x2,
        Teleport = 0x4,
        Player = 0x8,
        Bubble = 0x10,
        NPC = 0x20,

        PlayerMask = Bubble | Platform | Solid | Teleport | NPC,
        NPCMask = Solid | Player | Bubble | Platform | Teleport
    };
}

static constexpr float PlayerSize = 60.f;
static constexpr float PlayerSizeOffset = 2.f;
static constexpr float PlayerFootSize = 10.f;

static constexpr float BubbleVerticalVelocity = -100.f;
static constexpr float BubbleSize = 64.f;
static constexpr float NPCSize = 64.f;

static constexpr float TeleportDistance = 950.f;

static const sf::FloatRect MapBounds(0.f, 0.f, 16.f *64.f, 17.f * 64.f);

//map loading functions shared between client / server
void createCollisionObject(xy::Scene& scene, const tmx::Object&, CollisionType type);

std::string getSha(const std::string&);

#endif //DEMO_CLIENT_SERVER_SHARED_HPP_