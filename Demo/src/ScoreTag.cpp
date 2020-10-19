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

#include "ScoreTag.hpp"

#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>

const float ScoreTag::MaxLife = 2.f;

namespace
{
    const float speed = -120.f;
}

ScoreTagSystem::ScoreTagSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(ScoreTagSystem))
{
    requireComponent<ScoreTag>();
    requireComponent<xy::Text>();
    requireComponent<xy::Transform>();
}

//public
void ScoreTagSystem::process(float dt)
{
    auto& entities = getEntities();
    for (auto entity : entities)
    {
        auto& tag = entity.getComponent<ScoreTag>();
        tag.lifetime -= dt;
        if (tag.lifetime < 0)
        {
            getScene()->destroyEntity(entity);
            return;
        }

        auto alpha = 255.f * (tag.lifetime / tag.MaxLife);

        tag.colour.a = static_cast<std::uint8_t>(alpha);
        
        entity.getComponent<xy::Text>().setFillColour(tag.colour);
        entity.getComponent<xy::Transform>().move(0.f, speed * dt);
    }
}
