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

#include "PauseState.hpp"

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

#include <SFML/Window/Event.hpp>

PauseState::PauseState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus())
{
    m_buttonTexture.loadFromFile("assets/images/button.png");
    m_font.loadFromFile("assets/fonts/Cave-Story.ttf");

    sf::Image img;
    img.create(1, 1, sf::Color(0, 0, 0, 180));
    m_backgroundTexture.loadFromImage(img);

    load();
}

bool PauseState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
            requestStackPop();
            break;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        if (evt.joystickButton.joystickId == 0)
        {
            if (evt.joystickButton.button == 7) //start on xbox
            {
                requestStackPop();
            }
        }
    }
    
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    m_scene.forwardEvent(evt);
    return false;
}

void PauseState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool PauseState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void PauseState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private
void PauseState::load()
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);

    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>(m_backgroundTexture);
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize);

    //resume text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString("RESUME");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(72.f, 45.f);

    //resume button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getLocalBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -128.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getLocalBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            requestStackPop();
        }
    });

    //quit text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString("QUIT");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(48.f, 45.f);


    //quit button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getLocalBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            entity.getComponent<xy::Sprite>().setColour(sf::Color(200, 200, 200));
            
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