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


#include <xyginext/network/NetClient.hpp>

using namespace xy;

//public
bool NetClient::connect(const std::string& address, sf::Uint16 port, sf::Uint32 timeout)
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    return m_impl->connect(address, port, timeout);
}

bool NetClient::connected() const
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    return m_impl->connected();
}

void NetClient::disconnect()
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    m_impl->disconnect();
}

bool NetClient::pollEvent(NetEvent& evt)
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    return m_impl->pollEvent(evt);
}

void NetClient::sendPacket(sf::Uint32 id, void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    m_impl->sendPacket(id, data, size, flags, channel);
}

const NetPeer& NetClient::getPeer() const
{
    XY_ASSERT(m_impl, "create() has not yet been called!");
    return m_impl->getPeer();
}