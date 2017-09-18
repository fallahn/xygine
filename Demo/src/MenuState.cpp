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

#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>

#include <SFML/Window/Event.hpp>

MenuState::MenuState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData)
    : xy::State(stack, ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_sharedStateData   (sharedData)
{
    launchLoadingScreen();
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


    std::vector<sf::Vertex> buns =
    {
        sf::Vertex(sf::Vector2f(), sf::Color::Red), sf::Vertex(sf::Vector2f(100.f, 100.f), sf::Color::Red),
        sf::Vertex(sf::Vector2f(200.f, 100.f), sf::Color::Red), sf::Vertex(sf::Vector2f(100.f, 800.f), sf::Color::Red)
    };
    //rt.draw(buns.data(), buns.size(), sf::LineStrip, sf::Transform());

    rt.draw(m_scene);
}

//private
void MenuState::createMenu()
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addDirector<TextboxDirector>(m_sharedStateData);

    xy::AudioMixer::setLabel("Testy", 7);

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
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });

    auto& emitter = entity.addComponent<xy::ParticleEmitter>();
    emitter.settings.colour = sf::Color::White;
    emitter.settings.gravity = { -130.f, 1219.f };
    emitter.settings.initialVelocity = { 320.f, -530.f };
    emitter.settings.lifetime = 2.f;
    emitter.settings.emitRate = 20.f;
    emitter.settings.rotationSpeed = 12.f;
    emitter.start();

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
    bounds.width -= 48.f;
    entity.getComponent<xy::Text>().setCroppingArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MenuText;
    auto& tx3 = entity.addComponent<xy::Transform>();
    tx3.setPosition(24.f, 136.f);

    //join button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().addChild(tx3);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 64.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, -64.f);
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addCallback([this](xy::Entity entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Client;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}