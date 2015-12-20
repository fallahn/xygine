/*********************************************************************
Matt Marchant 2014 - 2015
http://trederia.blogspot.com

xygine - Zlib license.

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

#include <xygine/Server.hpp>
#include <xygine/Protocol.hpp>
#include <xygine/Log.hpp>
#include <xygine/Util.hpp>
#include <xygine/QuadTreeComponent.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Network/Packet.hpp>
#include <SFML/System/Lock.hpp>

using namespace xy;

namespace
{
    sf::Uint16 portNumber = 27012u;
    sf::Int16 clientUID = 16384;

    const float stepInterval = 1.f / 60.f;
    const float tickInterval = 1.f / 16.f;
    const float pingRate = 2.25f;

    //scene isn't drawn so put everything on the same layer to make it 
    //easier to iterate over entities
    const Scene::Layer sceneLayer = Scene::Layer::BackRear;
    const float sceneWidth = 1920.f * 5.f;
}

//server ctor
GameServer::GameServer()
    : m_thread          (&GameServer::executionThread, this),
    m_listening         (false),
    m_clientTimeoutTime (3.f),
    m_maxPlayers        (4u),
    m_connectedPlayers  (0u),
    m_waitingThreadEnd  (false),
    m_messageBus        (),
    m_scene             (m_messageBus, false)
{
    m_listener.setBlocking(false);
    m_connections.emplace_back(std::make_unique<RemoteConnection>());

    updateClientState = std::bind(&GameServer::updateClientLobbyState, this);

    m_thread.launch();
}

//dtor
GameServer::~GameServer()
{
    LOG("SERVER Server going down now... baibai", Logger::Type::Info);
    m_waitingThreadEnd = true;
    m_thread.wait();
}

//public
sf::Uint16 GameServer::getPort()
{
    return portNumber;
}

void GameServer::setPort(sf::Uint16 port)
{
    sf::Lock lock(m_mutex);
    portNumber = port;
}

void GameServer::setMaxPlayers(sf::Uint32 players)
{
    sf::Lock lock(m_mutex);
    m_maxPlayers = std::min(players, 4u);
}

//private
void GameServer::setListening(bool listening)
{
    if (listening)
    {
        if (!m_listening)
        {
            m_listening = (m_listener.listen(portNumber) == sf::TcpListener::Done);
        }
    }
    else
    {
        m_listener.close();
        m_listening = false;
    }
}

void GameServer::executionThread()
{
    setListening(true);

    LOG("SERVER starting server on " + std::to_string(portNumber), Logger::Type::Info);

    float stepTime = 0.f;
    float tickTime = 0.f;

    sf::Clock stepClock;
    sf::Clock tickClock;
    sf::Clock pingClock;

    while (!m_waitingThreadEnd)
    {
        handlePackets();        
        handleConnections();

        stepTime += stepClock.restart().asSeconds();
        tickTime += tickClock.restart().asSeconds();

        while (stepTime >= stepInterval)
        {
            update(stepInterval);
            stepTime -= stepInterval;
        }

        while (tickTime >= tickInterval)
        {
            tick();
            tickTime -= tickInterval;
        }

        if (pingClock.getElapsedTime().asSeconds() > pingRate)
        {
            pingClients();
            pingClock.restart();
        }

        sf::sleep(sf::milliseconds(100));
    }
}

void GameServer::update(float dt)
{
    while (!m_messageBus.empty())
    {
        auto msg = m_messageBus.poll();
        handleMessage(msg);
        m_scene.handleMessage(msg);
    }

    m_scene.update(dt);
}

void GameServer::tick()
{
    updateClientState();
}

float GameServer::now() const
{
    return m_clock.getElapsedTime().asSeconds();
}

void GameServer::handlePackets()
{
    bool hasTimeout = false;
    for (auto& c : m_connections)
    {
        if (c->ready)
        {
            sf::Packet packet;
            while (c->socket.receive(packet) == sf::Socket::Done)
            {
                handlePacket(packet, *c, hasTimeout);
                c->lastPacketTime = now();
                packet.clear();
            }

            if (now() >= c->lastPacketTime + m_clientTimeoutTime)
            {
                c->timeout = true;
                hasTimeout = true;
            }
        }
    }

    if (hasTimeout)
    {
        handleDisconnections();
    }
}

void GameServer::handlePacket(sf::Packet& packet, RemoteConnection& connection, bool& hasTimeout)
{
    sf::Int32 packetType;
    packet >> packetType;
    switch (packetType)
    {
    case Client::StartReady: //TODO check client is actually host
    {
        //build world
        m_scene.reset();
        //m_scene.setSceneWidth(sceneWidth);


        //switch to updating clients with game info
        updateClientState = std::bind(&GameServer::updateClientGameState, this);

        //set listening false so players can't join mid-game
        setListening(false);

        LOG("SERVER server began game", Logger::Type::Info);

        packet.clear();
        packet << static_cast<sf::Int32>(Server::GameStart);
        sendToAll(packet);
    }
        break;
    case Client::InputUpdate:
    {

    }
        break;
    case Client::RequestClientID:
    {
        sf::Packet replyPacket;
        replyPacket << static_cast<sf::Int32>(Server::ClientID) << connection.uid;
        connection.socket.send(replyPacket);
        //LOG("SERVER " + std::to_string(packetType), Logger::Type::Info);
    }
        break;
    case Client::Ping:

        break;
        default: break;
    }
}

void GameServer::handleConnections()
{
    if (!m_listening) return;

    if (m_listener.accept(m_connections[m_connectedPlayers]->socket) == sf::TcpListener::Done)
    {
        sf::Int16 uid = clientUID++;

        sf::Packet packet;
        packet << static_cast<sf::Int32>(Server::ClientID) << uid;
        m_connections[m_connectedPlayers]->socket.send(packet);

        m_connections[m_connectedPlayers]->ready = true;
        m_connections[m_connectedPlayers]->lastPacketTime = now();
        m_connections[m_connectedPlayers]->uid = uid;

        m_connectedPlayers++;
        LOG("SERVER connection accepted", Logger::Type::Info);

        if (m_connectedPlayers >= m_maxPlayers)
        {
            setListening(false);
        }
        else
        {
            m_connections.emplace_back(std::make_unique<RemoteConnection>());
        }
    }
}

void GameServer::handleDisconnections()
{
    for (auto it = m_connections.begin(); it != m_connections.end();)
    {
        if ((*it)->timeout)
        {
            //tell clients to despawn player
            sendToAll(sf::Packet() << static_cast<sf::Int32>(Server::PlayerDisconnect) << (*it)->uid);

            m_connectedPlayers--;
            it = m_connections.erase(it);

            /*if(m_connectedPlayers < m_maxPlayers)
            {
                m_connections.emplace_back(std::make_unique<RemoteConnection>());
                setListening(true);
            }*/
        }
        else
        {
            ++it;
        }
    }
}

void GameServer::sendToAll(sf::Packet& packet)
{
    for (auto& c : m_connections)
        if (c->ready) c->socket.send(packet);
}

void GameServer::pingClients()
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::Ping) << m_clock.getElapsedTime().asMilliseconds();
    sendToAll(packet);
}

void GameServer::handleMessage(const Message& msg)
{
    switch (msg.id)
    {
    case Message::Type::EntityMessage:
    {
        auto& msgData = msg.getData<Message::EntityEvent>();
        switch (msgData.action)
        {
        case Message::EntityEvent::Destroyed:
            break;
        default: break;
        }
        break;
    }
    case Message::Type::PlayerMessage:
    {
        auto& msgData = msg.getData<Message::PlayerEvent>();
        switch (msgData.action)
        {
            break;
        case Message::PlayerEvent::Spawned:
        {

        }
        break;
        case Message::PlayerEvent::Died:
        {

        }
        break;
        default: break;
        }
        break;
    }
    default: break;
    }
}

void GameServer::updateClientGameState()
{
    //send position of all entities
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::UpdateClientState);
    packet << m_clock.getElapsedTime().asMilliseconds();
    packet << m_scene.getLayer(sceneLayer).size();

    for (auto& c : m_connections)
    {
        c->socket.send(packet);
    }

    //send player info
    packet.clear();
    packet << static_cast<sf::Int32>(Server::UpdatePlayerState);

    for (auto& c : m_connections)
    {
        c->socket.send(packet);
    }
}

void GameServer::updateClientLobbyState()
{
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::LobbyData);
    packet << m_connectedPlayers;

    sendToAll(packet);
}

//ctor for remote connections
GameServer::RemoteConnection::RemoteConnection()
    : ready(false),
    timeout(false)
{
    socket.setBlocking(false);
}
