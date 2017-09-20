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
#include "MessageIDs.hpp"
#include "MapData.hpp"

#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/Transform.hpp>

#include <xyginext/core/Message.hpp>

AnimationControllerSystem::AnimationControllerSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(AnimationControllerSystem))
{
    requireComponent<AnimationController>();
    requireComponent<xy::Transform>();
    requireComponent<xy::SpriteAnimation>();
}

void AnimationControllerSystem::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::PlayerMessage)
    {
        const auto& data = msg.getData<PlayerEvent>();
        auto entity = data.entity;
        auto& controller = entity.getComponent<AnimationController>();
        controller.prevAnimation = controller.currentAnim;
        switch (data.type)
        {
        default: break;
        case PlayerEvent::Died:
            controller.currentAnim = AnimationController::Animation::Die;
            break;
        case PlayerEvent::FiredWeapon:
            controller.currentAnim = AnimationController::Animation::Shoot;
            break;
        }
        entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[controller.currentAnim]);
    }
}

void AnimationControllerSystem::process(float)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& xForm = entity.getComponent<xy::Transform>();
        auto& controller = entity.getComponent<AnimationController>();

        xForm.setScale(controller.direction, 1.f);

        //if animation has changed update it
        if (controller.nextAnimation != controller.currentAnim
            && controller.prevAnimation == controller.currentAnim)
        {
            controller.prevAnimation = controller.currentAnim = controller.nextAnimation;
            entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[controller.currentAnim]);
        }

        //if overriding anim such as shooting is playing
        //check to see if it has stopped
        if (controller.prevAnimation != controller.currentAnim)
        {
            auto& sprAnim = entity.getComponent<xy::SpriteAnimation>();
            if (sprAnim.stopped())
            {
                controller.currentAnim = controller.prevAnimation;
                sprAnim.play(controller.animationMap[controller.currentAnim]);
            }
        }
    }
}