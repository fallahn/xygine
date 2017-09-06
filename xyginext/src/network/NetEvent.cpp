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

#include <enet/enet.h>

#include <xyginext/network/NetData.hpp>

using namespace xy;

NetEvent::Packet::Packet()
    : m_packet(nullptr),
    m_id(0)
{

}

NetEvent::Packet::~Packet()
{
    if (m_packet)
    {
        enet_packet_destroy(m_packet);
    }
}

//public
sf::Uint32 NetEvent::Packet::getID() const
{
    XY_ASSERT(m_packet, "Not a valid packet instance");
    return m_id;
}

const void* NetEvent::Packet::getData() const
{
    XY_ASSERT(m_packet, "Not a valid packet instance");
    return &m_packet->data[sizeof(sf::Uint32)];
}

std::size_t NetEvent::Packet::getSize() const
{
    XY_ASSERT(m_packet, "Not a valid packet instance");
    return m_packet->dataLength - sizeof(sf::Uint32);
}

//private
void NetEvent::Packet::setPacketData(ENetPacket* packet)
{
    if (m_packet)
    {
        enet_packet_destroy(m_packet);
    }

    m_packet = packet;

    if (m_packet)
    {
        std::memcpy(&m_id, m_packet->data, sizeof(sf::Uint32));
    }
}