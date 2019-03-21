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

#include "GameState.hpp"
#include "States.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>

#include <xyginext/core/App.hpp>

#include <SFML/Window/Event.hpp>

GameState::GameState(xy::StateStack& ss, xy::State::Context ctx) :
xy::State(ss,ctx),
m_gameScene(ctx.appInstance.getMessageBus())
{
    createScene();
    
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

    ctx.appInstance.setMouseCursorVisible(false);
}

bool GameState::handleEvent(const sf::Event& evt)
{
    //look for mouse move events and send them to the paddle
    if(evt.type == sf::Event::MouseMoved)
    {
        auto worldMousePosition = xy::App::getRenderWindow()->mapPixelToCoords({evt.mouseMove.x, evt.mouseMove.y});
        xy::Command cmd;
        cmd.targetFlags = CommandID::Paddle;
        cmd.action = [worldMousePosition](xy::Entity entity, float)
        {
            //clamp the X position in the screen area minus the sprite width
            float posX = worldMousePosition.x;
            auto spriteWidth = entity.getComponent<xy::Sprite>().getTextureBounds().width / 2.f;
            posX = std::max(spriteWidth, worldMousePosition.x);
            posX = std::min(posX, xy::DefaultSceneSize.x - spriteWidth);

            auto& tx = entity.getComponent<xy::Transform>();
            auto currentPosition = tx.getPosition();
            currentPosition.x = posX;
            tx.setPosition(currentPosition);
        };
        m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }

    m_gameScene.forwardEvent(evt);
    return true;
}

void GameState::handleMessage(const xy::Message& msg)
{
    m_gameScene.forwardMessage(msg);
}

bool GameState::update(float dt)
{
    m_gameScene.update(dt);
    return true;
}

void GameState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_gameScene);
}

xy::StateID GameState::stateID() const
{
    return States::GameState;
}

void GameState::createScene()
{
    //add the systems
    auto& messageBus = getContext().appInstance.getMessageBus();
    m_gameScene.addSystem<xy::CommandSystem>(messageBus);
    m_gameScene.addSystem<xy::SpriteSystem>(messageBus);
    m_gameScene.addSystem<xy::RenderSystem>(messageBus);

    m_paddleTexture.loadFromFile("assets/images/paddle.png");

    auto entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x / 2.f, xy::DefaultSceneSize.y - 40.f);
    entity.addComponent<xy::Sprite>(m_paddleTexture);
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Paddle;

    auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Transform>().setOrigin(paddleBounds.width / 2.f, paddleBounds.height / 2.f);
}









