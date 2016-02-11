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

#include <xygine/network/ClientConnection.hpp>

#include <xygine/Log.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Assert.hpp>

#include <SFML/System/Clock.hpp>

namespace
{
    const sf::Int32 CLIENT_TIMEOUT = 10000;
    const sf::Int32 CONNECTION_TIMEOUT = 5000;
    const sf::Uint8 MAX_RETRIES = 10u;
}

using namespace xy;
using namespace xy::Network;

ClientConnection::ClientConnection()
    : m_serverPort  (0u),
    m_clientID      (-1),
    m_connected     (false),
    m_listenThread  (&ClientConnection::listen, this)
{

}

ClientConnection::~ClientConnection()
{
    disconnect();
}

//public
bool ClientConnection::connect()
{
    if (!m_connected)
    {
        m_socket.bind(sf::Socket::AnyPort);
        LOG("CLIENT - Bound connection to port: " + std::to_string(m_socket.getLocalPort()), xy::Logger::Type::Info);

        sf::Packet packet;
        packet << Network::PROTOCOL_ID;
        packet << m_ackSystem.createHeader();
        packet << PacketID(PacketType::Connect);
        //TODO attach other client info such as player name

        if (m_socket.send(packet, m_serverIp, m_serverPort) != sf::Socket::Done)
        {
            LOG("CLIENT - Failed to connect to server", xy::Logger::Type::Error);
            m_socket.unbind();
            return false;
        }

        m_socket.setBlocking(false);
        packet.clear();

        sf::IpAddress rxIP;
        PortNumber rxPort = 0u;
        sf::Clock timer;

        LOG("CLIENT - Attempting to connect to: " + m_serverIp.toString() + ":" + std::to_string(m_serverPort), xy::Logger::Type::Info);
        while (timer.getElapsedTime().asMilliseconds() < CONNECTION_TIMEOUT)
        {
            sf::Socket::Status s = m_socket.receive(packet, rxIP, rxPort);
            if (s != sf::Socket::Done)
            {
                LOG("CLIENT - Failed rx connection packet", xy::Logger::Type::Error);
                continue;
            }
            if (rxIP != m_serverIp)
            {
                LOG("CLIENT - Failed rx connection remote IP", xy::Logger::Type::Error);
                continue;
            }

            sf::Uint32 pID;
            if (!(packet >> pID) || pID != Network::PROTOCOL_ID)
            {
                LOG("CLIENT - Failed rx invalid protocol: " + std::to_string(pID), xy::Logger::Type::Error);
                continue;
            }

            AckSystem::Header header;
            packet >> header;
            m_ackSystem.packetReceived(header, packet.getDataSize());

            PacketID id;
            packet >> id;
            PacketType packetType = static_cast<PacketType>(id);
            if (packetType != PacketType::Connect)
            {
                if (packetType == PacketType::ServerFull)
                {
                    LOG("CLIENT - Server refused connection, server full.", xy::Logger::Type::Info);
                    m_socket.unbind();
                    m_socket.setBlocking(false);
                    return false;
                }
                continue;
            }
            packet >> m_clientID;
            LOG("CLIENT - Assigned ID of: " + std::to_string(m_clientID), xy::Logger::Type::Info);

            m_connected = true;
            m_socket.setBlocking(true);
            m_lastHeartbeat = m_serverTime;
            m_listenThread.launch();

            m_flowControl.reset();

            handlePacket(packetType, packet);
            return true;
        }
        LOG("CLIENT - Connect attempt timed out.", xy::Logger::Type::Error);
        m_socket.unbind();
        m_socket.setBlocking(false);
        return false;
    }
    LOG("CLIENT - Already connected to: " + m_serverIp.toString(), xy::Logger::Type::Error);
    return false;
}

bool ClientConnection::disconnect()
{
    if(!m_connected) return false;

    sf::Packet packet;
    packet << Network::PROTOCOL_ID;
    packet << m_ackSystem.createHeader();
    packet << PacketID(PacketType::Disconnect);

    //TODO should we wait for ack?
    auto status = m_socket.send(packet, m_serverIp, m_serverPort);
    m_connected = false;
    
#ifdef __linux__
    //horrible hack as trying to unbind a blocking socket
    //on linux appears not to work
    m_listenThread.terminate();
#endif
    m_socket.unbind();

    return (status == sf::Socket::Done);
}

void ClientConnection::update(float dt)
{
    m_serverTime += sf::seconds(dt);
    if (m_serverTime.asMilliseconds() < 0)
    {
        m_serverTime -= sf::milliseconds(Network::HighestTimestamp);
        m_lastHeartbeat = m_serverTime;
        return;
    }

    if (m_serverTime.asMilliseconds() - m_lastHeartbeat.asMilliseconds() > CLIENT_TIMEOUT)
    {
        LOG("CLIENT - Server connection timed out", xy::Logger::Type::Info);
        disconnect();
    }

    attemptResends();

    m_ackSystem.update(dt);
    m_flowControl.update(dt, m_ackSystem.getRoundTripTime() * 1000.f);

    REPORT("Client Recieved", std::to_string(m_ackSystem.getReceivedPacketCount()));
    REPORT("Client Sent", std::to_string(m_ackSystem.getSentPacketCount()));
    REPORT("Client Acked", std::to_string(m_ackSystem.getAckedPacketCount()));
    REPORT("Client Lost", std::to_string(m_ackSystem.getLostPacketCount()));
    REPORT("Client Ping", std::to_string(static_cast<int>(m_ackSystem.getRoundTripTime() * 1000.f)) + "ms");
}

bool ClientConnection::send(sf::Packet& packet, bool retry, sf::Uint8 retryCount)
{
    if (!m_connected) return false;

    sf::Packet stampedPacket;
    stampedPacket << Network::PROTOCOL_ID;
    stampedPacket << m_ackSystem.createHeader();
    stampedPacket.append(packet.getData(), packet.getDataSize());

    if (retry)
    {
        XY_ASSERT(retryCount <= MAX_RETRIES, "Maximum number of retries is 10");
        m_resendAttempts.emplace_back();
        auto& rty = m_resendAttempts.back();
        rty.count = retryCount;
        rty.id = m_ackSystem.getLocalSequence();
        rty.packet = packet; //make sure resends don't include a header
    }

    if (m_socket.send(stampedPacket, m_serverIp, m_serverPort) == sf::Socket::Done)
    {
        m_ackSystem.packetSent(stampedPacket.getDataSize());
        return true;
    }
    return false;
}

void ClientConnection::setServerInfo(const sf::IpAddress& ip, PortNumber port)
{
    m_serverIp = ip;
    m_serverPort = port;
}

void ClientConnection::setPacketHandler(const PacketHandler& ph)
{
    m_packetHandler = ph;
}

void ClientConnection::removePacketHandler()
{
    m_packetHandler = nullptr;
}

bool ClientConnection::connected() const
{
    return m_connected;
}

ClientID ClientConnection::getClientID() const
{
    return m_clientID;
}

float ClientConnection::getSendRate() const
{
    return m_flowControl.getSendRate();
}

//private
void ClientConnection::attemptResends()
{
    const auto& acks = m_ackSystem.getAcks();
    for (auto it = m_resendAttempts.begin(); it != m_resendAttempts.end();)
    {
        if (std::find(acks.begin(), acks.end(), it->id) != acks.end())
        {
            //remove pending packet
            it = m_resendAttempts.erase(it);
            continue;
        }
        else
        {
            //check pending acks. if current seq - pending > 32 resend and dec count
            if (m_ackSystem.getLocalSequence() - it->id > 32)
            {
                if (it->count > 0)
                {
                    it->id = m_ackSystem.getLocalSequence();
                    it->count--;
                    send(it->packet);
                    LOG("CLIENT - Resending Packet, " + std::to_string(it->count) + " tries remaining.", xy::Logger::Type::Info);
                }
                else
                {
                    it = m_resendAttempts.erase(it);
                    LOG("CLIENT - Failed sending packet after multiple attempts", xy::Logger::Type::Info);
                    continue;
                }
            }
        }
        ++it;
    }
}

const sf::Time& ClientConnection::getTime() const
{
    return m_serverTime;
}

const sf::Time& ClientConnection::getLastHeartbeat() const
{
    return m_lastHeartbeat;
}

void ClientConnection::setTime(const sf::Time& time)
{
    m_serverTime = time;
}

void ClientConnection::handlePacket(PacketType type, sf::Packet& packet)
{
    switch (type)
    {
    case PacketType::HeartBeat:
        {
            sf::Packet p;
            p << Network::PROTOCOL_ID;
            p << m_ackSystem.createHeader();
            p << PacketID(PacketType::HeartBeat);
            if (m_socket.send(p, m_serverIp, m_serverPort) != sf::Socket::Done)
            {
                LOG("CLIENT - Failed sending heartbeat", xy::Logger::Type::Warning);
            }
            else
            {
                m_ackSystem.packetSent(p.getDataSize());
            }

            sf::Int32 timestamp;
            packet >> timestamp;
            setTime(sf::milliseconds(timestamp));
            m_lastHeartbeat = m_serverTime;
        }
        return;
    case PacketType::Disconnect:
        disconnect();
        return;
    default:break;
    }

    if (m_packetHandler)
    {
        m_packetHandler(type, packet, this);
    }
}

void ClientConnection::listen()
{
    sf::Packet packet;
    sf::IpAddress rxIP;
    PortNumber rxPort = 0u;
    LOG("CLIENT - Started listen thread", xy::Logger::Type::Info);

    while (m_connected)
    {
        packet.clear();
        auto status = m_socket.receive(packet, rxIP, rxPort);
        if (status != sf::Socket::Done)
        {
            if (m_connected)
            {
                LOG("CLIENT - Failed receiving packet from: " + rxIP.toString() + ":" + std::to_string(rxPort) + ", status: " + std::to_string(status), xy::Logger::Type::Error);
                continue;
            }
            else
            {
                LOG("CLIENT - Socket unbound", xy::Logger::Type::Error);
                break;
            }
        }

        if (rxIP != m_serverIp)
        {
            //ignore stranger packets
            continue;
        }

        sf::Uint32 pID;
        if (!(packet >> pID) || pID != Network::PROTOCOL_ID)
        {
            LOG("CLIENT - Bad protocol ID", xy::Logger::Type::Error);
            continue;
        }

        AckSystem::Header header;
        packet >> header;
        m_ackSystem.packetReceived(header, packet.getDataSize());

        //TODO discard packets older than newest rx'd?

        PacketID packetID;
        packet >> packetID;

        //handle the packet
        handlePacket(static_cast<PacketType>(packetID), packet);
    }
    LOG("CLIENT - Listen thread quit.", xy::Logger::Type::Info);
}