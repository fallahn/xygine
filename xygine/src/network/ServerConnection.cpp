/*********************************************************************
© Matt Marchant 2014 - 2017
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

#include <xygine/network/ServerConnection.hpp>

#include <xygine/Log.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Assert.hpp>
#include <xygine/MessageBus.hpp>

#include <SFML/System/Lock.hpp>
#include <SFML/System/Clock.hpp>

#include <algorithm>

namespace
{
    const sf::Int32 HEARTBEAT_RATE = 1000;
    const sf::Int32 HEARTBEAT_RETRIES = 5;
    const sf::Int32 CLIENT_TIMEOUT = 10000;
    const sf::Uint8 MAX_RETRIES = 10u;
}

using namespace xy;
using namespace xy::Network;

ServerConnection::ServerConnection(MessageBus& mb)
    : m_lastClientID    (-1),
    m_maxClients        (4u),
    m_running           (false),
    m_listenThread      (&ServerConnection::listen, this),
    m_totalBytesSent    (0u),
    m_totalBytesReceived(0u),
    m_messageBus        (mb)
{

}

ServerConnection::~ServerConnection()
{
    stop();
}

//public
void ServerConnection::setPacketHandler(const PacketHandler& ph)
{
    m_packetHandler = ph;
}

void ServerConnection::setTimeoutHandler(const TimeoutHandler& th)
{
    m_timoutHandler = th;
}

bool ServerConnection::send(ClientID id, sf::Packet& packet, bool retry, sf::Uint8 retryCount)
{
    sf::Lock lock(m_mutex);

    auto result = m_clients.find(id);
    if (result == m_clients.end()) return false;

    sf::Packet stampedPacket;
    stampedPacket << Network::PROTOCOL_ID;
    stampedPacket << result->second.ackSystem->createHeader();
    stampedPacket.append(packet.getData(), packet.getDataSize());

    if (retry)
    {
        XY_ASSERT(retryCount <= MAX_RETRIES, "Maximum number of retries is 10");
        result->second.resendAttempts.emplace_back();
        auto& rty = result->second.resendAttempts.back();
        rty.count = retryCount;
        rty.id = result->second.ackSystem->getLocalSequence();
        rty.packet = packet; //make sure resends don't include a header
    }

    if (m_outgoingSocket.send(stampedPacket, result->second.ipAddress, result->second.portNumber) != sf::Socket::Done)
    {
        LOG("SERVER - Failed sending packet to " + std::to_string(id), xy::Logger::Type::Warning);
        return false;
    }

    result->second.ackSystem->packetSent(packet.getDataSize());

    m_totalBytesSent += packet.getDataSize();
    return true;
}

bool ServerConnection::send(const sf::IpAddress& ip, PortNumber port, sf::Packet& packet, bool retry, sf::Uint8 retryCount)
{
    sf::Packet stampedPacket;
    stampedPacket << Network::PROTOCOL_ID;

    auto id = getClientID(ip, port);
    if (id != NullID)
    {
        sf::Lock lock(m_mutex);
        stampedPacket << m_clients[id].ackSystem->createHeader();
        m_clients[id].ackSystem->packetSent(packet.getDataSize());
    }
    else
    {
        stampedPacket << AckSystem::Header();
    }
    stampedPacket.append(packet.getData(), packet.getDataSize());
    
    if (retry)
    {
        XY_ASSERT(retryCount <= MAX_RETRIES, "Maximum number of retries is 10");
        m_clients[id].resendAttempts.emplace_back();
        auto& rty = m_clients[id].resendAttempts.back();
        rty.count = retryCount;
        sf::Lock lock(m_mutex);
        rty.id = m_clients[id].ackSystem->getLocalSequence();
        rty.packet = packet; //make sure resends don't include a header
    }

    if (m_outgoingSocket.send(stampedPacket, ip, port) != sf::Socket::Done)
    {
        LOG("SERVER - Failed sending packet to " + ip.toString(), xy::Logger::Type::Warning);
        return false;
    }
    
    m_totalBytesSent += packet.getDataSize();
    return true;
}

void ServerConnection::broadcast(sf::Packet& packet, bool retry, sf::Uint8 retryCount, ClientID ignore)
{
    sf::Lock lock(m_mutex);
    for (auto& c : m_clients)
    {
        if (c.first != ignore)
        {
            send(c.first, packet, retry, retryCount);
        }
    }
}

ClientID ServerConnection::addClient(const sf::IpAddress& ip, PortNumber port)
{
    sf::Lock lock(m_mutex);
    //check not already added
    for (const auto& c : m_clients)
    {
        if (c.second.ipAddress == ip && c.second.portNumber == port)
        {
            return ClientID(Network::NullID);
        }
    }

    //create new client
    ClientID id = m_lastClientID++;
    ClientInfo clientInfo(ip, port, m_serverTime);
    m_clients.emplace(id, std::move(clientInfo));
    LOG("SERVER - Added client with ID: " + std::to_string(id) + " on port: " + std::to_string(port), xy::Logger::Type::Info);

    //broadcast a client has joined
    sf::Packet packet;
    packet << PacketID(PacketType::ClientJoined) << id;
    broadcast(packet, true);

    auto msg = m_messageBus.post<xy::Message::NetworkEvent>(xy::Message::NetworkMessage);
    msg->action = Message::NetworkEvent::ConnectionAdded;
    msg->clientID = id;

    return id;
}

ClientID ServerConnection::getClientID(const sf::IpAddress& ip, PortNumber port)
{
    sf::Lock lock(m_mutex);
    for (const auto& c : m_clients)
    {
        if (c.second.ipAddress == ip && c.second.portNumber == port)
        {
            return c.first;
        }
    }

    return ClientID(Network::NullID);
}

bool ServerConnection::hasClient(ClientID id) const
{
    return (m_clients.find(id) != m_clients.end());
}

bool ServerConnection::hasClient(const sf::IpAddress& ip, PortNumber port)
{
    return (getClientID(ip, port) >= 0);
}

bool ServerConnection::getClientInfo(ClientID id, const ClientInfo* info)
{
    sf::Lock lock(m_mutex);
    for (const auto& c : m_clients)
    {
        if (c.first == id)
        {
            info = &c.second;
            return true;
        }
    }
    info = nullptr;
    return false;
}

bool ServerConnection::removeClient(ClientID id)
{
    sf::Lock lock(m_mutex);
    auto result = m_clients.find(id);
    if (result == m_clients.end()) return false;

    sf::Packet packet;
    packet << PacketID(PacketType::Disconnect);
    send(id, packet, true);
    m_clients.erase(result);

    //notify everyone else client left
    packet.clear();
    packet << PacketID(PacketType::ClientLeft);
    packet << id;
    broadcast(packet, true);

    auto msg = m_messageBus.post<xy::Message::NetworkEvent>(xy::Message::NetworkMessage);
    msg->action = xy::Message::NetworkEvent::ConnectionRemoved;
    msg->clientID = id;

    return true;
}

bool ServerConnection::removeClient(const sf::IpAddress& ip, PortNumber port)
{
    sf::Lock lock(m_mutex);
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
    {
        if (it->second.ipAddress == ip && it->second.portNumber == port)
        {
            auto clientID = it->first;
            
            sf::Packet packet;
            packet << PacketID(PacketType::Disconnect);
            send(it->first, packet, true);
            m_clients.erase(it);

            //notify everyone else client left
            packet.clear();
            packet << PacketID(PacketType::ClientLeft);
            packet << clientID;
            broadcast(packet, true);

            auto msg = m_messageBus.post<xy::Message::NetworkEvent>(xy::Message::NetworkMessage);
            msg->action = xy::Message::NetworkEvent::ConnectionRemoved;
            msg->clientID = clientID;

            return true;
        }
    }

    return false;
}

void ServerConnection::disconnectAll()
{
    if (m_running)
    {
        sf::Packet packet;
        packet << PacketID(PacketType::Disconnect);
        broadcast(packet);
        sf::Lock lock(m_mutex);
        m_clients.clear();
    }
}

bool ServerConnection::start(PortNumber port)
{
    if (m_incomingSocket.bind(port) != sf::Socket::Done)
    {
        return false;
    }

    if (!m_running)
    {
        m_outgoingSocket.bind(sf::Socket::AnyPort); //TODO make a fixed range? easier for firewalling
        init();
        LOG("SERVER - Opened outgoing port: " + std::to_string(m_outgoingSocket.getLocalPort()), xy::Logger::Type::Info);
        LOG("SERVER - Opened incoming port: " + std::to_string(m_incomingSocket.getLocalPort()), xy::Logger::Type::Info);

        m_running = true;
        m_listenThread.launch();
        return true;
    }
    return false;
}

bool ServerConnection::stop()
{
    if (m_running)
    {
        disconnectAll();
        m_running = false;
#ifdef __linux__
        //horrible hack as trying to unbind a blocking socket
        //on linux appears not to work
        m_listenThread.terminate();
#endif
        m_incomingSocket.unbind();
        return true;
    }
    return false;
}

void ServerConnection::update(float dt)
{
    m_serverTime += sf::seconds(dt);

    //check for time overflow
    if (m_serverTime.asMilliseconds() < 0)
    {
        //use sfml data types when networking
        auto maxTime = sf::Int32(Network::HighestTimestamp);
        m_serverTime -= sf::milliseconds(maxTime);
        sf::Lock lock(m_mutex);
        for (auto& c : m_clients)
        {
            c.second.lastHeartbeat = sf::milliseconds(std::abs(c.second.lastHeartbeat.asMilliseconds()) - maxTime);
        }
    }

    sf::Lock lock(m_mutex);
    for (auto it = m_clients.begin(); it != m_clients.end();)
    {
        auto elapsedTime = m_serverTime.asMilliseconds() - it->second.lastHeartbeat.asMilliseconds();

        //update client heartbeat
        if (elapsedTime > HEARTBEAT_RATE)
        {
            //remove timeouts
            if (elapsedTime > CLIENT_TIMEOUT || it->second.heartbeatRetry > HEARTBEAT_RETRIES)
            {
                auto clid = it->first;
                LOG("SERVER - Client " + std::to_string(clid) + " has timed out", xy::Logger::Type::Info);
                if (m_timoutHandler)
                {
                    m_timoutHandler(it->first);
                }
                it = m_clients.erase(it);
                
                //broadcast client leaving notification (could use a reason flag?)
                sf::Packet packet;
                packet << PacketID(PacketType::ClientLeft) << clid;
                broadcast(packet, true);

                auto msg = m_messageBus.post<xy::Message::NetworkEvent>(xy::Message::NetworkMessage);
                msg->action = xy::Message::NetworkEvent::ConnectionRemoved;
                msg->clientID = clid;

                continue;
            }

            //send heartbeat to remaining
            if (!it->second.heartbeatWaiting || (elapsedTime >= HEARTBEAT_RATE * (it->second.heartbeatRetry + 1)))
            {
                sf::Packet heartbeat;
                //don't need to add ack header here as send() takes care of it
                heartbeat << PacketID(PacketType::HeartBeat);
                heartbeat << m_serverTime.asMilliseconds();
                send(it->first, heartbeat);
                //LOG("SERVER - ping!", xy::Logger::Type::Info);

                if (it->second.heartbeatRetry == 0)
                {
                    it->second.heartbeatSent = m_serverTime;
                }
                it->second.heartbeatWaiting = true;
                ++it->second.heartbeatRetry;

                m_totalBytesSent += heartbeat.getDataSize();
            }

            it->second.attemptResends(it->first, *this);
            it->second.ackSystem->update(dt);           
        }
        ++it;
    }
}

bool ServerConnection::running() const
{
    return m_running;
}

std::size_t ServerConnection::getClientCount() const
{
    return m_clients.size();
}

void ServerConnection::setMaxClients(std::size_t count)
{
    m_maxClients = count;
}

std::size_t ServerConnection::getMaxClients() const
{
    return m_maxClients;
}

//private
void ServerConnection::listen()
{
    sf::IpAddress ip;
    PortNumber port = 0u;
    sf::Packet packet;

    LOG("SERVER - Started listening...", xy::Logger::Type::Info);

    while (m_running)
    {
        packet.clear();
        auto status = m_incomingSocket.receive(packet, ip, port);
        if (status != sf::Socket::Done)
        {
            if (m_running)
            {
                LOG("SERVER - Error rx packet from: " + ip.toString() + ":" + std::to_string(port) + ", Code: " + std::to_string(status), xy::Logger::Type::Warning);
                continue;
            }
            else
            {
                LOG("SERVER - Socket unbound", xy::Logger::Type::Warning);
                break;
            }
        }

        //check remote IP is actually one of our clients - TODO this only works after accepting a connection, duh
        /*if (std::find_if(m_clients.begin(), m_clients.end(), [&ip](const std::unordered_map<ClientID, ClientInfo>::value_type& ci) {return ci.second.ipAddress == ip; }) == m_clients.end())
        {
            LOG("SERVER - Received packet from unknown source", xy::Logger::Type::Warning);
            continue;
        }*/

        m_totalBytesReceived += packet.getDataSize();

        sf::Uint32 pID;
        if (!(packet >> pID) || pID != Network::PROTOCOL_ID)
        {
            LOG("SERVER - invalid or missing protocol ID", xy::Logger::Type::Warning);
            continue;
        }

        AckSystem::Header header;
        packet >> header;
        auto clientID = getClientID(ip, port);
        if (clientID != NullID)
        {
            //REPORT("SERVER header", std::to_string(header.ack));

            sf::Lock lock(m_mutex);
            m_clients[clientID].ackSystem->packetReceived(header, packet.getDataSize());
            //TODO discard this packet if it's older than the newest rx'd?
            //REPORT("SERVER sent", std::to_string(m_clients[clientID].ackSystem->getSentPacketCount()));
            //REPORT("SERVER acked", std::to_string(m_clients[clientID].ackSystem->getAckedPacketCount()));
            /*REPORT("SERVER lost", std::to_string(m_clients[clientID].ackSystem->getLostPacketCount()));*/
        }

        //REPORT("incoming ack", std::to_string(header.ack));


        PacketID packetID = 0;
        packet >> packetID;

        handlePacket(ip, port, static_cast<PacketType>(packetID), packet);
    }

    LOG("SERVER - Stopped listening...", xy::Logger::Type::Info);
    LOG("SERVER - Listen thread quit.", xy::Logger::Type::Info);
}

void ServerConnection::init()
{
    m_lastClientID = 0u;
    m_running = false;
    m_totalBytesSent = 0u;
    m_totalBytesReceived = 0u;
}

void ServerConnection::handlePacket(const sf::IpAddress& ip, PortNumber port, PacketType id, sf::Packet& packet)
{
    //TODO do we want to consume packets by returning from the
    //switch block, or allow users to act on these packet types too?
    sf::Lock lock(m_mutex);
    ClientID clientID = getClientID(ip, port);
    if (clientID >= 0)
    {
        switch (id)
        {
        case PacketType::Disconnect:
            {
                removeClient(ip, port);
                return;
            }
        case PacketType::HeartBeat:
            {
                auto c = m_clients.find(clientID);
                if (c == m_clients.end())
                {
                    LOG("SERVER - Heartbeat from unknown client...", xy::Logger::Type::Warning);
                    return;
                }
                if (!c->second.heartbeatWaiting)
                {
                    LOG("SERVER - Invalid heartbeat packet received...", xy::Logger::Type::Warning);
                    return;
                }

                c->second.lastHeartbeat = m_serverTime;
                c->second.heartbeatWaiting = false;
                c->second.heartbeatRetry = 0u;
                return;
            }
        default: break;
        }
    }
    else
    {
        if (id == PacketType::Connect)
        {
            if (m_clients.size() < m_maxClients)
            {
                ClientID nid = addClient(ip, port);
                sf::Packet p;
                p << PacketID(PacketType::Connect);
                p << nid;
                send(nid, p);
                LOG("SERVER - Sent connection acceptance", xy::Logger::Type::Info);
            }
            else
            {
                //send refusal :(
                sf::Packet p;
                p << PacketID(PacketType::ServerFull);
                send(ip, port, p);
                LOG("SERVER - Sent connection refusal, server full.", xy::Logger::Type::Info);
            }
            return;
        }
    }

    if (m_packetHandler) m_packetHandler(ip, port, id, packet, this);
}


//----------//


void ServerConnection::ClientInfo::attemptResends(ClientID clid, ServerConnection& connection)
{
    const auto& acks = ackSystem->getAcks();
    //REPORT("SERVER ack count", std::to_string(acks.size()));
    for (auto it = resendAttempts.begin(); it != resendAttempts.end();)
    {
        if (std::find_if(acks.begin(), acks.end(), [it](const auto& ack){return it->id == ack;}) != acks.end())
        {
            //remove pending packet
            it = resendAttempts.erase(it);
            continue;
        }
        else
        {
            //check pending acks. if current seq - pending > 32 resend and dec count
            if (ackSystem->getLocalSequence() - it->id > 32)
            {
                if (it->count > 0)
                {
                    it->id = ackSystem->getLocalSequence();
                    it->count--;
                    connection.send(clid, it->packet);
                    LOG("SERVER - Resending Packet, " + std::to_string(it->count) + " tries remaining.", xy::Logger::Type::Info);
                }
                else
                {
                    it = resendAttempts.erase(it);
                    LOG("SERVER - Packet remained un-acked after multiple attempts", xy::Logger::Type::Info);
                    continue;
                }
            }
        }
        ++it;
    }
}