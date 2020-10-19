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
#include "MessageIDs.hpp"
#include "Localisation.hpp"

#include <xyginext/gui/Gui.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Color ButtonActiveColour(220, 100, 100);
}

PauseState::PauseState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus())
{
    m_buttonTexture.loadFromFile(xy::FileSystem::getResourcePath() + "assets/images/button.png");
    m_font.loadFromFile(xy::FileSystem::getResourcePath() + "assets/fonts/Cave-Story.ttf");

    sf::Image img;
    img.create(1, 1, sf::Color(0, 0, 0, 180));
    m_backgroundTexture.loadFromImage(img);

    load();

    xy::App::setMouseCursorVisible(true);
}

PauseState::~PauseState()
{
    xy::App::setMouseCursorVisible(false);
}

bool PauseState::handleEvent(const sf::Event& evt)
{
    // Don't process events which the gui wants
    if (xy::ui::wantsMouse() || xy::ui::wantsKeyboard())
    {
        return true;
    }
    
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
            unpause();
            break;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        if (evt.joystickButton.joystickId == 0)
        {
            if (evt.joystickButton.button == 7) //start on xbox
            {
                unpause();
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
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::TextSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);   

    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>(m_backgroundTexture);
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize);

    //button callbacks
    auto selectedID = m_scene.getSystem<xy::UISystem>().addSelectionCallback(
        [](xy::Entity entity)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        auto rect = sprite.getTextureRect();
        rect.top = 256.f;
        sprite.setTextureRect(rect);
    });
    auto unselectedID = m_scene.getSystem<xy::UISystem>().addSelectionCallback(
        [](xy::Entity entity)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        auto rect = sprite.getTextureRect();
        rect.top = 0.f;
        sprite.setTextureRect(rect);
    });

    //resume text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::Resume]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(72.f, 45.f);

    //resume button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -128.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, std::uint64_t flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            unpause();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            unpause();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, std::uint32_t, std::uint32_t button)
    {
        if (button == 0)
        {
            unpause();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;

    //quit text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::Quit]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(48.f, 45.f);


    //quit button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity entity, std::uint64_t flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            entity.getComponent<xy::Sprite>().setColour(ButtonActiveColour);
            
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            entity.getComponent<xy::Sprite>().setColour(ButtonActiveColour);

            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity entity, std::uint32_t, std::uint32_t button)
    {
        if (button == 0)
        {
            entity.getComponent<xy::Sprite>().setColour(ButtonActiveColour);

            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}

void PauseState::unpause() 
{
    auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
    msg->action = MenuEvent::UnpauseGame;
}
