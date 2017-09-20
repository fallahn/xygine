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

        const auto currPos = xForm.getPosition();
        auto vel = controller.lastPostion - currPos;


        switch (controller.actorID)
        {
        case ActorID::Client:
            //this is our local player
        {
            float scale = entity.getComponent<Player>().direction == Player::Direction::Left ? 1.f : -1.f;
            xForm.setScale(scale, 1.f);
        }
            break;
        case ActorID::PlayerOne:
        case ActorID::PlayerTwo:
            //these are remote versions

        case ActorID::Clocksy:
        default:
            //set direction
            if ((controller.lastVelocity.x <= 0 && vel.x > 0)
                || (controller.lastVelocity.x >= 0 && vel.x < 0))
            {
                xForm.setScale(vel.x / std::abs(vel.x), 1.f);
            }
            break;
        }

        //use velocity to decide which animation should be playing
        AnimationController::Animation anim = AnimationController::Animation::Idle;
        if (vel.x != 0) anim = AnimationController::Animation::Walk;
        if (vel.y < 0) anim = AnimationController::Animation::JumpUp;
        else if (vel.y > 0.2f) anim = AnimationController::Animation::JumpDown;

        if (anim != controller.currentAnim &&
            controller.currentAnim == controller.previousAnimation) //we're not being overridden right now
        {
            //set SpriteAnimation
            entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[anim]);
            controller.currentAnim = controller.previousAnimation = anim;
        }



        if (controller.previousAnimation != controller.currentAnim)
        {
            //check if overriding anim (such as shooting/dying) has finished
            //and revert to previously playing animation
            if (entity.getComponent<xy::SpriteAnimation>().stopped())
            {
                entity.getComponent<xy::SpriteAnimation>().play(controller.previousAnimation);
                controller.currentAnim = controller.previousAnimation;
            }
        }
        controller.lastPostion = currPos;
        controller.lastVelocity = vel;
    }
}