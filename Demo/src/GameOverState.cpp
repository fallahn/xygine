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
#include "MessageIDs.hpp"
#include "Localisation.hpp"

#include <xyginext/gui/Gui.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Callback.hpp>

#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>

namespace
{
    struct Timer final
    {
        float time = 30.f;
        xy::MessageBus& messageBus;

        explicit Timer(xy::MessageBus& mb) : messageBus(mb) {}
        void operator () (xy::Entity entity, float dt)
        {
            entity.getComponent<xy::Text>().setString(std::to_string(std::max(static_cast<std::int32_t>(time), 0)));

            time -= dt;
            if (time < 0)
            {
                messageBus.post<MenuEvent>(MessageID::MenuMessage)->action = MenuEvent::QuitGameClicked;
            }
        }
    };
}

GameoverState::GameoverState(xy::StateStack& stack, xy::State::Context ctx, const SharedStateData& data)
    : xy::State(stack, ctx),
    m_scene(ctx.appInstance.getMessageBus())
{
    m_buttonTexture.loadFromFile(xy::FileSystem::getResourcePath() + "assets/images/button.png");
    m_font.loadFromFile(xy::FileSystem::getResourcePath() + "assets/fonts/Cave-Story.ttf");

    sf::Image img;
    img.create(1, 1, sf::Color(0, 0, 0, 180));
    m_backgroundTexture.loadFromImage(img);

    initScene();

    if (data.continueCount > 0 && data.continueCount < 4)
    {
        loadContinue(data);
    }
    else
    {
        loadGameOver(data);
    }

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());

    xy::App::setMouseCursorVisible(true);
}

GameoverState::~GameoverState()
{
    xy::App::setMouseCursorVisible(false);
}

bool GameoverState::handleEvent(const sf::Event& evt)
{
    // Don't process events which the gui wants
    if (xy::ui::wantsMouse() || xy::ui::wantsKeyboard())
    {
        return true;
    }
    
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
void GameoverState::initScene()
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::TextSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);   
    
    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>(m_backgroundTexture);
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize);
}

void GameoverState::loadContinue(const SharedStateData& data)
{
    //title text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -460.f);
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::Continue]);
    entity.getComponent<xy::Text>().setCharacterSize(260);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Drawable>().setDepth(2);

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -210.f);
    entity.addComponent<xy::Text>(m_font).setString(std::to_string(data.continueCount) + " TRIES REMAINING");
    entity.getComponent<xy::Text>().setCharacterSize(70);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Drawable>().setDepth(2);

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -140.f);
    entity.addComponent<xy::Text>(m_font).setString("30");
    entity.getComponent<xy::Text>().setCharacterSize(80);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Drawable>().setDepth(2);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = Timer(getContext().appInstance.getMessageBus());

    //score text
    for (auto i = 0u; i < data.playerCount; ++i)
    {
        entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);       
        entity.addComponent<xy::Text>(m_font).setFillColour(sf::Color::Red);
        entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);

        if (data.playerCount == 1)
        {
            entity.getComponent<xy::Transform>().move(0.f, -100.f);
            entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::Score] + data.scores[0]);
            entity.getComponent<xy::Text>().setCharacterSize(140);
        }
        else
        {
            entity.getComponent<xy::Text>().setCharacterSize(100);
            if (i == 0)
            {
                entity.getComponent<xy::Transform>().move(0.f, -100.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerOneScore] + data.scores[0]);
            }
            else
            {
                entity.getComponent<xy::Transform>().move(0.f, -20.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerTwoScore] + data.scores[1]);
            }
        }
    }

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


    //YES button text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::Yes]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(38.f, 45.f);

    //YES button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    auto bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 256.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(-bounds.width / 2.f, 256.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, std::uint64_t flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::ContinueGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::ContinueGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, std::uint32_t, std::uint32_t button)
    {
        if (button == 0)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::ContinueGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;



    //NO button text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::No]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(32.f, 45.f);

    // NO button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 256.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(bounds.width / 2.f, 256.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, std::uint64_t flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, std::uint32_t, std::uint32_t button)
    {
        if (button == 0)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
 
}

void GameoverState::loadGameOver(const SharedStateData& data)
{
    //title text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -460.f);
    entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::GameOver]);
    entity.getComponent<xy::Text>().setCharacterSize(260);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Drawable>().setDepth(2);

    //score text
    for (auto i = 0u; i < data.playerCount; ++i)
    {
        entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
        entity.addComponent<xy::Text>(m_font).setFillColour(sf::Color::Red);
        entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
        entity.addComponent<xy::Drawable>().setDepth(2);

        if (data.playerCount == 1)
        {
            entity.getComponent<xy::Transform>().move(0.f, -100.f);
            entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::Score] + data.scores[0]);
            entity.getComponent<xy::Text>().setCharacterSize(140);
        }
        else
        {
            entity.getComponent<xy::Text>().setCharacterSize(100);
            if (i == 0)
            {
                entity.getComponent<xy::Transform>().move(0.f, -100.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerOneScore] + data.scores[0]);
            }
            else
            {
                entity.getComponent<xy::Transform>().move(0.f, -20.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerTwoScore] + data.scores[1]);
            }
        }
    }

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


    //OK button text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(m_font).setString("OK");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    //tx2.setOrigin(38.f, 45.f);

    //OK button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    auto bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 256.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 256.f);

    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, (-bounds.height / 2.f) * 0.75f);
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, std::uint64_t flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, std::uint32_t, std::uint32_t button)
    {
        if (button == 0)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::QuitGameClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;



    //NO button text
    //entity = m_scene.createEntity();
    //entity.addComponent<xy::Text>(m_font).setString(Locale::Strings[Locale::No]);
    //entity.getComponent<xy::Text>().setCharacterSize(60);
    //entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    //auto& tx = entity.addComponent<xy::Transform>();
    //tx.setOrigin(32.f, 45.f);

    //// NO button
    //entity = m_scene.createEntity();
    //entity.addComponent<xy::Sprite>().setTexture(m_buttonTexture);
    //bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    //entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 256.f, bounds.width, bounds.height / 4.f });
    //entity.addComponent<xy::Drawable>();
    //entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    //entity.getComponent<xy::Transform>().addChild(tx);
    //entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    //entity.getComponent<xy::Transform>().move(bounds.width / 2.f, 128.f);
    //tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    //bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    //entity.addComponent<xy::UIHitBox>().area = bounds;
    //entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
    //    m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, std::uint64_t flags)
    //{
    //    if (flags & xy::UISystem::LeftMouse)
    //    {
    //        auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
    //        msg->action = MenuEvent::QuitGameClicked;
    //    }
    //});
    //entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
    //    m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    //{
    //    if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
    //    {
    //        auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
    //        msg->action = MenuEvent::QuitGameClicked;
    //    }
    //});
    //entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
    //    m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, std::uint32_t, std::uint32_t button)
    //{
    //    if (button == 0)
    //    {
    //        auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
    //        msg->action = MenuEvent::QuitGameClicked;
    //    }
    //});
    //entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    //entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;


}
