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

//this should always be included first on windows, to ensure it is
//included before windows.h (in this case by Log.hpp)
#include <enet/enet.h> 

#include "NetConf.hpp"
#include <xyginext/network/NetHost.hpp>
#include <xyginext/core/Log.hpp>
#include <xyginext/core/Assert.hpp>

using namespace xy;

namespace
{
    ENetPacket* createPacket(sf::Uint32 id, void* data, std::size_t size, NetFlag flags)
    {
        sf::Int32 packetFlags = 0;
        if (flags == NetFlag::Reliable)
        {
            packetFlags |= ENET_PACKET_FLAG_RELIABLE;
        }
        else if (flags == NetFlag::Unreliable)
        {
            packetFlags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
        }
        else if (flags == NetFlag::Unsequenced)
        {
            packetFlags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT | ENET_PACKET_FLAG_UNSEQUENCED;
        }

        ENetPacket* packet = enet_packet_create(&id, sizeof(sf::Uint32), packetFlags);
        enet_packet_resize(packet, sizeof(sf::Uint32) + size);
        std::memcpy(&packet->data[sizeof(sf::Uint32)], data, size);

        return packet;
    }
}

NetHost::NetHost()
    : m_host    (nullptr)
{
    if (!NetConf::instance)
    {
        NetConf::instance = std::make_unique<NetConf>();
    }
}

NetHost::~NetHost()
{
    stop();
}

//public
bool NetHost::start(const std::string& address, sf::Uint16 port, std::size_t maxClients, std::size_t maxChannels, sf::Uint32 incoming, sf::Uint32 outgoing)
{
    if (m_host)
    {
        Logger::log("Host already exists!", Logger::Type::Error);
        return false;
    }
    
    if (!NetConf::instance->m_initOK)
    {
        Logger::log("Network subsystem not initialised, creating host failed", Logger::Type::Error);
        return false;
    }

    XY_ASSERT(port > 0, "Invalid port value");
    XY_ASSERT(maxChannels > 0, "Invalid channel count");
    XY_ASSERT(maxClients > 0, "Invalid client count");

    ENetAddress add;
    if (address.empty())
    {
        add.host = ENET_HOST_ANY;
    }
    else
    {
        if (!enet_address_set_host(&add, address.c_str()) == 0)
        {
            Logger::log("Failed setting host listen address", Logger::Type::Error);
            return false;
        }
    }
    add.port = port;

    m_host = enet_host_create(&add, maxClients, maxChannels, incoming, outgoing);
    if (!m_host)
    {
        Logger::log("There was an error creating the server host", Logger::Type::Error);
        return false;
    }

    LOG("Created server host on port " + std::to_string(port), Logger::Type::Info);
    return true;
}

void NetHost::stop()
{
    if (m_host)
    {
        if (m_host->connectedPeers > 0)
        {
            for (auto i = 0u; i < m_host->connectedPeers; ++i)
            {
                auto peer = &m_host->peers[i];
                
                ENetEvent evt;
                enet_peer_disconnect(peer, 0);

                //wait 3 seconds for a response
                while (enet_host_service(m_host, &evt, 3000) > 0)
                {
                    switch (evt.type)
                    {
                    default:break;
                    case ENET_EVENT_TYPE_RECEIVE:
                        //clear rx'd packets from buffer by destroying them
                        enet_packet_destroy(evt.packet);
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        peer = nullptr;
                        LOG("Disconnected client", Logger::Type::Info);
                        return;
                    }
                }

                //timed out so force disconnect
                LOG("Server disconnect timed out", Logger::Type::Info);
                enet_peer_reset(peer);
            }
        }

        enet_host_destroy(m_host);
        m_host = nullptr;
    }
}

bool NetHost::pollEvent(NetEvent& evt)
{
    if (!m_host) return false;

    ENetEvent hostEvt;
    if (enet_host_service(m_host, &hostEvt, 0) > 0)
    {
        switch (hostEvt.type)
        {
        default: 
            evt.type = NetEvent::None;
            break;
        case ENET_EVENT_TYPE_CONNECT:
            evt.type = NetEvent::ClientConnect;
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            evt.type = NetEvent::ClientDisconnect;
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            evt.type = NetEvent::PacketReceived;
            evt.packet.setPacketData(hostEvt.packet);
            //our event takes ownership and promises to clean up the packet
            //enet_packet_destroy(hostEvt.packet);
            break;
        }
        evt.peer.m_peer = hostEvt.peer;
        return true;
    }
    return false;
}

void NetHost::broadcastPacket(sf::Uint32 id, void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    if (m_host)
    {
        enet_host_broadcast(m_host, channel, createPacket(id, data, size, flags));
    }
}

void NetHost::sendPacket(const NetPeer& peer, sf::Uint32 id, void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    if (peer.m_peer)
    {
        enet_peer_send(peer.m_peer, channel, createPacket(id, data, size, flags));
    }
}