/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/network/EnetHostImpl.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/Assert.hpp"

#include "xyginext/network/NetData.hpp"

using namespace xy;

namespace
{
    ENetPacket* createPacket(std::uint8_t id, const void* data, std::size_t size, NetFlag flags)
    {
        std::int32_t packetFlags = 0;
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

        ENetPacket* packet = enet_packet_create(&id, sizeof(id), packetFlags);
        enet_packet_resize(packet, sizeof(id) + size);
        std::memcpy(&packet->data[sizeof(id)], data, size);

        return packet;
    }
}

EnetHostImpl::EnetHostImpl()
    : m_host(nullptr)
{
    if (!NetConf::instance)
    {
        NetConf::instance = std::make_unique<NetConf>();
    }
}

EnetHostImpl::~EnetHostImpl()
{
    stop();
}

//public
bool EnetHostImpl::start(const std::string& address, std::uint16_t port, std::size_t maxClients, std::size_t maxChannels, std::uint32_t incoming, std::uint32_t outgoing)
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
        if (enet_address_set_host(&add, address.c_str()) != 0)
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

    //enables the default packet compression
    enet_host_compress_with_range_coder(m_host);

    LOG("Created server host on port " + std::to_string(port), Logger::Type::Info);
    return true;
}

void EnetHostImpl::stop()
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

bool EnetHostImpl::pollEvent(NetEvent& evt)
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
            evt.packet.setPacketData(hostEvt.packet->data, hostEvt.packet->dataLength);
            
            enet_packet_destroy(hostEvt.packet);
            break;
        }
        evt.peer.setPeer(hostEvt.peer);
        return true;
    }
    return false;
}

void EnetHostImpl::broadcastPacket(std::uint8_t id, const void* data, std::size_t size, NetFlag flags, std::uint8_t channel)
{
    if (m_host)
    {
        enet_host_broadcast(m_host, channel, createPacket(id, data, size, flags));
    }
}

void EnetHostImpl::sendPacket(const NetPeer& peer, std::uint8_t id, const void* data, std::size_t size, NetFlag flags, std::uint8_t channel)
{
    if (peer)
    {
        enet_peer_send(static_cast<_ENetPeer*>(const_cast<void*>(peer.getPeer())), channel, createPacket(id, data, size, flags));
    }
}

std::size_t EnetHostImpl::getConnectedPeerCount() const
{
    return (m_host) ? m_host->connectedPeers : 0;
}

std::uint32_t EnetHostImpl::getAddress() const
{
    return m_host ? m_host->address.host : 0;
}

std::uint16_t EnetHostImpl::getPort() const
{
    return m_host ? m_host->address.port : 0;
}
