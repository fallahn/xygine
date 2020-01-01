/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/network/NetHost.hpp"

using namespace xy;

NetHost::~NetHost()
{
    if(m_impl) m_impl->stop();
}

//public
void NetHost::stop()
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    m_impl->stop();
}

bool NetHost::pollEvent(NetEvent& evt)
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    return m_impl->pollEvent(evt);
}

void NetHost::broadcastPacket(sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    m_impl->broadcastPacket(id, data, size, flags, channel);
}

void NetHost::sendPacket(const NetPeer& peer, sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    m_impl->sendPacket(peer, id, data, size, flags, channel);
}
std::size_t NetHost::getConnectedPeerCount() const
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    return m_impl->getConnectedPeerCount();
}

std::uint32_t NetHost::getAddress() const
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    return m_impl->getAddress();
}

std::uint16_t NetHost::getPort() const
{
    XY_ASSERT(m_impl, "start() has not yet been called!");
    return m_impl->getPort();
}
