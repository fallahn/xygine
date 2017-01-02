/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <NetworkDemoBot.hpp>
#include <NetworkDemoPacketIDs.hpp>
#include <NetworkDemoPlayerInput.hpp>

#include <SFML/System/Lock.hpp>
#include <SFML/System/Clock.hpp>

#include <xygine/Log.hpp>

using namespace std::placeholders;
using namespace NetDemo;

namespace
{
    NetDemo::Input input;
    float broadcastAccumulator = 0.f;
    sf::Clock broadcastClock;
}

NetBot::NetBot()
    : m_position(540.f)
{
    m_packetHandler = std::bind(&NetBot::handlePacket, this, _1, _2, _3);
    m_connection.setPacketHandler(m_packetHandler);
}

//public
void NetBot::update(float dt)
{
    m_connection.update(dt);

    //LOG("Ball: " + std::to_string(m_ball.position.x), xy::Logger::Type::Info);
    
    if (connected())
    {
        //calc input from ball position
        const float distance = m_ball.position.y - m_position;
        m_position += distance * 0.4f;
        input.position = m_position;

        //update connection
        const float sendRate = 1.f / m_connection.getSendRate();
        broadcastAccumulator += broadcastClock.restart().asSeconds();
        while (broadcastAccumulator >= sendRate)
        {
            input.clientID = m_connection.getClientID();
            input.timestamp = m_connection.getTime().asMilliseconds();

            broadcastAccumulator -= sendRate;
            sf::Packet packet;
            packet << xy::PacketID(PacketID::PlayerInput) << input;
            m_connection.send(packet);

            input.counter++;
        }
    }
}

void NetBot::connect(sf::IpAddress addr, xy::PortNumber port)
{
    m_connection.setServerInfo(addr, port);
    m_connection.connect();
}

void NetBot::disconnect()
{
    m_connection.disconnect();
}

//private
void NetBot::handlePacket(xy::Network::PacketType type, sf::Packet& packet, xy::Network::ClientConnection*)
{
    switch (type)
    {
    default:break;
    case xy::Network::Connect:
    {
        sf::Packet newPacket;
        newPacket << xy::PacketID(PacketID::PlayerDetails);
        newPacket << m_connection.getClientID();
        newPacket << "CPU";
        m_connection.send(newPacket, true);
    }
        break;
    case PacketID::BallSpawned:
    case PacketID::BallUpdate:
    {
        sf::Uint64 id;
        sf::Lock lock(m_connection.getMutex());
        packet >> id >> m_ball.position.x >> m_ball.position.y >> m_ball.velocity.x >> m_ball.velocity.y;
    }
        break;
    }
}