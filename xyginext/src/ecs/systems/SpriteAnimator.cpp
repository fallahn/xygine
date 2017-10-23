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

#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>

#include <xyginext/core/Message.hpp>

using namespace xy;

SpriteAnimator::SpriteAnimator(MessageBus& mb)
    : System(mb, typeid(SpriteAnimator))
{
    requireComponent<Sprite>();
    requireComponent<SpriteAnimation>();
}

//public
void SpriteAnimator::process(float dt)
{
    auto& entities = getEntities();
    for (auto& entity : entities) 
    {
        auto& animation = entity.getComponent<SpriteAnimation>();
        if (animation.m_playing)
        {
            auto& sprite = entity.getComponent<Sprite>();
            animation.m_currentFrameTime -= dt;
            if (animation.m_currentFrameTime < 0)
            {
                XY_ASSERT(sprite.m_animations[animation.m_id].framerate > 0, "Illegal Frame Rate");
                XY_ASSERT(sprite.m_animations[animation.m_id].frameCount > 0, "Illegal Frame Count");
                animation.m_currentFrameTime += (1.f / sprite.m_animations[animation.m_id].framerate);

                auto lastFrame = animation.m_frameID;
                animation.m_frameID = (animation.m_frameID + 1) % sprite.m_animations[animation.m_id].frameCount;

                if (animation.m_frameID < lastFrame && !sprite.m_animations[animation.m_id].looped)
                {
                    animation.stop();
                    continue;
                }

                sprite.setTextureRect(sprite.m_animations[animation.m_id].frames[animation.m_frameID]);
            }
        }
    }
}
