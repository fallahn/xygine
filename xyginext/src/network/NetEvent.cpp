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

#ifdef min
#undef min
#endif

#include <xyginext/network/NetData.hpp>

using namespace xy;

NetEvent::Packet::Packet()
    : m_id(0),
    m_size(0)
{

}

NetEvent::Packet::~Packet()
{
    /*if (m_packet)
    {
        enet_packet_destroy(m_packet);
    }*/
}

//public
sf::Uint32 NetEvent::Packet::getID() const
{
    XY_ASSERT(m_size, "Not a valid packet instance");
    return m_id;
}

const void* NetEvent::Packet::getData() const
{
    XY_ASSERT(m_size, "Not a valid packet instance");
    return m_data.data();
}

std::size_t NetEvent::Packet::getSize() const
{
    //XY_ASSERT(m_packet, "Not a valid packet instance");
    //return m_packet->dataLength - sizeof(sf::Uint32);
    return m_size;
}

//private
void NetEvent::Packet::setPacketData(const std::uint8_t* data, std::size_t size)
{
    //truncate large packets
    size = std::min(size, m_data.size());

    if (size)
    {
        std::memcpy(&m_id, data, sizeof(sf::Uint32));
        
        size -= sizeof(sf::Uint32);
        std::memcpy(m_data.data(), data + sizeof(sf::Uint32), size);
        m_size = size;
    }
}