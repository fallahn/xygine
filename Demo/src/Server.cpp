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
#include "ServerMessages.hpp"
#include "CollisionSystem.hpp"
#include "Hitbox.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <xyginext/core/FileSystem.hpp>

#include <tmxlite/Map.hpp>

#include <SFML/System/Clock.hpp>

#include <cstring>

#define CLIENT_MESSAGE(x) m_host.broadcastPacket(PacketID::ServerMessage, x, xy::NetFlag::Reliable, 1)

namespace
{
    const float tickRate = 1.f / 25.f;
    const float updateRate = 1.f / 60.f;
}

GameServer::GameServer()
    : m_ready       (false),
    m_running       (false),
    m_thread        (&GameServer::update, this),
    m_scene         (m_messageBus),
    m_currentMap    (0)
{
    m_clients[0].data.actor.type = ActorID::PlayerOne;
    m_clients[0].data.spawnX = 96.f;
    m_clients[0].data.spawnY = 864.f;

    m_clients[1].data.actor.type = ActorID::PlayerTwo;
    m_clients[1].data.spawnX = 928.f;
    m_clients[1].data.spawnY = 864.f;
}

GameServer::~GameServer()
{
    stop();
}

//public
void GameServer::start()
{
    //we can log these locally, as we'd be in the same thread
    xy::Logger::log("Starting local server", xy::Logger::Type::Info);
    
    m_mapFiles = xy::FileSystem::listFiles("assets/maps");
    if (m_mapFiles.empty())
    {
        xy::Logger::log("No maps found in map directory", xy::Logger::Type::Error);
        return;
    }

    m_running = true;
    m_thread.launch();
}

void GameServer::stop()
{
    //xy::Logger::log("Stopping server", xy::Logger::Type::Info);
    CLIENT_MESSAGE(MessageIdent::StopServer);

    m_running = false;
    m_thread.wait();
    m_ready = false;
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

            //send client reconciliation
            for (const auto& c : m_clients)
            {
                if (c.data.actor.id > -1)
                {
                    auto ent = m_scene.getEntity(c.data.actor.id);
                    const auto& tx = ent.getComponent<xy::Transform>().getPosition();
                    const auto& player = ent.getComponent<Player>();

                    ActorState state;
                    state.actor.id = c.data.actor.id;
                    state.actor.type = c.data.actor.type;
                    state.x = tx.x;
                    state.y = tx.y;
                    state.clientTime = player.history[player.lastUpdatedInput].timestamp;
                    state.playerState = player.state;
                    state.playerVelocity = player.velocity;

                    m_host.sendPacket(c.peer, PacketID::ClientUpdate, state, xy::NetFlag::Unreliable);
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
    const auto& peer = evt.peer;

    auto client = std::find_if(m_clients.begin(), m_clients.end(),
        [&peer](const Client& client)
    {
        return client.peer == peer;
    });

    if (client != m_clients.end())
    {
        //broadcast to clients
        m_host.broadcastPacket(PacketID::ClientDisconnected, client->data, xy::NetFlag::Reliable, 1);

        //remove from scene
        m_scene.destroyEntity(m_scene.getEntity(client->data.actor.id));

        //update the client array
        client->data.actor.id = -1;
        client->data.peerID = 0;
        client->peer = {};
    }
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
        if (m_clients[0].data.actor.id != ActorID::None)
        {
            playerNumber = 1;
            //send existing client data
            m_host.sendPacket(evt.peer, PacketID::ClientData, m_clients[0].data, xy::NetFlag::Reliable, 1);
        }
        //add the player actor to the scene
        spawnPlayer(playerNumber);

        //send the client info
        m_clients[playerNumber].data.peerID = evt.peer.getID();
        m_clients[playerNumber].peer = evt.peer;
        m_host.broadcastPacket(PacketID::ClientData, m_clients[playerNumber].data, xy::NetFlag::Reliable, 1);

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
    m_scene.addSystem<CollisionSystem>(m_messageBus);    
    m_scene.addSystem<ActorSystem>(m_messageBus);
    m_scene.addSystem<PlayerSystem>(m_messageBus);
    m_scene.addSystem<xy::CallbackSystem>(m_messageBus);
    m_scene.addSystem<xy::CommandSystem>(m_messageBus);
}

void GameServer::loadMap()
{
    tmx::Map map;
    if (map.load("assets/maps/" + m_mapFiles[m_currentMap]))
    {
        m_mapData.actorCount = MapData::MaxActors;
        std::strcpy(m_mapData.mapName, m_mapFiles[m_currentMap].c_str());

        //load collision geometry
        sf::Uint8 flags = 0;
        const auto& layers = map.getLayers();
        for (const auto& layer : layers)
        {
            if (layer->getType() == tmx::Layer::Type::Object)
            {
                //create map collision
                auto name = xy::Util::String::toLower(layer->getName());
                if (name == "platform")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        createCollisionObject(m_scene, obj, CollisionType::Platform);
                    }

                    flags |= MapFlags::Platform;
                }
                else if (name == "solid")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        createCollisionObject(m_scene, obj, CollisionType::Solid);
                    }
                    flags |= MapFlags::Solid;
                }
            }
        }
        if (flags != MapFlags::Server)
        {
            CLIENT_MESSAGE(MessageIdent::MapFailed);
            return;
        }

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
    else
    {
        //broadcast message ident that causes client to quit
        CLIENT_MESSAGE(MessageIdent::MapFailed);
    }
}

sf::Int32 GameServer::spawnPlayer(std::size_t player)
{
    auto entity = m_scene.createEntity();
    entity.addComponent<Actor>().type = (player == 0) ? ActorID::PlayerOne : ActorID::PlayerTwo;
    entity.getComponent<Actor>().id = entity.getIndex();
    m_clients[player].data.actor = entity.getComponent<Actor>();
    entity.addComponent<xy::Transform>().setPosition(m_clients[player].data.spawnX, m_clients[player].data.spawnY);
    entity.getComponent<xy::Transform>().setOrigin(PlayerSize / 2.f, PlayerSize);

    entity.addComponent<CollisionComponent>().addHitbox({ 0.f, 0.f, PlayerSize, PlayerSize }, CollisionType::Player);
    entity.getComponent<CollisionComponent>().addHitbox({ 0.f, PlayerSize, PlayerSize, 10.f }, CollisionType::Foot);

    //add client controller
    entity.addComponent<Player>().playerNumber = static_cast<sf::Uint8>(player);
    entity.addComponent<xy::CommandTarget>().ID = (player == 0) ? CommandID::PlayerOne : CommandID::PlayerTwo;

    return entity.getIndex();
}
