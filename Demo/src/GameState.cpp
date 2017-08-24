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
#include "PacketIDs.hpp"
#include "MapData.hpp"

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
    launchLoadingScreen();
    loadAssets();
    m_server.start();
    m_client.create(2);
    quitLoadingScreen();
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
            if (m_server.ready())
            {
                m_client.connect("localhost", 40003);
            }
            break;
        case sf::Keyboard::S:
            m_client.disconnect();
            break;
        case sf::Keyboard::D:
            
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
            handlePacket(evt);
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
    
    //preload textures
    m_textureResource.get("assets/images/bubble.png");
}

void GameState::handlePacket(const xy::NetEvent& evt)
{
    switch (evt.packet.getID())
    {
    default: break;
    case PacketID::ActorAbsolute:
        //set absolute state of actor
    {
        const auto& state = evt.packet.as<ActorState>();
        std::cout << "Actor at: " << state.x << ", " << state.y << std::endl;
    }
        break;
    case PacketID::ActorUpdate:
        //do actor interpolation
    {
        const auto& state = evt.packet.as<ActorState>();
        xy::App::printStat("Actor " + std::to_string(state.actor.id), std::to_string(state.x) + ", " + std::to_string(state.y));
    }
        break;
    case PacketID::MapData:
    {
        MapData data = evt.packet.as<MapData>();
        std::cout << data.mapName << ", " << (int)data.actorCount << std::endl;
    }
        //TODO load map (and clear old actors)
        //TODO load new actors

        //send ready signal
        m_client.sendPacket(PacketID::ClientReady, 0, xy::NetFlag::Reliable, 1);
        break;
    }
}