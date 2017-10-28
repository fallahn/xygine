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

#include "MenuState.hpp"
#include "CommandIDs.hpp"
#include "StateIDs.hpp"
#include "TextboxDirector.hpp"
#include "MenuDirector.hpp"
#include "SpringFlower.hpp"
#include "LoadingScreen.hpp"
#include "MessageIDs.hpp"
#include "MenuCallbacks.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/AudioListener.hpp>
#include <xyginext/ecs/components/Callback.hpp>

#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>

#include <xyginext/graphics/postprocess/Blur.hpp>

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Math.hpp>

#include <xyginext/core/FileSystem.hpp>

#include <SFML/Window/Event.hpp>

#include <array>


MenuState::MenuState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData, LoadingScreen& ls)
    : xy::State(stack, ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_helpScene         (ctx.appInstance.getMessageBus()),
    m_sharedStateData   (sharedData),
    m_loadingScreen     (ls),
    m_helpShown         (false),
    m_blurEffect        (nullptr)
{
    launchLoadingScreen();
    loadKeybinds();
    createScene();
    createMenu();
    createHelp();
    ctx.appInstance.setClearColour({ 1, 0, 10 });
    ctx.renderWindow.setMouseCursorVisible(true);
    quitLoadingScreen();
}

//public
bool MenuState::handleEvent(const sf::Event& evt)
{
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    m_scene.forwardEvent(evt);

    m_helpScene.getSystem<xy::UISystem>().handleEvent(evt);
    m_helpScene.forwardEvent(evt);

    return true;
}

void MenuState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
    m_helpScene.forwardMessage(msg);

    if (msg.id == MessageID::MenuMessage)
    {
        const auto& data = msg.getData<MenuEvent>();
        if (data.action == MenuEvent::HelpButtonClicked)
        {
            showHelpMenu();
        }
    }
}

bool MenuState::update(float dt)
{
    m_scene.update(dt);
    m_helpScene.update(dt);
    return true;
}

void MenuState::draw()
{
    auto& rt = getContext().renderWindow;

    rt.draw(m_scene);
    rt.draw(m_helpScene);
}

//private
void MenuState::loadKeybinds()
{
    //check user cfg exists
    auto usrDir = xy::FileSystem::getConfigDirectory(dataDir);
    const std::string fileName("keybinds.cfg");

    auto setP1Defaults = [&]()
    {
        auto* p1 = m_keyBinds.addObject("player_one");
        p1->addProperty("left", std::to_string(sf::Keyboard::A));
        p1->addProperty("right", std::to_string(sf::Keyboard::D));
        p1->addProperty("jump", std::to_string(sf::Keyboard::W));
        p1->addProperty("shoot", std::to_string(sf::Keyboard::Space));
        p1->addProperty("controller_id", "0");
        p1->addProperty("controller_jump", "0");
        p1->addProperty("controller_shoot", "1");
    };

    auto setP2Defaults = [&]()
    {
        auto* p2 = m_keyBinds.addObject("player_two");
        p2->addProperty("left", std::to_string(sf::Keyboard::Left));
        p2->addProperty("right", std::to_string(sf::Keyboard::Right));
        p2->addProperty("jump", std::to_string(sf::Keyboard::Up));
        p2->addProperty("shoot", std::to_string(sf::Keyboard::Numpad0));
        p2->addProperty("controller_id", "1");
        p2->addProperty("controller_jump", "0");
        p2->addProperty("controller_shoot", "1");
    };

    m_keyBinds.loadFromFile(usrDir + fileName);

    if (!m_keyBinds.findObjectWithName("player_one"))
    {
        setP1Defaults();
        m_keyBinds.save(usrDir + fileName);
    }

    if (!m_keyBinds.findObjectWithName("player_two"))
    {
        setP2Defaults();
        m_keyBinds.save(usrDir + fileName);
    }

    bool needsUpdate = false;
    auto* binds = m_keyBinds.findObjectWithName("player_one");
    xy::ConfigProperty* property = nullptr;

    //-----player one-----//
    if (property = binds->findProperty("controller_id"))
    {
        m_sharedStateData.inputBindings[0].controllerID = property->getValue<sf::Int32>();
    }
    else
    {
        m_sharedStateData.inputBindings[0].controllerID = 0;
        binds->addProperty("controller_id", "0");
        needsUpdate = true;
    }

    if (property = binds->findProperty("left"))
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Left] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Left] = sf::Keyboard::A;
        binds->addProperty("left", std::to_string(sf::Keyboard::A));
        needsUpdate = true;
    }

    if (property = binds->findProperty("right"))
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Right] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Right] = sf::Keyboard::D;
        binds->addProperty("right", std::to_string(sf::Keyboard::D));
        needsUpdate = true;
    }

    if (property = binds->findProperty("jump"))
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Jump] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Jump] = sf::Keyboard::W;
        binds->addProperty("jump", std::to_string(sf::Keyboard::W));
        needsUpdate = true;
    }

    if (property = binds->findProperty("shoot"))
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Shoot] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[0].keys[InputBinding::Shoot] = sf::Keyboard::Space;
        binds->addProperty("shoot", std::to_string(sf::Keyboard::Space));
        needsUpdate = true;
    }


    //-----player two-----//
    binds = m_keyBinds.findObjectWithName("player_two");
    if (property = binds->findProperty("controller_id"))
    {
        m_sharedStateData.inputBindings[1].controllerID = property->getValue<sf::Int32>();
    }
    else
    {
        m_sharedStateData.inputBindings[1].controllerID = 1;
        binds->addProperty("controller_id", "1");
        needsUpdate = true;
    }

    if (property = binds->findProperty("left"))
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Left] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Left] = sf::Keyboard::Left;
        binds->addProperty("left", std::to_string(sf::Keyboard::Left));
        needsUpdate = true;
    }

    if (property = binds->findProperty("right"))
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Right] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Right] = sf::Keyboard::Right;
        binds->addProperty("right", std::to_string(sf::Keyboard::Right));
        needsUpdate = true;
    }

    if (property = binds->findProperty("jump"))
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Jump] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Jump] = sf::Keyboard::Up;
        binds->addProperty("jump", std::to_string(sf::Keyboard::Up));
        needsUpdate = true;
    }

    if (property = binds->findProperty("shoot"))
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Shoot] = static_cast<sf::Keyboard::Key>(property->getValue<sf::Int32>());
    }
    else
    {
        m_sharedStateData.inputBindings[1].keys[InputBinding::Shoot] = sf::Keyboard::RControl;
        binds->addProperty("shoot", std::to_string(sf::Keyboard::RControl));
        needsUpdate = true;
    }


    if (needsUpdate)
    {
        m_keyBinds.save(usrDir + fileName);
    }
}

void MenuState::createScene()
{    
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::AudioSystem>(mb);
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::QuadTree>(mb, sf::FloatRect(sf::Vector2f(), xy::DefaultSceneSize)); //actually this only needs to cover the bottom of the screen
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<SpringFlowerSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addDirector<TextboxDirector>(m_sharedStateData);
    m_scene.addDirector<MenuDirector>(m_textureResource);

    m_blurEffect = &m_scene.addPostProcess<xy::PostBlur>();
    m_blurEffect->setFadeSpeed(2.5f);

    xy::AudioMixer::setLabel("FX", 0);
    xy::AudioMixer::setLabel("Music", 1);    
    
    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/menu_background.png"));
    entity.addComponent<xy::Drawable>().setDepth(-10);
    entity.addComponent<xy::AudioEmitter>().setSource("assets/sound/music/menu.ogg");
    entity.getComponent<xy::AudioEmitter>().setChannel(1);
    entity.getComponent<xy::AudioEmitter>().setLooped(true);
    entity.getComponent<xy::AudioEmitter>().setVolume(0.25f);
    entity.getComponent<xy::AudioEmitter>().play();


    //grass at front
    entity = m_scene.createEntity();
    auto bounds = entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/grass.png")).getTextureBounds();
    bounds.width = xy::DefaultSceneSize.x;
    entity.addComponent<xy::Transform>().setPosition(0.f, xy::DefaultSceneSize.y - (bounds.height * 0.9f));
    entity.getComponent<xy::Sprite>().setTextureRect(bounds);
    m_textureResource.get("assets/images/grass.png").setRepeated(true);
    entity.addComponent<xy::Drawable>().setDepth(10);


    //springy grass
    float xPos = xy::Util::Random::value(80.f, 112.f);
    for (auto i = 0; i < 20; ++i)
    {
        entity = m_scene.createEntity();
        entity.addComponent<SpringFlower>(-64.f).headPos.x += xy::Util::Random::value(-8.f, 9.f);
        entity.getComponent<SpringFlower>().textureRect = { 20.f, 0.f, 36.f, 64.f };
        entity.getComponent<SpringFlower>().colour = { 160,170,160 };
        entity.getComponent<SpringFlower>().mass = 0.4f;
        entity.getComponent<SpringFlower>().stiffness = -28.f;
        entity.getComponent<SpringFlower>().damping = -0.4f;
        entity.addComponent<xy::Drawable>(m_textureResource.get("assets/images/grass.png"));
        entity.addComponent<xy::Transform>().setPosition(xPos, xy::DefaultSceneSize.y - xy::Util::Random::value(5.f, 11.f));

        xPos += xy::Util::Random::value(80.f, 112.f);
    }

    //and a couple of flowers
    xPos = xy::Util::Random::value(60.f, 90.f);
    sf::Uint8 darkness = 140;
    sf::Int32 depth = -3;
    for (auto i = 0; i < 12; ++i)
    {
        entity = m_scene.createEntity();
        entity.addComponent<SpringFlower>(-256.f).headPos.x += xy::Util::Random::value(-12.f, 14.f);
        entity.getComponent<SpringFlower>().textureRect = { 0.f, 0.f, 64.f, 256.f };
        entity.getComponent<SpringFlower>().colour = { darkness, darkness, darkness };
        entity.addComponent<xy::Drawable>(m_textureResource.get("assets/images/flower.png")).setDepth(depth);
        entity.addComponent<xy::Transform>().setPosition(xPos, xy::DefaultSceneSize.y + xy::Util::Random::value(0.f, 64.f));

        if ((i%4) == 0)
        {
            xPos += xy::Util::Random::value(400.f, 560.f);
            darkness = 160;
            depth = -3;
        }
        else
        {
            auto value = xy::Util::Random::value(20.f, 80.f);
            xPos += value;
            darkness = xy::Util::Math::clamp(sf::Uint8(darkness + static_cast<sf::Uint8>(value - 20.f)), sf::Uint8(140), sf::Uint8(255));
            depth+=2;
        }
    }

    //help sign
    entity = m_scene.createEntity();
    bounds = entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/help_sign.png")).getTextureBounds();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x - (bounds.width + 202.f), xy::DefaultSceneSize.y - bounds.height);
    entity.addComponent<xy::Drawable>().setDepth(9);
    entity.addComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([&mb](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            auto* msg = mb.post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::HelpButtonClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().area = bounds;


    m_scene.getActiveCamera().getComponent<xy::AudioListener>().setVolume(1.f);
}

void MenuState::createMenu()
{
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
    

    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");
    auto parentEnt = m_scene.createEntity();
    auto& tx = parentEnt.addComponent<xy::Transform>();
    parentEnt.addComponent<xy::Callback>().active = true;
    parentEnt.getComponent<xy::Callback>().function = MenuSliderCallback(m_menuTarget);

    createFirstMenu(tx, selectedID, unselectedID, font);
    createSecondMenu(tx, selectedID, unselectedID, font);
    createThirdMenu(tx, selectedID, unselectedID, font);

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}

void MenuState::createHelp()
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_helpScene.addSystem<xy::UISystem>(mb);
    m_helpScene.addSystem<xy::CallbackSystem>(mb);
    m_helpScene.addSystem<xy::SpriteSystem>(mb);
    m_helpScene.addSystem<xy::RenderSystem>(mb);
    m_helpScene.addSystem<xy::TextRenderer>(mb);

    //clicker
    auto entity = m_helpScene.createEntity();
    entity.addComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::MouseUp] =
        m_helpScene.getSystem<xy::UISystem>().addMouseButtonCallback([&mb](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            auto* msg = mb.post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::HelpButtonClicked;
        }
    });
    entity.getComponent<xy::UIHitBox>().area = { 0.f, 0.f, 64.f, 64.f };
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setPosition(1518.f, 20.f);
    /*entity.addComponent<xy::Text>(m_fontResource.get("assets/fonts/Cave-Story.ttf")).setString("X");
    entity.getComponent<xy::Text>().setCharacterSize(56);
    entity.getComponent<xy::Text>().setFillColour({ 200, 10, 220, 110 });*/


    //help menu
    entity = m_helpScene.createEntity();
    entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/help.png"));
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setPosition(0.f, HelpMenuCallback::HidePosition);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = HelpMenuCallback(m_helpShown, m_scene);

    //background
    entity = m_helpScene.createEntity();
    m_textureResource.setFallbackColour(sf::Color::White);
    auto size = entity.addComponent<xy::Sprite>(m_textureResource.get("help_background")).getSize();
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize.x / size.x, xy::DefaultSceneSize.y / size.y);
    entity.addComponent<xy::Drawable>().setDepth(-1);
    entity.getComponent<xy::Sprite>().setColour(sf::Color::Transparent);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = HelpBackgroundCallback(m_helpShown);

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_helpScene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}

void MenuState::showHelpMenu()
{
    XY_ASSERT(m_blurEffect, "Missing Post Process");
    
    if (m_helpShown)
    {
        //hide it
        m_blurEffect->setEnabled(false);
    }
    else
    {
        //show it
        m_blurEffect->setEnabled(true);
        m_scene.setSystemActive<xy::UISystem>(false);
    }
    
    m_helpShown = !m_helpShown;
}

void MenuState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingScreen.update(dt);
    rw.draw(m_loadingScreen);
}