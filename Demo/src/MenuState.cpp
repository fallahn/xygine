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

#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/AudioListener.hpp>

#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>

#include <SFML/Window/Event.hpp>

#include <array>


MenuState::MenuState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData)
    : xy::State(stack, ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_sharedStateData   (sharedData)
{
    launchLoadingScreen();
    createScene();
    createMenu();
    ctx.appInstance.setClearColour({ 1, 0, 10 });
    quitLoadingScreen();
}

//public
bool MenuState::handleEvent(const sf::Event& evt)
{
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    m_scene.forwardEvent(evt);
    return true;
}

void MenuState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool MenuState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void MenuState::draw()
{
    auto& rt = getContext().renderWindow;

    rt.draw(m_scene);
}

//private
void MenuState::createScene()
{    
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::AudioSystem>(mb);
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addDirector<TextboxDirector>(m_sharedStateData);
    m_scene.addDirector<MenuDirector>(m_textureResource);

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
    entity.addComponent<xy::Transform>().setPosition(0.f, xy::DefaultSceneSize.y - bounds.height);
    entity.getComponent<xy::Sprite>().setTextureRect(bounds);
    m_textureResource.get("assets/images/grass.png").setRepeated(true);
    entity.addComponent<xy::Drawable>().setDepth(10);


    m_scene.getActiveCamera().getComponent<xy::AudioListener>().setVolume(1.f);
}

void MenuState::createMenu()
{
    //host text
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString("HOST");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(52.f, 45.f);

    //host button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -128.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] = 
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });

    //join text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString("JOIN");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(54.f, 45.f);

    //ip text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(m_sharedStateData.remoteIP);
    entity.getComponent<xy::Text>().setCharacterSize(65);
    bounds.width -= 72.f;
    entity.getComponent<xy::Text>().setCroppingArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MenuText;
    auto& tx3 = entity.addComponent<xy::Transform>();
    tx3.setPosition(44.f, 146.f);

    //join button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().addChild(tx3);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 64.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, -64.f);
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Client;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });


    //quit text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString("QUIT");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(136.f, 15.f);

    //quit button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx4);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 256.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            requestStackClear();
            xy::App::quit();
        }
    });

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}