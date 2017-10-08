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

#include "TowerDirector.hpp"
#include "MessageIDs.hpp"
#include "AnimationController.hpp"
#include "CommandIDs.hpp"
#include "ActorSystem.hpp"

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/Scene.hpp>

TowerDirector::TowerDirector()
{

}

//public
void TowerDirector::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::AnimationMessage)
    {
        const auto& data = msg.getData<AnimationEvent>();
        if (data.newAnim == AnimationController::Die)
        {
            auto actorID = data.entity.getComponent<Actor>().type;
            
            xy::Command cmd;
            cmd.targetFlags = CommandID::Princess;
            cmd.action = [actorID](xy::Entity entity, float)
            {
                auto ourID = entity.getComponent<Actor>().type;
                if ((ourID == ActorID::PrincessOne && actorID == ActorID::PlayerOne)
                    || (ourID == ActorID::PrincessTwo && actorID == ActorID::PlayerTwo))
                {
                    auto& controller = entity.getComponent<AnimationController>();
                    controller.currentAnim = AnimationController::Shoot;
                    entity.getComponent<xy::SpriteAnimation>().stop(); //rewinds the frame position
                    entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[AnimationController::Shoot]);
                }
                //TODO clap when killing a baddie
            };
            sendCommand(cmd);
        }
    }
}

void TowerDirector::handleEvent(const sf::Event&)
{

}

void TowerDirector::process(float)
{

}