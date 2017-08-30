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

        sf::Vector2f motion;
        if (player.input.mask & InputFlag::Up)
        {
            motion.y = -1.f;
        }
        if (player.input.mask & InputFlag::Down)
        {
            motion.y += 1.f;
        }
        if (player.input.mask & InputFlag::Left)
        {
            motion.x = -1.f;
        }
        if (player.input.mask & InputFlag::Right)
        {
            motion.x += 1.f;
        }

        //normalise if not along one axis
        if (xy::Util::Vector::lengthSquared(motion) > 1)
        {
            motion = xy::Util::Vector::normalise(motion);
        }

        //create the delta time from timestamp input
        auto prevInput = (player.currentInput + player.history.size() - 2) % player.history.size();
        auto delta = player.input.timestamp - player.history[prevInput].timestamp;
        tx.move(motion * speed * static_cast<float>(delta) / 1000.f);
    }
}

void PlayerSystem::reconcile(float x, float y, sf::Int32 timestamp, xy::Entity player)
{
    //DPRINT("Reconcile to: ", std::to_string(x) + ", " + std::to_string(y));
}