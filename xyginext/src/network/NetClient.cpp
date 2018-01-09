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


#include "EnetClientImpl.hpp"

#include <xyginext/network/NetClient.hpp>

using namespace xy;

NetClient::NetClient()
{
    m_impl = std::make_unique<EnetClientImpl>();
}

NetClient::~NetClient()
{

}


//public
bool NetClient::create(std::size_t maxChannels, std::size_t maxClients, sf::Uint32 incoming, sf::Uint32 outgoing)
{
    return m_impl->create(maxChannels, maxClients, incoming, outgoing);
}

bool NetClient::connect(const std::string& address, sf::Uint16 port, sf::Uint32 timeout)
{
    return m_impl->connect(address, port, timeout);
}

bool NetClient::connected() const
{
    return m_impl->connected();
}

void NetClient::disconnect()
{
    m_impl->disconnect();
}

bool NetClient::pollEvent(NetEvent& evt)
{
    return m_impl->pollEvent(evt);
}

void NetClient::sendPacket(sf::Uint32 id, void* data, std::size_t size, NetFlag flags, sf::Uint8 channel)
{
    m_impl->sendPacket(id, data, size, flags, channel);
}

const NetPeer& NetClient::getPeer() const
{
    return m_impl->getPeer();
}