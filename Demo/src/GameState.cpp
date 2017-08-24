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

#include "GameState.hpp"

#include <xyginext/core/App.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>

#include <xyginext/network/NetData.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    struct TestPacket
    {
        int a = 2;
        float z = 54.f;
    };
}

GameState::GameState(xy::StateStack& stack, xy::State::Context ctx)
    : xy::State (stack, ctx),
    m_scene     (ctx.appInstance.getMessageBus())
{
    loadAssets();

    m_client.create(2);
}

//public
bool GameState::handleEvent(const sf::Event& evt)
{
    m_scene.forwardEvent(evt);

    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::A:
            m_client.connect("localhost", 40003);
            break;
        case sf::Keyboard::S:
            m_client.disconnect();
            break;
        case sf::Keyboard::D:
            m_client.sendPacket(3, TestPacket(), xy::NetFlag::Reliable);
            break;
        }
    }

    return false;
}

void GameState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool GameState::update(float dt)
{
    xy::NetEvent evt;
    while (m_client.pollEvent(evt))
    {
        if (evt.type == xy::NetEvent::PacketReceived)
        {
            std::cout << "Packet Received" << std::endl;
        }
    }
    
    m_scene.update(dt);
    return false;
}

void GameState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private
void GameState::loadAssets()
{
    auto& mb = getContext().appInstance.getMessageBus();

    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    
  
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.getComponent<xy::Transform>().setPosition(230.f, 0.f);
    entity.getComponent<xy::Transform>().setScale({ 0.2f, 0.2f });
    entity.getComponent<xy::Transform>().setRotation(122.f);
    entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/sphere_test.png"));

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(200.f, 410.f);
    entity.addComponent<xy::Text>(m_fontResource.get("assets/fonts/VeraMono.ttf")).setString("Bunnage!");
    entity.getComponent<xy::Text>().setCroppingArea({ 0.f, 0.f, 90.f, 24.f });
}