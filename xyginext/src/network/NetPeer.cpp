/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include <enet/enet.h>

#include <xyginext/network/NetData.hpp>

using namespace xy;

std::string Detail::getEnetPeerAddress(void* peer)
{
    XY_ASSERT(peer, "Not a valid peer");

    auto bytes = static_cast<_ENetPeer*>(peer)->address.host;

    std::string ret = std::to_string(bytes & 0x000000FF);
    ret += "." + std::to_string((bytes & 0x0000FF00) >> 8);
    ret += "." + std::to_string((bytes & 0x00FF0000) >> 16);
    ret += "." + std::to_string((bytes & 0xFF000000) >> 24);
    return ret;
}

sf::Uint16 Detail::getEnetPeerPort(void* peer)
{
    XY_ASSERT(peer, "Not a valid peer");

    return static_cast<_ENetPeer*>(peer)->address.port;
}

sf::Uint32 Detail::getEnetPeerID(void* peer)
{
    XY_ASSERT(peer, "Not a valid peer");

    return static_cast<_ENetPeer*>(peer)->connectID;
}

sf::Uint32 Detail::getEnetRoundTrip(void* peer)
{
    XY_ASSERT(peer, "Not a valid peer");

    return static_cast<_ENetPeer*>(peer)->roundTripTime;
}

NetPeer::State Detail::getEnetPeerState(void* peer)
{
    if (!peer)
    {
        return NetPeer::State::Disconnected;
    }

    switch (static_cast<_ENetPeer*>(peer)->state)
    {
    case ENET_PEER_STATE_ACKNOWLEDGING_CONNECT:
        return NetPeer::State::AcknowledingConnect;
    case ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT:
        return NetPeer::State::AcknowledingDisconnect;
    case ENET_PEER_STATE_CONNECTED:
        return NetPeer::State::Connected;
    case ENET_PEER_STATE_CONNECTING:
        return NetPeer::State::Connecting;
    case ENET_PEER_STATE_CONNECTION_PENDING:
        return NetPeer::State::PendingConnect;
    case ENET_PEER_STATE_CONNECTION_SUCCEEDED:
        return NetPeer::State::Succeeded;
    case ENET_PEER_STATE_DISCONNECTED:
        return NetPeer::State::Disconnected;
    case ENET_PEER_STATE_DISCONNECTING:
        return NetPeer::State::Disconnecting;
    case ENET_PEER_STATE_DISCONNECT_LATER:
        return NetPeer::State::DisconnectLater;
    case ENET_PEER_STATE_ZOMBIE:
        return NetPeer::State::Zombie;
    }
    return NetPeer::State::Zombie;
}