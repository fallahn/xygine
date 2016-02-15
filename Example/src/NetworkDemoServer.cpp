/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <NetworkDemoServer.hpp>
#include <NetworkDemoBallLogic.hpp>
#include <NetworkDemoPacketIDs.hpp>

namespace
{
    const float snapshotInterval = 1.f / 20.f;
}

using namespace std::placeholders;

Server::Server()
    : m_messageBus          (),
    m_scene                 (m_messageBus),
    m_snapshotAccumulator   (0.f)
{
    m_packetHandler = std::bind(&Server::handlePacket, this, _1, _2, _3, _4, _5);
    m_connection.setPacketHandler(m_packetHandler);
    m_connection.setMaxClients(2);

}

//public
bool Server::start()
{
    m_snapshotAccumulator = 0.f;
    LOG("SERVER - starting connection", xy::Logger::Type::Info);
    return m_connection.start();
}

void Server::stop()
{
    m_connection.stop();
}

void Server::update(float dt)
{
    if (m_connection.running())
    {
        while (!m_messageBus.empty())
        {
            const auto& msg = m_messageBus.poll();
            handleMessage(msg);
            m_scene.handleMessage(msg);
        }

        m_scene.update(dt);
        m_connection.update(dt);

        m_snapshotAccumulator += m_snapshotClock.restart().asSeconds();
        while (m_snapshotAccumulator >= snapshotInterval)
        {
            m_snapshotAccumulator -= snapshotInterval;
            sendSnapshot();
        }
    }
}

//private
void Server::handleMessage(const xy::Message& msg)
{

}

void Server::sendSnapshot()
{
    const auto& ents = m_scene.getLayer(xy::Scene::Layer::BackRear).getChildren();

    sf::Packet packet;
    packet << xy::PacketID(PacketID::PositionUpdate);
    packet << sf::Uint8(ents.size());

    for (const auto& e : ents)
    {
        auto position = e->getPosition();
        packet << e->getUID() << position.x << position.y;
    }
    m_connection.broadcast(packet);
}

void Server::handlePacket(const sf::IpAddress& ip, xy::PortNumber port, xy::Network::PacketType type, sf::Packet& packet, xy::Network::ServerConnection* connection)
{
    switch (type)
    {
    default: break;
    case PacketID::PlayerDetails:
        Player player;
        packet >> player.id;
        packet >> player.name;
        sf::Lock(connection->getMutex());
        spawnPlayer(player);
        break;
    }
}

//temp
void Server::spawnBall()
{
    sf::Vector2f spawnPos(960.f, 540.f);
    auto ballEntity = xy::Entity::create(m_messageBus);
    ballEntity->setPosition(spawnPos);
    auto ballLogic = xy::Component::create<BallLogic>(m_messageBus);
    ballEntity->addComponent(ballLogic);
    auto ent = m_scene.addEntity(ballEntity, xy::Scene::Layer::BackRear);

    sf::Packet packet;
    packet << xy::PacketID(PacketID::BallSpawned);
    packet << ent->getUID();
    packet << spawnPos.x << spawnPos.y;
    m_connection.broadcast(packet, true);
}

sf::Uint64 Server::spawnPlayer(Player& player)
{
    XY_ASSERT(m_players.size() < 2, "Too many clients!");
    if (m_players.empty())
    {
        player.number = 1;
    }
    else
    {
        player.number = (m_players[0].number == 1) ? 2 : 1;
    }
    m_players.push_back(player);

    sf::Vector2f position;
    position.y = 540.f;
    position.x = (player.number == 1) ? 30.f : 1920 - 30.f;

    auto playerEntity = xy::Entity::create(m_messageBus);
    //TODO add logic controller
    auto ent = m_scene.addEntity(playerEntity, xy::Scene::Layer::BackRear);

    sf::Packet packet;
    packet << xy::PacketID(PacketID::PlayerSpawned);
    packet << player.id << ent->getUID();
    packet << position.x << position.y;
    m_connection.broadcast(packet, true);

    //spawn ball if both players connected
    if (m_players.size() == 1/*2*/)
    {
        spawnBall();
    }

    return 0;
}