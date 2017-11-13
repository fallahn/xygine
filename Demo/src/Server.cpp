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
#include "AnimationController.hpp"
#include "CommandIDs.hpp"
#include "ServerMessages.hpp"
#include "CollisionSystem.hpp"
#include "Hitbox.hpp"
#include "ClientServerShared.hpp"
#include "sha1.hpp"
#include "BubbleSystem.hpp"
#include "NPCSystem.hpp"
#include "FruitSystem.hpp"
#include "MessageIDs.hpp"
#include "InventoryDirector.hpp"
#include "PowerupSystem.hpp"
#include "BonusSystem.hpp"
#include "HatSystem.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>

#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <xyginext/core/FileSystem.hpp>

#include <tmxlite/Map.hpp>

#include <SFML/System/Clock.hpp>

#include <cstring>
#include <fstream>

#define CLIENT_MESSAGE(x) m_host.broadcastPacket(PacketID::ServerMessage, x, xy::NetFlag::Reliable, 1)

namespace
{
    const float tickRate = 1.f / 25.f;
    const float updateRate = 1.f / 60.f;
    const float endOfRoundTime = 6.f;
    const float defaultRoundTime = 39.f; //after this everything is angry
    const float roundWarnTime = 2.5f; //allows time for clients to do warning
    const float watchdogTime = 20.f; //change map this many seconds after round time regardless
    const float MaxPauseTime = 5.f * 60.f;
}

std::bitset<4> GameServer::GameOverOrPaused = { (1 << GameOver) | (1 << Paused) }; //should be constexpr but makes g++ cry

GameServer::GameServer()
    : m_ready               (false),
    m_running               (false),
    m_thread                (&GameServer::update, this),
    m_scene                 (m_messageBus),
    m_mapSkipCount          (0),
    m_currentMap            (0),
    m_endOfRoundPauseTime   (3.f),
    m_currentRoundTime      (0.f),
    m_roundTimeout          (defaultRoundTime)
{
    m_clients[0].data.actor.type = ActorID::PlayerOne;
    m_clients[0].data.spawnX = PlayerOneSpawn.x;
    m_clients[0].data.spawnY = PlayerOneSpawn.y;
    m_clients[0].data.playerNumber = 0;

    m_clients[1].data.actor.type = ActorID::PlayerTwo;
    m_clients[1].data.spawnX = PlayerTwoSpawn.x;
    m_clients[1].data.spawnY = PlayerTwoSpawn.y;
    m_clients[1].data.playerNumber = 1;
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
    
    initMaplist();
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
    float pauseTimeout = 0.f;

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
            while (!m_messageBus.empty())
            {
                auto msg = m_messageBus.poll();
                handleMessage(msg);
                m_scene.forwardMessage(msg);
            }

            //only update the server if clients are connected
            //and not paused
            if ((m_host.getConnectedPeerCount() > 0 || m_stateFlags.test(ChangingMaps))
                && /*!m_stateFlags.test(GameOver) && !m_stateFlags.test(Paused)*/((m_stateFlags & GameOverOrPaused) == 0))
            {
                m_scene.update(updateRate);

                //check if it's time to make everything angry
                checkRoundTime(updateRate);

                //check if it's time to change map
                checkMapStatus(updateRate);

                //check to see if client requested pause
                if (!m_stateFlags.test(ChangingMaps) && m_stateFlags.test(PendingPause))
                {
                    m_stateFlags.set(Paused, m_stateFlags.test(PendingPause));
                    m_stateFlags.set(PendingPause, false);
                    pauseTimeout = 0.f;

                    //broadcast paused to clients
                    m_host.broadcastPacket(PacketID::RequestClientPause, sf::Uint8(0), xy::NetFlag::Reliable, 1);
                }               
            }
            //and unpause if client is idling
            pauseTimeout += dt;
            if (pauseTimeout > MaxPauseTime)
            {
                m_stateFlags.set(Paused, false);
                m_host.broadcastPacket(PacketID::RequestClientPause, sf::Uint8(1), xy::NetFlag::Reliable, 1);
            }
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
                const auto& anim = actor.getComponent<AnimationController>();

                ActorState state;
                state.actor.id = actorComponent.id;
                state.actor.type = actorComponent.type;
                state.x = tx.x;
                state.y = tx.y;
                state.serverTime = m_serverTime.getElapsedTime().asMilliseconds();
                state.animationDirection = anim.direction;
                state.animationID = anim.nextAnimation;

                m_host.broadcastPacket(PacketID::ActorUpdate, state, xy::NetFlag::Unreliable);
            }

            //check if all players are dead
            bool gameOver = m_clients[0].ready || m_clients[1].ready;

            //send client reconciliation
            for (const auto& c : m_clients)
            {
                if (c.data.actor.id > -1)
                {
                    auto ent = m_scene.getEntity(c.data.actor.id);
                    const auto& tx = ent.getComponent<xy::Transform>().getPosition();
                    const auto& player = ent.getComponent<Player>();

                    ClientState state;
                    state.actor.id = c.data.actor.id;
                    state.actor.type = c.data.actor.type;
                    state.x = tx.x;
                    state.y = tx.y;
                    state.clientTime = player.history[player.lastUpdatedInput].input.timestamp;
                    state.playerState = player.state;
                    state.playerVelocity = player.velocity.y;
                    state.playerTimer = player.timer;
                    state.playerCanLand = player.canLand;
                    state.playerLives = player.lives;
                    if (player.canJump) state.boolFlags |= ClientState::JumpFlag;
                    if (player.canRideBubble) state.boolFlags |= ClientState::BubbleFlag;
                    if (player.hasHat)state.boolFlags |= ClientState::HatFlag;


                    //auto collisionState = ent.getComponent<CollisionComponent>().serialise();
                    //std::vector<sf::Uint8> data(sizeof(ClientState) + collisionState.size()); //TODO recycle this to save on reallocations
                    //auto ptr = data.data();
                    //std::memcpy(ptr, &state, sizeof(state));
                    //ptr += sizeof(state);
                    //std::memcpy(ptr, collisionState.data(), collisionState.size());

                    m_host.sendPacket(c.peer, PacketID::ClientUpdate, state, xy::NetFlag::Unreliable);
                    //m_host.sendPacket(c.peer, PacketID::ClientUpdate, data.data(), data.size(), xy::NetFlag::Unreliable);

                    gameOver = (gameOver && player.state == Player::State::Dead);
                }
            }

            if (gameOver && !m_stateFlags.test(GameOver))
            {
                //state changed, send message
                m_host.broadcastPacket(PacketID::GameOver, 0, xy::NetFlag::Reliable, 1);
            }
            m_stateFlags.set(GameOver, gameOver);

#ifdef XY_DEBUG
            m_host.broadcastPacket(PacketID::DebugMapCount, m_mapData.NPCCount, xy::NetFlag::Unreliable, 0);
#endif
        }
    }

    //cleanly disconnect any clients
    m_host.stop();
}

void GameServer::handleConnect(const xy::NetEvent& evt)
{
    LOG("Client connected from " + evt.peer.getAddress(), xy::Logger::Type::Info);

    //if already hosting 2 players on same client, send rejection
    if (m_clients[0].data.actor.id != ActorID::None
        && m_clients[1].data.actor.id != ActorID::None)
    {
        m_host.sendPacket(evt.peer, PacketID::ServerFull, sf::Uint8(0), xy::NetFlag::Reliable, 1);
        return;
    }

    if (!m_stateFlags.test(ChangingMaps))
    {
        //send map name, list of actor ID's up to count
        m_host.sendPacket(evt.peer, PacketID::MapJoin, m_mapData, xy::NetFlag::Reliable, 1);
    }
    else
    {
        //queue until change complete
        m_queuedClient = std::make_unique<xy::NetPeer>(evt.peer);
    }
}

void GameServer::handleDisconnect(const xy::NetEvent& evt)
{
    //LOG("Client dropped from server", xy::Logger::Type::Info);
    CLIENT_MESSAGE(MessageIdent::ClientDropped);
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
        auto entity = m_scene.getEntity(client->data.actor.id);
        
        auto* msg = m_messageBus.post<NetworkEvent>(MessageID::NetworkMessage);
        msg->type = NetworkEvent::Disconnected;
        msg->playerID = entity.getComponent<Player>().playerNumber;

        m_scene.destroyEntity(entity);

        //update the client array
        client->data.actor.id = ActorID::None;
        client->data.peerID = 0;
        client->peer = {};
        client->ready = false;
        client->level = 1;

        //unpause the game if it is paused so remaining player can continue
        if (m_stateFlags.test(Paused))
        {
            m_stateFlags.set(Paused, false);
            m_host.broadcastPacket(PacketID::RequestClientPause, sf::Uint8(1), xy::NetFlag::Reliable, 1);
        }
    }
}

void GameServer::handlePacket(const xy::NetEvent& evt)
{
    switch (evt.packet.getID())
    {
    default: break;
    case PacketID::RequestServerPause:
    {
        auto pause = evt.packet.as<sf::Uint8>();
        if (pause == 0 && !m_stateFlags.test(Paused))
        {
            m_stateFlags.set(PendingPause, true);
        }
        else
        {
            if (m_stateFlags.test(Paused))
            {
                m_stateFlags.set(Paused, false);
                m_host.broadcastPacket(PacketID::RequestClientPause, sf::Uint8(1), xy::NetFlag::Reliable, 1);
            }
        }
    }
        break;
        //if client loaded send initial positions
    case PacketID::ClientReady:
    {
        sf::Uint8 playerCount = evt.packet.as<sf::Uint8>();

        for (auto i = 0; i < playerCount; ++i)
        {
            std::size_t playerNumber = 0;
            if (m_clients[0].data.actor.id != ActorID::None)
            {
                playerNumber = 1;

                if (playerCount == 1)
                {
                    //send existing client data
                    m_host.sendPacket(evt.peer, PacketID::ClientData, m_clients[0].data, xy::NetFlag::Reliable, 1);
                }
            }
            //add the player actor to the scene
            spawnPlayer(playerNumber);

            //send the client info
            m_clients[playerNumber].data.peerID = evt.peer.getID();
            m_clients[playerNumber].peer = evt.peer;
            m_clients[playerNumber].ready = true;
            m_clients[playerNumber].level = 1;
            m_host.broadcastPacket(PacketID::ClientData, m_clients[playerNumber].data, xy::NetFlag::Reliable, 1);
        }
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

        m_currentRoundTime = 0.f;
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
            player.history[player.currentInput].input.mask = ip.input;
            player.history[player.currentInput].input.timestamp = ip.clientTime;
            player.currentInput = (player.currentInput + 1) % player.history.size();
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::ClientContinue:
    {
        auto id = evt.packet.as<sf::Int16>();
        auto entity = m_scene.getEntity(id);

        //reset the player info
        if (entity.getComponent<Actor>().id == id)
        {
            auto& player = entity.getComponent<Player>();
            if (player.state == Player::State::Dead)
            {
                entity.getComponent<xy::Transform>().setPosition(player.spawnPosition);
                player.state = Player::State::Walking;
                player.direction =
                    (player.spawnPosition.x < (MapBounds.width / 2.f)) ? Player::Direction::Right : Player::Direction::Left;

                player.timer = PlayerInvincibleTime;
                player.lives = PlayerStartLives;
                player.bonusFlags = 0;

                auto* msg = m_messageBus.post<GameEvent>(MessageID::GameMessage);
                msg->action = GameEvent::Restarted;
                msg->playerID = player.playerNumber;

                m_clients[player.playerNumber].level = 1;
                m_host.sendPacket(m_clients[player.playerNumber].peer, PacketID::LevelUpdate, m_clients[player.playerNumber].level, xy::NetFlag::Reliable, 1);
            }
        }
    }
        break;
    case PacketID::MapReady:
    {
        sf::Int16 actor = evt.packet.as<sf::Int16>();
        if (actor == ActorID::PlayerOne)
        {
            m_clients[0].ready = true;
        }
        else if (actor == ActorID::PlayerTwo)
        {
            m_clients[1].ready = true;
        }
        beginNewRound();

    }
        break;
    }
}

void GameServer::checkRoundTime(float dt)
{
    if (m_stateFlags.test(ChangingMaps) || m_mapData.NPCCount == 0) return;
    
    float lastTime = m_currentRoundTime;
    m_currentRoundTime += dt;

    if (lastTime < (m_roundTimeout - roundWarnTime)
        && m_currentRoundTime >= (m_roundTimeout - roundWarnTime))
    {
        //send warning message
        m_host.broadcastPacket(PacketID::RoundWarning, sf::Uint8(0), xy::NetFlag::Reliable, 1);
    }
    else if (lastTime < m_roundTimeout &&
        m_currentRoundTime >= m_roundTimeout && m_mapData.NPCCount > 0)
    {
        //make everything angry
        xy::Command cmd;
        cmd.targetFlags = CommandID::NPC;
        cmd.action = [&](xy::Entity entity, float)
        {
            entity.getComponent<NPC>().angry = true;
            //broadcast to clients
            ActorEvent evt;
            evt.type = ActorEvent::GotAngry;
            evt.actor = entity.getComponent<Actor>();
            m_host.broadcastPacket(PacketID::ActorEvent, evt, xy::NetFlag::Reliable, 1);
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        //spawn some gooblies
        static const float offset = 200.f;
        if (m_clients[0].data.actor.type != ActorID::None)
        {
            xy::Command cmd;
            cmd.targetFlags = CommandID::PlayerOne;
            cmd.action = [&](xy::Entity entity, float)
            {
                if (entity.getComponent<Player>().state != Player::State::Dead)
                {
                    //spawn and set player as target
                    auto ent = spawnNPC(ActorID::Goobly, { MapBounds.left + offset, MapBounds.top + offset });
                    ent.getComponent<NPC>().target = entity;
                }
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        }

        if (m_clients[1].data.actor.type != ActorID::None)
        {
            xy::Command cmd;
            cmd.targetFlags = CommandID::PlayerTwo;
            cmd.action = [&](xy::Entity entity, float)
            {
                if (entity.getComponent<Player>().state != Player::State::Dead)
                {
                    //spawn and set player as target
                    auto ent = spawnNPC(ActorID::Goobly, { MapBounds.width - offset, MapBounds.top + offset });
                    ent.getComponent<NPC>().target = entity;
                }
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
        
    }

    //sometimes the actor count gets messed up so we have a fail-safe timeout
    if (m_currentRoundTime > (m_roundTimeout + watchdogTime)
        && m_mapData.NPCCount <= 1)
    {
        m_mapData.NPCCount = 0;
    }
}

void GameServer::checkMapStatus(float dt)
{
    m_endOfRoundPauseTime -= dt;

    if (m_mapData.NPCCount == 0
        && m_endOfRoundPauseTime < 0)
    {
        //clear remaining actors (should just be collectables / bubbles)
        //as well as any geometry
        xy::Command cmd;
        cmd.targetFlags = CommandID::MapItem;
        cmd.action = [&](xy::Entity entity, float)
        {
            m_scene.destroyEntity(entity);
            //std::cout << "delete server geometry" << std::endl;
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        m_scene.update(0.f); //force the command right away


        //load next map
        m_currentMap = (m_currentMap + 1) % m_mapFiles.size();
        loadMap(); //TODO we need to check this was successful
        

        //tell clients to do map change
        //m_host.broadcastPacket(PacketID::MapChange, m_mapData, xy::NetFlag::Reliable, 1);

        //set clients to not ready
        m_clients[0].ready = false;
        m_clients[1].ready = false;
        m_stateFlags.set(ChangingMaps, true);

        //move players to spawn point ready for next map
        cmd.targetFlags = CommandID::PlayerOne | CommandID::PlayerTwo;
        cmd.action = [&](xy::Entity entity, float)
        {
            if (entity.getComponent<xy::CommandTarget>().ID & CommandID::PlayerOne)
            {
                entity.getComponent<xy::Transform>().setPosition(PlayerOneSpawn);
            }
            else
            {
                entity.getComponent<xy::Transform>().setPosition(PlayerTwoSpawn);
            }
            entity.getComponent<Player>().state = Player::State::Disabled;
            entity.getComponent<Player>().velocity.y = 0.f;
            if (entity.getComponent<Player>().hasHat)
            {
                auto* msg = m_messageBus.post<PlayerEvent>(MessageID::PlayerMessage);
                msg->type = PlayerEvent::LostHat;
                msg->entity = entity;

                entity.getComponent<Player>().hasHat = false;
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        m_scene.setSystemActive<NPCSystem>(false);
        m_scene.setSystemActive<CollisionSystem>(false);
        m_scene.setSystemActive<PowerupSystem>(false);
        m_scene.setSystemActive<BonusSystem>(false);
        m_scene.setSystemActive<HatSystem>(false);

        auto* msg = m_messageBus.post<MapEvent>(MessageID::MapMessage);
        msg->type = MapEvent::MapChangeStarted;

        //increase the level count for connected clients and send update
        auto updateLevel = [&](Client& client)
        {
            if (client.data.actor.type != ActorID::None)
            {
                client.level++;
                if (client.level > MapsToWin)
                {
                    //game completed tell client
                    m_host.sendPacket(client.peer, PacketID::GameComplete, sf::Uint8(0), xy::NetFlag::Reliable, 1);
                }
                else
                {
                    //send value update
                    m_host.sendPacket(client.peer, PacketID::LevelUpdate, client.level, xy::NetFlag::Reliable, 1);
                    //and inform of next map
                    m_host.sendPacket(client.peer, PacketID::MapChange, m_mapData, xy::NetFlag::Reliable, 1);
                }
            }
        };
        updateLevel(m_clients[0]);
        updateLevel(m_clients[1]);
    }
}

void GameServer::initMaplist()
{
    auto mapFiles = xy::FileSystem::listFiles("assets/maps");

    //if no map cycle list create it
    if (!xy::FileSystem::fileExists("assets/maps/mapcycle.txt"))
    {
        //TODO should really be using user data directory
        std::ofstream file("assets/maps/mapcycle.txt");
        if (file.good())
        {
            for (const auto& map : mapFiles)
            {
                file << map << std::endl;
            }
            file.close();
        }
    }

    static const std::size_t maxMaps = 255;
    std::ifstream file("assets/maps/mapcycle.txt");
    if (file.good())
    {
        std::string line;
        while (!file.eof() && m_mapFiles.size() < maxMaps)
        {
            std::getline(file, line);
            m_mapFiles.push_back(line);
        }
    }

    //remove from list if file doesn't exist
    m_mapFiles.erase(std::remove_if(m_mapFiles.begin(), m_mapFiles.end(),
        [&mapFiles](const std::string& str) 
    {
        return std::find(mapFiles.begin(), mapFiles.end(), str) == mapFiles.end();
    }), m_mapFiles.end());

    //m_currentMap = xy::Util::Random::value(0, std::min(5, static_cast<int>(m_mapFiles.size())));
}

void GameServer::initScene()
{
    m_scene.addSystem<xy::QuadTree>(m_messageBus, MapBounds);
    m_scene.addSystem<CollisionSystem>(m_messageBus, true);    
    m_scene.addSystem<ActorSystem>(m_messageBus);
    m_scene.addSystem<BubbleSystem>(m_messageBus, m_host);
    m_scene.addSystem<NPCSystem>(m_messageBus, m_host);
    m_scene.addSystem<FruitSystem>(m_messageBus, m_host);
    m_scene.addSystem<PowerupSystem>(m_messageBus, m_host);
    m_scene.addSystem<BonusSystem>(m_messageBus, m_host);
    m_scene.addSystem<HatSystem>(m_messageBus, m_host);
    m_scene.addSystem<PlayerSystem>(m_messageBus, true);
    //m_scene.addSystem<xy::CallbackSystem>(m_messageBus);
    m_scene.addSystem<xy::CommandSystem>(m_messageBus);

    m_scene.addDirector<InventoryDirector>(m_host);

    m_scene.setSystemActive<HatSystem>(false); //no hats on first level plz
}

void GameServer::loadMap()
{
    m_mapData.NPCCount = 0; //make sure count was reset
    m_mapData.crateCount = 0;

    tmx::Map map;
    if (map.load("assets/maps/" + m_mapFiles[m_currentMap]))
    {
        auto sha1 = getSha("assets/maps/" + m_mapFiles[m_currentMap]);

        std::strcpy(m_mapData.mapName, m_mapFiles[m_currentMap].c_str());
        std::strcpy(m_mapData.mapSha, sha1.c_str());

        m_scene.getSystem<PowerupSystem>().setSpawnFlags(0);

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
                        flags |= MapFlags::Platform;
                    }
                }
                else if (name == "solid")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        createCollisionObject(m_scene, obj, CollisionType::Solid);
                        flags |= MapFlags::Solid;
                    }                   
                }
                else if (name == "teleport")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        createCollisionObject(m_scene, obj, CollisionType::Teleport);
                    }

                    //only enable powerups if we have 4 spawn points
                    if (objs.size() == 4)
                    {
                        m_scene.getSystem<PowerupSystem>().setSpawnFlags(PowerupSystem::Flame | PowerupSystem::Lightning);
                        if(xy::Util::Random::value(0, 1) == 0) m_scene.getSystem<BonusSystem>().setEnabled(true);
                    }
                    else
                    {
                        m_scene.getSystem<BonusSystem>().setEnabled(false);
                    }

                    flags |= MapFlags::Teleport;
                }
                else if (name == "spawn")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        auto actor = ActorID::None;
                        auto objName = xy::Util::String::toLower(obj.getName());
                        if (objName == "whirlybob")
                        {
                            actor = ActorID::Whirlybob;
                        }
                        else if (objName == "clocksy")
                        {
                            actor = ActorID::Clocksy;
                        }
                        else if (objName == "squatmo")
                        {
                            actor = ActorID::Squatmo;
                        }
                        else if (objName == "balldock")
                        {
                            actor = ActorID::Balldock;
                        }

                        if (actor != ActorID::None)
                        {
                            auto entity = spawnNPC(actor, { obj.getPosition().x, obj.getPosition().y });
                            m_mapData.NPCs[m_mapData.NPCCount++] = entity.getComponent<Actor>();
                        }
                    }
                    //spawnNPC(ActorID::Clocksy, { 220.f, 220.f }); spawnCount++;
                
                    flags |= (m_mapData.NPCCount == 0) ? 0 : MapFlags::Spawn;
                }
                else if (name == "crates")
                {
                    const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
                    for (const auto& obj : objs)
                    {
                        if (xy::Util::String::toLower(obj.getName()) == "crate")
                        {
                            bool explosive = false;
                            const auto& properties = obj.getProperties();
                            if (!properties.empty() && 
                                xy::Util::String::toLower(properties[0].getName()) == "explosive")
                            {
                                explosive = properties[0].getBoolValue();
                            }

                            //TODO create ent / actor with explosive parameter
                        }
                    }
                }
            }
        }
        if (flags != MapFlags::Server)
        {
            CLIENT_MESSAGE(MessageIdent::MapFailed);
            //std::cout << m_mapFiles[m_currentMap] << ", Bad flags! " << std::bitset<8>(flags) << std::endl;
            return;
        }

        //create hard edges - we could probably only do this once
        //as they don't change, but let's stick with the flow
        std::array<sf::FloatRect, 2u> bounds = 
        {
            sf::FloatRect(0.f, 0.f, 64.f, MapBounds.height),
            sf::FloatRect(MapBounds.width - 64.f, 0.f, 64.f, MapBounds.height)
        };
        for (auto rect : bounds)
        {
            auto entity = m_scene.createEntity();
            entity.addComponent<xy::Transform>().setPosition(rect.left, rect.top);
            entity.addComponent<CollisionComponent>().addHitbox({ 0.f, 0.f, rect.width, rect.height }, CollisionType::HardBounds);
            entity.addComponent<xy::QuadTreeItem>().setArea({ 0.f, 0.f, rect.width, rect.height });
            entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem;
            entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::HardBounds);
            entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Bubble | CollisionFlags::MagicHat);
        }

        //check if map has a round time associated with it
        const auto& properties = map.getProperties();
        auto result = std::find_if(properties.begin(), properties.end(),
            [](const tmx::Property& property)
        {
            return xy::Util::String::toLower(property.getName()) == "round_time";
        });

        if (result != properties.end())
        {
            m_roundTimeout = result->getFloatValue();
        }
        else
        {
            m_roundTimeout = defaultRoundTime;
        }

        m_serverTime.restart();
    }
    else
    {
        //broadcast message ident that causes client to quit
        CLIENT_MESSAGE(MessageIdent::MapFailed);
        //std::cout << "failed opening next map" << std::endl;
    }
}

void GameServer::beginNewRound()
{
    //check if all connected clients are ready then send message to continue
    bool p1Ready = ((m_clients[0].data.actor.id == ActorID::None) || (m_clients[0].data.actor.id != ActorID::None && m_clients[0].ready));
    bool p2Ready = ((m_clients[1].data.actor.id == ActorID::None) || (m_clients[1].data.actor.id != ActorID::None && m_clients[1].ready));

    if (p1Ready && p2Ready)
    {
        m_scene.setSystemActive<NPCSystem>(true);
        m_scene.setSystemActive<CollisionSystem>(true);
        m_scene.setSystemActive<PowerupSystem>(true);
        m_scene.setSystemActive<BonusSystem>(true);
        
        if (xy::Util::Random::value(0, 2) == 2)
        {
            m_scene.setSystemActive<HatSystem>(true);
        }

        m_stateFlags.set(ChangingMaps, false);

        if (m_mapSkipCount) //someone scored a bonus and skips 5 maps
        {
            m_mapData.NPCCount = 0;
            m_endOfRoundPauseTime = -1.f;
            m_mapSkipCount--;
        }
        else
        {
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

                m_host.broadcastPacket(PacketID::ActorAbsolute, state, xy::NetFlag::Reliable, 1);
            }

            m_currentRoundTime = 0.f;

            xy::Command cmd;
            cmd.targetFlags = CommandID::PlayerOne | CommandID::PlayerTwo;
            cmd.action = [](xy::Entity entity, float)
            {
                if (entity.getComponent<Player>().lives > 0)
                {
                    entity.getComponent<Player>().state = Player::State::Walking;
                }
                else //we have to set this else player remains 'disabled'
                {
                    entity.getComponent<Player>().state = Player::State::Dead;
                }
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

            //check if anyone tried joining mid map change
            if (m_queuedClient)
            {
                if (m_clients[0].data.actor.id != ActorID::None
                    && m_clients[1].data.actor.id != ActorID::None)
                {
                    m_host.sendPacket(*m_queuedClient, PacketID::ServerFull, sf::Uint8(0), xy::NetFlag::Reliable, 1);
                }
                else
                {
                    m_host.sendPacket(*m_queuedClient, PacketID::MapJoin, m_mapData, xy::NetFlag::Reliable, 1);
                }
                m_queuedClient.reset();
            }

            //raise message to say new map started
            auto* msg = m_messageBus.post<MapEvent>(MessageID::MapMessage);
            msg->type = MapEvent::MapChangeComplete;
        }
    }
}

sf::Int32 GameServer::spawnPlayer(std::size_t player)
{
    auto entity = m_scene.createEntity();
    entity.addComponent<Actor>().type = (player == 0) ? ActorID::PlayerOne : ActorID::PlayerTwo;
    entity.getComponent<Actor>().id = entity.getIndex();
    m_clients[player].data.actor = entity.getComponent<Actor>();
    entity.addComponent<xy::Transform>().setPosition(m_clients[player].data.spawnX, m_clients[player].data.spawnY);
    entity.getComponent<xy::Transform>().setOrigin(PlayerOrigin);

    entity.addComponent<CollisionComponent>().addHitbox(PlayerBounds, CollisionType::Player);
    entity.getComponent<CollisionComponent>().addHitbox(PlayerFoot, CollisionType::Foot);
    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Player);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PlayerMask);
    entity.addComponent<xy::QuadTreeItem>().setArea(entity.getComponent<CollisionComponent>().getLocalBounds());

    entity.addComponent<AnimationController>();

    //add client controller
    entity.addComponent<Player>().playerNumber = static_cast<sf::Uint8>(player);
    entity.getComponent<Player>().spawnPosition = entity.getComponent<xy::Transform>().getPosition();
    if (player == 1) entity.getComponent<Player>().direction = Player::Direction::Left;
    entity.addComponent<xy::CommandTarget>().ID = (player == 0) ? CommandID::PlayerOne : CommandID::PlayerTwo;

    //raise a message to say this happened
    auto* msg = m_messageBus.post<PlayerEvent>(MessageID::PlayerMessage);
    msg->type = PlayerEvent::Spawned;
    msg->entity = entity;

    return entity.getIndex();
}

xy::Entity GameServer::spawnNPC(sf::Int32 id, sf::Vector2f pos)
{
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(pos);
    entity.addComponent<Actor>().id = entity.getIndex();
    entity.getComponent<Actor>().type = id;

    entity.addComponent<xy::QuadTreeItem>().setArea(WhirlyBobBounds);

    entity.addComponent<AnimationController>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapItem | CommandID::NPC;

    entity.addComponent<NPC>();
    sf::Uint32 collisionFlags = CollisionFlags::NPCMask;
    switch (id)
    {
    default: break;
    case ActorID::Whirlybob:
        entity.getComponent<NPC>().velocity = 
        {
            xy::Util::Random::value(-1.f, 1.f),
            xy::Util::Random::value(-1.f, 1.f)
        };
        entity.getComponent<NPC>().velocity = xy::Util::Vector::normalise(entity.getComponent<NPC>().velocity);
        entity.addComponent<CollisionComponent>().addHitbox(WhirlyBobBounds, CollisionType::NPC);
        entity.getComponent<xy::Transform>().setOrigin(WhirlyBobOrigin);
        break;
    case ActorID::Clocksy:
        entity.getComponent<NPC>().velocity.x = (xy::Util::Random::value(0, 1) == 1) ? -1.f : 1.f;      
        entity.addComponent<CollisionComponent>().addHitbox(ClocksyBounds, CollisionType::NPC);
        entity.getComponent<CollisionComponent>().addHitbox(ClocksyFoot, CollisionType::Foot); //feets!
        entity.getComponent<xy::Transform>().setOrigin(ClocksyOrigin);
        break;
    case ActorID::Goobly:
        entity.addComponent<CollisionComponent>().addHitbox(GooblyBounds, CollisionType::NPC);
        entity.getComponent<xy::Transform>().setOrigin(GooblyOrigin);
        collisionFlags &= ~CollisionFlags::Solid;
        break;
    case ActorID::Balldock:
        entity.getComponent<NPC>().velocity.x = (xy::Util::Random::value(0, 1) == 1) ? -1.f : 1.f;
        entity.addComponent<CollisionComponent>().addHitbox(BalldockBounds, CollisionType::NPC);
        entity.getComponent<CollisionComponent>().addHitbox(BalldockFoot, CollisionType::Foot);
        entity.getComponent<xy::Transform>().setOrigin(BalldockOrigin);
        break;
    case ActorID::Squatmo:
        entity.addComponent<CollisionComponent>().addHitbox(SquatmoBounds, CollisionType::NPC);
        entity.getComponent<CollisionComponent>().addHitbox(SquatmoFoot, CollisionType::Foot);
        entity.getComponent<xy::Transform>().setOrigin(SquatmoOrigin);
        break;
    }

    entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::NPC);
    entity.getComponent<CollisionComponent>().setCollisionMaskBits(collisionFlags);
    
    auto* msg = m_messageBus.post<NpcEvent>(MessageID::NpcMessage);
    msg->type = NpcEvent::Spawned;
    msg->entityID = entity.getIndex();

    return entity;
}

void GameServer::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    default: break;
    case MessageID::NpcMessage:
    {
        const auto& data = msg.getData<NpcEvent>();
        if (data.type == NpcEvent::Died)
        {
            //remove from list of actors
            sf::Int8 i = 0;
            for (; i < m_mapData.NPCCount; ++i)
            {
                if (m_mapData.NPCs[i].id == data.entityID)
                {
                    break;
                }
            }

            m_mapData.NPCCount--;
            m_mapData.NPCs[i] = m_mapData.NPCs[m_mapData.NPCCount];

            //LOG(std::to_string(m_mapData.actorCount), xy::Logger::Type::Info);
            m_endOfRoundPauseTime = endOfRoundTime;
        }
    }
        break;
    case MessageID::ItemMessage:
    {
        const auto& data = msg.getData<ItemEvent>();
        if (data.actorID == ActorID::Bonus)
        {
            //trigger map skip if player has bonus
            if (data.player.getComponent<Player>().bonusFlags == Bonus::BONUS)
            {
                m_mapData.NPCCount = 0;
                m_endOfRoundPauseTime = -1.f;
                m_mapSkipCount = 4;


                //notify clients
                m_host.broadcastPacket(PacketID::RoundSkip, sf::Uint8(0), xy::NetFlag::Reliable, 1);

                //reset bonus jigger
                auto player = data.player;
                player.getComponent<Player>().bonusFlags = 0;

                auto* msg = m_messageBus.post<ItemEvent>(MessageID::ItemMessage);
                msg->actorID = ActorID::Bonus;
                msg->player = player;
            }
        }
    }
        break;
    case MessageID::PlayerMessage:
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::GotHat)
        {
            auto type = (data.entity.getComponent<Player>().playerNumber == 0) ? HatFlag::OneOn : HatFlag::TwoOn;
            m_host.broadcastPacket(PacketID::HatChange, sf::Uint8(type), xy::NetFlag::Reliable, 1);
        }
        else if (data.type == PlayerEvent::LostHat)
        {
            auto type = (data.entity.getComponent<Player>().playerNumber == 0) ? HatFlag::OneOff : HatFlag::TwoOff;
            m_host.broadcastPacket(PacketID::HatChange, sf::Uint8(type), xy::NetFlag::Reliable, 1);
        }
    }
        break;
    }
}