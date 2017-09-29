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

#include "GameOverState.hpp"
#include "SharedStateData.hpp"

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

GameoverState::GameoverState(xy::StateStack& stack, xy::State::Context ctx, const SharedStateData& data)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus())
{
    m_buttonTexture.loadFromFile("assets/images/button.png");
    m_font.loadFromFile("assets/fonts/Cave-Story.ttf");

    sf::Image img;
    img.create(1, 1, sf::Color(0, 0, 0, 120));
    m_backgroundTexture.loadFromImage(img);

    load(data);
}

bool GameoverState::handleEvent(const sf::Event& evt)
{
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    m_scene.forwardEvent(evt);
    return false;
}

void GameoverState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool GameoverState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void GameoverState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private
void GameoverState::load(const SharedStateData& data)
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);

    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>(m_backgroundTexture);
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize);

    //title text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(-490.f, -560.f);
    entity.addComponent<xy::Text>(m_font).setString("GAME OVER");
    entity.getComponent<xy::Text>().setCharacterSize(260);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);

    //score text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(-360.f, -220.f);
    entity.addComponent<xy::Text>(m_font).setString("Score: " + data.score);
    entity.getComponent<xy::Text>().setCharacterSize(140);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);

    //button text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString("OK");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(32.f, 45.f);

    //button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getLocalBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 128.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getLocalBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            requestStackClear();
            requestStackPush(StateID::MainMenu);
        }
    });

    

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}
