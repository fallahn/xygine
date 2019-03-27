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

#include "PauseState.hpp"
#include "ResourceIDs.hpp"

#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Window/Event.hpp>

PauseState::PauseState(xy::StateStack& ss, xy::State::Context ctx, SharedStateData& sd)
    : xy::State (ss, ctx),
    m_scene     (ctx.appInstance.getMessageBus()),
    m_sharedData(sd)
{
    auto& mb = ctx.appInstance.getMessageBus();
    m_scene.addSystem<xy::TextSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);

    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.addComponent<xy::Drawable>().setDepth(-2); //makes sure it sits behind text
    auto& verts = entity.getComponent<xy::Drawable>().getVertices();
    verts =
    {
        sf::Vertex(sf::Vector2f(0.f, 0.f), sf::Color(0,0,0,120)),
        sf::Vertex(sf::Vector2f(xy::DefaultSceneSize.x, 0.f), sf::Color(0,0,0,120)),
        sf::Vertex(xy::DefaultSceneSize, sf::Color(0,0,0,120)),
        sf::Vertex(sf::Vector2f(0.f, xy::DefaultSceneSize.y), sf::Color(0,0,0,120)),
    };
    entity.getComponent<xy::Drawable>().updateLocalBounds(); // this is required whenever vertex data is modified


    auto& font = m_sharedData.resources.get<sf::Font>(FontID::handles[FontID::MenuFont]);

    //text entities
    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.addComponent<xy::Text>(font).setString("PAUSED");
    entity.getComponent<xy::Text>().setCharacterSize(180);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>();

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 208.f);
    entity.addComponent<xy::Text>(font).setString("Press Q to Quit or P to Continue");
    entity.getComponent<xy::Text>().setCharacterSize(40);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>();

    m_scene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_scene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

    ctx.appInstance.setMouseCursorVisible(true);
}

//public
bool PauseState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::P:
        case sf::Keyboard::Escape:
        case sf::Keyboard::Pause:
            requestStackPop();
            break;
        case sf::Keyboard::Q:
            requestStackClear();
            requestStackPush(States::MyFirstState);
            break;
        }
    }

    m_scene.forwardEvent(evt);
    return false; //this is a pause state - don't forward events to states below!
}

void PauseState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool PauseState::update(float dt)
{
    m_scene.update(dt);
    return false;
}

void PauseState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}