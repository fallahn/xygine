/*********************************************************************
(c) Jonny Paton 2018

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

#include "MyFirstState.hpp"
#include "States.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>

#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

MyFirstState::MyFirstState(xy::StateStack& ss, xy::State::Context ctx) :
xy::State(ss,ctx),
m_scene(ctx.appInstance.getMessageBus())
{
    createScene();
}

bool MyFirstState::handleEvent(const sf::Event& evt)
{
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    m_scene.forwardEvent(evt);
    return true;
}

void MyFirstState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool MyFirstState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void MyFirstState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_scene);
}

xy::StateID MyFirstState::stateID() const
{
    return States::MyFirstState;
}

void MyFirstState::createScene()
{
    //add the systems
    auto& messageBus = getContext().appInstance.getMessageBus();
    m_scene.addSystem<xy::TextSystem>(messageBus);
    m_scene.addSystem<xy::UISystem>(messageBus);
    m_scene.addSystem<xy::RenderSystem>(messageBus);

    m_font.loadFromFile("ProggyClean.ttf");

    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(200.f, 200.f);
    entity.addComponent<xy::Text>(m_font).setString("xygine tutorial");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.addComponent<xy::Drawable>();

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(200.f, 300.f);
    entity.addComponent<xy::Text>(m_font).setString("Play");
    entity.getComponent<xy::Text>().setCharacterSize(40);
    entity.addComponent<xy::Drawable>();

    entity.addComponent<xy::UIHitBox>().area = xy::Text::getLocalBounds(entity);

    auto callbackID = m_scene.getSystem<xy::UISystem>().addMouseButtonCallback(
        [&](xy::Entity, sf::Uint64 flags)
        {
            if(flags & xy::UISystem::LeftMouse)
            {
                requestStackClear();
                requestStackPush(States::GameState);
            }
        });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::MouseUp] = callbackID;

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(200.f, 360.f);
    entity.addComponent<xy::Text>(m_font).setString("Quit");
    entity.getComponent<xy::Text>().setCharacterSize(40);
    entity.addComponent<xy::Drawable>();

    entity.addComponent<xy::UIHitBox>().area = xy::Text::getLocalBounds(entity);

    callbackID = m_scene.getSystem<xy::UISystem>().addMouseButtonCallback(
        [&](xy::Entity, sf::Uint64 flags)
        {
            if(flags & xy::UISystem::LeftMouse)
            {
                getContext().appInstance.quit();
            }
        });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::MouseUp] = callbackID;

}









