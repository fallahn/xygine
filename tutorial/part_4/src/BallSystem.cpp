/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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

#include "BallSystem.hpp"
#include "MessageIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/Scene.hpp>

BallSystem::BallSystem(xy::MessageBus& mb)
 : xy::System(mb, typeid(BallSystem))
{
    requireComponent<Ball>();
    requireComponent<xy::Transform>();
}

 //public
void BallSystem::process(float dt)
{
    auto& entities = getEntities();
    for(auto entity : entities)
    {
        auto& ball = entity.getComponent<Ball>();
        switch(ball.state)
        {
            default:
            case Ball::State::Waiting:             
                break;
            case Ball::State::Active:
            {
                auto& tx = entity.getComponent<xy::Transform>();
                tx.move(ball.velocity * Ball::Speed * dt);

                sf::FloatRect bounds(sf::Vector2f(), xy::DefaultSceneSize);
                if(!bounds.contains(tx.getPosition()))
                {
                    getScene()->destroyEntity(entity);

                    auto* msg = postMessage<BallEvent>(MessageID::BallMessage);
                    msg->action = BallEvent::Despawned;
                    msg->position = tx.getPosition();
                }
            }
                break;
        }
    }
}