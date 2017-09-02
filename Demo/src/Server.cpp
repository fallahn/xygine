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

#include "Server.hpp"
#include "MapData.hpp"
#include "PacketIDs.hpp"
#include "ActorSystem.hpp"
#include "PlayerSystem.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/System/Clock.hpp>

#include <cstring>

namespace
{
    const float tickRate = 1.f / 25.f;
    const float updateRate = 1.f / 60.f;
}

GameServer::GameServer()
    : m_ready       (false),
    m_running       (false),
    m_thread        (&GameServer::update, this),
    m_scene         (m_messageBus)
{
    m_clients[0].actor.type = ActorID::PlayerOne;
    m_clients[0].spawnX = 20.f;
    m_clients[0].spawnY = 1016.f;

    m_clients[1].actor.type = ActorID::PlayerTwo;
    m_clients[1].spawnX = 920.f;
    m_clients[1].spawnY = 1016.f;
}

GameServer::~GameServer()
{
    stop();
}

//public
void GameServer::start()
{
    xy::Logger::log("Starting local server", xy::Logger::Type::Info);
    
    m_running = true;
    m_thread.launch();
}

void GameServer::stop()
{
    xy::Logger::log("Stopping server", xy::Logger::Type::Info);

    m_running = false;
    m_thread.wait();
}

//private
void GameServer::update()
{
    sf::Clock clock;
    float tickAccumulator = 0.f;
    float updateAccumulator = 0.f;

    initScene();
    loadMap();
    m_ready = m_host.start("", 40003, 2, 2);

    while (m_running)
    {
        const float dt = clock.restart().asSeconds();
        tickAccumulator += dt;
        updateAccumulator += dt;

        while (updateAccumulator > updateRate)
        {
            updateAccumulator -= updateRate;

            //player inputs are broadcast at 60fps (ish) so we need to try to keep up
            xy::NetEvent evt;
            while (m_host.pollEvent(evt))
            {
                switch (evt.type)
                {
                case xy::NetEvent::ClientConnect:
                    handleConnect(evt);
                    break;
                case xy::NetEvent::ClientDisconnect:
                    handleDisconnect(evt);
                    break;
                case xy::NetEvent::PacketReceived:
                    handlePacket(evt);
                    break;
                default: break;
                }
            }

            //update scene logic.
            m_scene.update(updateRate);
        }

        //network updates are less frequent than logic updates
        while (tickAccumulator > tickRate)
        {
            tickAccumulator -= tickRate;

            //broadcast scene state - TODO assemble this into one large packet rather than many small?
            const auto& actors = m_scene.getSystem<ActorSystem>().getActors();
            for (const auto& actor : actors)
            {
                const auto& actorComponent = actor.getComponent<Actor>();
                const auto& tx = actor.getComponent<xy::Transform>().getPosition();

                ActorState state;
                state.actor.id = actorComponent.id;
                state.actor.type = actorComponent.type;
                state.x = tx.x;
                state.y = tx.y;
                state.serverTime = m_serverTime.getElapsedTime().asMilliseconds();

                m_host.broadcastPacket(PacketID::ActorUpdate, state, xy::NetFlag::Unreliable);
            }

            //send client reconciliation - TODO ideally we only want to send this to the interested peer
            for (const auto& c : m_clients)
            {
                if (c.actor.id > -1)
                {
                    auto ent = m_scene.getEntity(c.actor.id);
                    const auto& tx = ent.getComponent<xy::Transform>().getPosition();
                    const auto& player = ent.getComponent<Player>();

                    ActorState state;
                    state.actor.id = c.actor.id;
                    state.actor.type = c.actor.type;
                    state.x = tx.x;
                    state.y = tx.y;
                    state.clientTime = player.history[player.lastUpdatedInput].timestamp;

                    m_host.broadcastPacket(PacketID::ClientUpdate, state, xy::NetFlag::Unreliable);
                }
            }
        }
    }

    m_host.stop();
}

void GameServer::handleConnect(const xy::NetEvent& evt)
{
    LOG("Client connected from " + evt.peer.getAddress(), xy::Logger::Type::Info);

    //TODO check not existing client

    //send map name, list of actor ID's up to count
    m_host.sendPacket(evt.peer, PacketID::MapData, m_mapData, xy::NetFlag::Reliable, 1);
}

void GameServer::handleDisconnect(const xy::NetEvent& evt)
{
    LOG("Client dropped from server", xy::Logger::Type::Info);

    //TODO broadcast to clients

    //TODO update the client array
}

void GameServer::handlePacket(const xy::NetEvent& evt)
{
    switch (evt.packet.getID())
    {
    default: break;
        //if client loaded send initial positions
    case PacketID::ClientReady:
    {
        std::size_t playerNumber = 0;
        if (m_clients[0].actor.id != ActorID::None)
        {
            playerNumber = 1;
            //send existing client data
            m_host.sendPacket(evt.peer, PacketID::ClientData, m_clients[0], xy::NetFlag::Reliable, 1);
        }
        //add the player actor to the scene
        spawnPlayer(playerNumber);

        //send the client info
        m_clients[playerNumber].peerID = evt.peer.getID();
        m_host.broadcastPacket(PacketID::ClientData, m_clients[playerNumber], xy::NetFlag::Reliable, 1);

        //send initial position of existing actors
        const auto& actors = m_scene.getSystem<ActorSystem>().getActors();
        for (const auto& actor : actors)
        {
            const auto& actorComponent = actor.getComponent<Actor>();
            const auto& tx = actor.getComponent<xy::Transform>().getPosition();

            ActorState state;
            state.actor.id = actorComponent.id;
            state.actor.type = actorComponent.type;
            state.x = tx.x;
            state.y = tx.y;

            m_host.sendPacket(evt.peer, PacketID::ActorAbsolute, state, xy::NetFlag::Reliable, 1);
        }
    }
        break;
    case PacketID::ClientInput:
    {        
        auto ip = evt.packet.as<InputUpdate>();
        xy::Command cmd;
        cmd.targetFlags = (ip.playerNumber == 0) ? CommandID::PlayerOne : CommandID::PlayerTwo;
        cmd.action = [ip](xy::Entity entity, float)
        {            
            auto& player = entity.getComponent<Player>();

            //update player input history
            player.history[player.currentInput].mask = ip.input;
            player.history[player.currentInput].timestamp = ip.clientTime;
            player.currentInput = (player.currentInput + 1) % player.history.size();
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    }
}

void GameServer::initScene()
{
    m_scene.addSystem<ActorSystem>(m_messageBus);
    m_scene.addSystem<PlayerSystem>(m_messageBus);
    m_scene.addSystem<xy::CallbackSystem>(m_messageBus);
    m_scene.addSystem<xy::CommandSystem>(m_messageBus);
}

void GameServer::loadMap()
{
    m_mapData.actorCount = MapData::MaxActors;
    std::strcpy(m_mapData.mapName, "Flaps");

    //do actor loading
    for (auto i = 0; i < m_mapData.actorCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(xy::Util::Random::value(0.f, xy::DefaultSceneSize.x), xy::Util::Random::value(0.f, xy::DefaultSceneSize.y));
        entity.addComponent<Actor>().id = entity.getIndex();
        entity.getComponent<Actor>().type = ActorID::BubbleOne;
        entity.addComponent<Velocity>().x = xy::Util::Random::value(-10.f, 10.f) * 50.f;
        entity.getComponent<Velocity>().y = xy::Util::Random::value(-10.f, 10.f) * 50.f;

        entity.addComponent<xy::Callback>().function = 
            [](xy::Entity e, float dt)
        {
            auto& tx = e.getComponent<xy::Transform>();
            auto& vel = e.getComponent<Velocity>();
            tx.move(vel.x * dt, vel.y * dt);
            
            auto pos = tx.getPosition();
            if (pos.x < 0)
            {
                pos.x = 0.f;
                auto newVel = xy::Util::Vector::reflect({ vel.x, vel.y }, { 1.f, 0.f });
                vel.x = newVel.x;
                vel.y = newVel.y;
            }
            else if (pos.x > xy::DefaultSceneSize.x)
            {
                pos.x = xy::DefaultSceneSize.x;
                auto newVel = xy::Util::Vector::reflect({ vel.x, vel.y }, { -1.f, 0.f });
                vel.x = newVel.x;
                vel.y = newVel.y;
            }
            else if (pos.y < 0)
            {
                pos.y = 0.f;
                auto newVel = xy::Util::Vector::reflect({ vel.x, vel.y }, { 0.f, 1.f });
                vel.x = newVel.x;
                vel.y = newVel.y;
            }
            else if (pos.y > xy::DefaultSceneSize.y)
            {
                pos.y = xy::DefaultSceneSize.y;
                auto newVel = xy::Util::Vector::reflect({ vel.x, vel.y }, { 0.f, -1.f });
                vel.x = newVel.x;
                vel.y = newVel.y;
            }
        };
        entity.getComponent<xy::Callback>().active = true;

        //update map data
        m_mapData.actors[i] = entity.getComponent<Actor>();
    }

    m_serverTime.restart();
}

sf::Int32 GameServer::spawnPlayer(std::size_t player)
{
    auto entity = m_scene.createEntity();
    entity.addComponent<Actor>().type = (player == 0) ? ActorID::PlayerOne : ActorID::PlayerTwo;
    entity.getComponent<Actor>().id = entity.getIndex();
    m_clients[player].actor = entity.getComponent<Actor>();
    entity.addComponent<xy::Transform>().setPosition(m_clients[player].spawnX, m_clients[player].spawnY);

    //add client controller
    entity.addComponent<Player>().playerNumber = static_cast<sf::Uint8>(player);
    entity.addComponent<xy::CommandTarget>().ID = (player == 0) ? CommandID::PlayerOne : CommandID::PlayerTwo;

    return entity.getIndex();
}