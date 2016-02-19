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
#include <NetworkDemoPlayerController.hpp>
#include <CommandIds.hpp>

#include <xygine/Reports.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

namespace
{
    const float snapshotInterval = 1.f / 20.f;
    const sf::Vector2f paddleSize(20.f, 100.f);
    const sf::Vector2f ballSize(20.f, 20.f);

    const sf::Vector2f playerOneSpawn(180.f, 540.f);
    const sf::Vector2f playerTwoSpawn(1920.f - 180.f, 540.f);
}

using namespace std::placeholders;
using namespace NetDemo;

Server::Server()
    : m_messageBus          (),
    m_scene                 (m_messageBus),
    m_connection            (m_messageBus),
    m_snapshotAccumulator   (0.f),
    m_ballID                (0u),
    m_collisionWorld        (m_scene, m_messageBus, sf::Color::Cyan)
{
    m_packetHandler = std::bind(&Server::handlePacket, this, _1, _2, _3, _4, _5);
    m_connection.setPacketHandler(m_packetHandler);
    m_connection.setMaxClients(2);

    //m_scene.drawDebug(true);
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

        {
            sf::Lock lock(m_connection.getMutex());
            m_scene.update(dt);
        }
        m_connection.update(dt);

        m_snapshotAccumulator += m_snapshotClock.restart().asSeconds();
        while (m_snapshotAccumulator >= snapshotInterval)
        {
            m_snapshotAccumulator -= snapshotInterval;
            sendSnapshot();
        }
    }
}

void Server::drawDebug(sf::RenderTarget& rt)
{
    rt.draw(m_scene);
}

void Server::setDebugView(sf::View view)
{
    m_scene.setView(view);
}

//private
void Server::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    default: break;
    case NetMessageId::PongMessage:
    {
        auto msgData = msg.getData<PongEvent>();
        switch (msgData.type)
        {
        case PongEvent::BallDestroyed:
            {
                sf::Packet packet;
                packet << xy::PacketID(PacketID::EntityDestroyed) << m_ballID;
                m_connection.broadcast(packet, true);
            }
            spawnBall();
            break;
        default: break;
        }
        break;
    }
    break;
    case xy::Message::NetworkMessage:
    {
        auto& msgData = msg.getData<xy::Message::NetworkEvent>();
        switch (msgData.action)
        {
        case xy::Message::NetworkEvent::ConnectionAdded:

            break;
        case xy::Message::NetworkEvent::ConnectionRemoved:
        {
            auto clid = msgData.clientID;
            auto result = std::find_if(m_players.begin(), m_players.end(), 
                [clid](const Player& player)
            {
                return (player.id == clid);
            });

            if (result != m_players.end())
            {
                xy::Command cmd;
                cmd.entityID = result->entID;
                cmd.action = [](xy::Entity& entity, float)
                {
                    entity.destroy();
                };
                sf::Lock lock(m_connection.getMutex());
                m_scene.sendCommand(cmd);

                m_players.erase(result);
            }
        }
            break;
        default: break;
        }

    }
    break;
    }
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

    //broadcast player info such as ID, position, and last input ID
    packet.clear();
    packet << xy::PacketID(PacketID::PlayerUpdate) << sf::Uint8(m_players.size());
    for (const auto& p : m_players)
    {
        packet << p.entID << p.position << p.lastInputId;
    }
    m_connection.broadcast(packet);

    //broadcast ball info for client reconciliation
    xy::Command cmd;
    cmd.entityID = m_ballID;
    cmd.action = [this](xy::Entity& ent, float)
    {
        auto position = ent.getPosition();
        auto component = ent.getComponent<BallLogic>();
        auto velocity = component->getVelocity();
        auto step = component->getCurrentStep();
        component->clearHistory();

        sf::Packet packet;
        packet << xy::PacketID(PacketID::BallUpdate);
        packet << m_ballID << position.x << position.y << velocity.x << velocity.y << step;
        m_connection.broadcast(packet);
    };
    sf::Lock lock(m_connection.getMutex());
    m_scene.sendCommand(cmd);
}

void Server::handlePacket(const sf::IpAddress& ip, xy::PortNumber port, xy::Network::PacketType type, sf::Packet& packet, xy::Network::ServerConnection* connection)
{
    switch (type)
    {
    default: 

        break;
    case PacketID::PlayerDetails:
    {
        Player player;
        packet >> player.id;
        packet >> player.name;

        if (m_players.size() == 1)
        {
            //send existing player details
            auto position = (m_players[0].number == 1) ? playerOneSpawn : playerTwoSpawn;
            position.y = m_players[0].position;
            sf::Packet p;
            p << xy::PacketID(PacketID::PlayerSpawned);
            p << m_players[0].id << m_players[0].entID;
            p << position.x << position.y;
            m_connection.send(player.id, packet, true);
        }
              
        sf::Lock(m_connection.getMutex());
        spawnPlayer(player);
    }
        break;
    case PacketID::PlayerInput:
    {
        Input input;
        packet >> input;

        auto result = std::find_if(m_players.begin(), m_players.end(), [&input](const Player& p) {return p.id == input.clientID; });
        if (result != m_players.end())
        {
            xy::Command cmd;
            cmd.entityID = result->entID;
            cmd.action = [result, input](xy::Entity& entity, float)
            {
                auto controller = entity.getComponent<PlayerController>();
                controller->setInput(input, false);
                //REPORT("SERVER position", std::to_string(entity.getWorldPosition().y));
                //update player info with velocity, last inputID, position
                result->lastInputId = controller->getLastInputID();
                result->position = controller->getLastPosition();
            };
            sf::Lock lock(m_connection.getMutex());
            m_scene.sendCommand(cmd);
        }
    }
        break;
    }
}

void Server::spawnBall()
{
    sf::Vector2f spawnPos(960.f, 540.f);
    
    auto ballEntity = xy::Entity::create(m_messageBus);
    ballEntity->setPosition(spawnPos);
    
    auto ballLogic = xy::Component::create<BallLogic>(m_messageBus);
    ballLogic->setCollisionObjects(m_collisionWorld.getEntities());
    auto logic = ballEntity->addComponent(ballLogic);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    drawable->getDrawable().setSize(ballSize);
    drawable->getDrawable().setOrigin(ballSize / 2.f);
    drawable->getDrawable().setFillColor(sf::Color::Cyan);
    ballEntity->addComponent(drawable);

    //putting the ball on a different layer means
    //we don't needlessly broadcast its position
    auto ent = m_scene.addEntity(ballEntity, xy::Scene::Layer::BackMiddle); 
    m_ballID = ent->getUID();

    sf::Packet packet;
    packet << xy::PacketID(PacketID::BallSpawned);
    packet << m_ballID;
    packet << spawnPos.x << spawnPos.y;
    packet << logic->getVelocity().x << logic->getVelocity().y;
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
    
    sf::Vector2f position = (player.number == 1) ? playerOneSpawn : playerTwoSpawn;

    auto playerEntity = xy::Entity::create(m_messageBus);
    playerEntity->setPosition(position);
    player.entID = playerEntity->getUID();
    
    auto playerController = xy::Component::create<PlayerController>(m_messageBus);
    playerEntity->addComponent(playerController);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    drawable->getDrawable().setSize(paddleSize);
    drawable->getDrawable().setOrigin(paddleSize / 2.f);
    drawable->getDrawable().setFillColor(sf::Color::Cyan);
    playerEntity->addComponent(drawable);

    m_collisionWorld.addEntity(m_scene.addEntity(playerEntity, xy::Scene::Layer::BackRear));

    sf::Packet packet;
    packet << xy::PacketID(PacketID::PlayerSpawned);
    packet << player.id << player.entID;
    packet << position.x << position.y;
    m_connection.broadcast(packet, true);

    m_players.push_back(player);

    //spawn ball if both players connected
    if (m_players.size() == 2)
    {
        spawnBall();
    }

    return 0;
}