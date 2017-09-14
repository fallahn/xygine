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

#include "AnimationController.hpp"

#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/Transform.hpp>

AnimationControllerSystem::AnimationControllerSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(AnimationControllerSystem))
{
    requireComponent<AnimationController>();
    requireComponent<xy::Transform>();
    requireComponent<xy::SpriteAnimation>();
}

void AnimationControllerSystem::process(float)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& xForm = entity.getComponent<xy::Transform>();
        auto& controller = entity.getComponent<AnimationController>();

        const auto currPos = xForm.getPosition();
        auto vel = controller.lastPostion - currPos;

        //set direction
        if ((controller.lastVelocity.x <= 0 && vel.x > 0)
            || (controller.lastVelocity.x >= 0 && vel.x < 0))
        {
            xForm.setScale(vel.x / std::abs(vel.x), 1.f);
        }

        //use velocity to decide which animation should be playing
        AnimationController::Animation anim = AnimationController::Animation::Idle;
        if (vel.x != 0) anim = AnimationController::Animation::Walk;
        if (vel.y < 0) anim = AnimationController::Animation::JumpUp;
        else if (vel.y > 0) anim = AnimationController::Animation::JumpDown;

        if (anim != controller.currentAnim)
        {
            //set SpriteAnimatior
            entity.getComponent<xy::SpriteAnimation>().play(anim);

            controller.currentAnim = anim;
        }

        //check if overriding anim (such as shooting/dying) has finished
        //and revert to previously playing animation


        controller.lastPostion = currPos;
        controller.lastVelocity = vel;
    }
}