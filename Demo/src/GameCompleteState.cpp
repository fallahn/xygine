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

#include "GameCompleteState.hpp"
#include "SharedStateData.hpp"
#include "StateIDs.hpp"

#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>

#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

#include <SFML/Window/Event.hpp>

GameCompleteState::GameCompleteState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sd)
    : xy::State (stack, ctx),
    m_sharedData(sd),
    m_scene     (ctx.appInstance.getMessageBus())
{

}

//public
bool GameCompleteState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased
        || evt.type == sf::Event::JoystickButtonReleased)
    {
        //TODO make this skip ending and display summary

        requestStackClear();
        requestStackPush(StateID::MainMenu);
    }
    
    //forward scene event
    m_scene.forwardEvent(evt);

    //TODO forward UI event
    return false;
}

void GameCompleteState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool GameCompleteState::update(float dt)
{
    m_scene.update(dt);
    return false;
}

void GameCompleteState::draw()
{
    auto& rt = getContext().renderWindow;

    rt.draw(m_scene);
}