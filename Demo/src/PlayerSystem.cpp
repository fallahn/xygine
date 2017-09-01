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

#include "PlayerSystem.hpp"
#include "MapData.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/core/App.hpp>

namespace
{
    const float speed = 1600.f;
}

PlayerSystem::PlayerSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(PlayerSystem))
{
    requireComponent<Player>();
    requireComponent<xy::Transform>();
}

//public
void PlayerSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& player = entity.getComponent<Player>();
        auto& tx = entity.getComponent<xy::Transform>();

        //current input actually points to next empty slot.
        std::size_t idx = (player.currentInput + player.history.size() - 1) % player.history.size();

        //parse any outstanding inputs
        while (player.lastUpdatedInput != idx)
        {   
            tx.move(speed * parseInput(player.history[player.lastUpdatedInput].mask) * getDelta(player.history, player.lastUpdatedInput));
            player.lastUpdatedInput = (player.lastUpdatedInput + 1) % player.history.size();  
        }
    }
}

void PlayerSystem::reconcile(float x, float y, sf::Int64 timestamp, xy::Entity entity)
{
    //DPRINT("Reconcile to: ", std::to_string(x) + ", " + std::to_string(y));

    auto& player = entity.getComponent<Player>();
    auto& tx = entity.getComponent<xy::Transform>();

    tx.setPosition(x, y);

    //find the oldest timestamp not used by server
    auto ip = std::find_if(player.history.rbegin(), player.history.rend(),
        [timestamp](const Input& input)
    {
        return (timestamp == input.timestamp);
    });

    //and reparse inputs
    if (ip != player.history.rend())
    {
        auto idx = std::distance(player.history.rbegin(), ip);
        while (idx != player.currentInput) //currentInput points to the next free slot in history
        {
            tx.move(speed * parseInput(player.history[idx].mask) * getDelta(player.history, idx));
            idx = (idx + 1) % player.history.size();
        }
    }
}

//private
sf::Vector2f PlayerSystem::parseInput(sf::Uint16 mask)
{
    sf::Vector2f motion;
    if (mask & InputFlag::Up)
    {
        motion.y = -1.f;
    }
    if (mask & InputFlag::Down)
    {
        motion.y += 1.f;
    }
    if (mask & InputFlag::Left)
    {
        motion.x = -1.f;
    }
    if (mask & InputFlag::Right)
    {
        motion.x += 1.f;
    }

    //normalise if not along one axis
    if (xy::Util::Vector::lengthSquared(motion) > 1)
    {
        motion = xy::Util::Vector::normalise(motion);
    }

    return motion;
}

float PlayerSystem::getDelta(const History& history, std::size_t idx)
{
    auto prevInput = (idx + history.size() - 1) % history.size();
    auto delta = history[idx].timestamp - history[prevInput].timestamp;

    return static_cast<float>(delta) / 1000000.f;
}