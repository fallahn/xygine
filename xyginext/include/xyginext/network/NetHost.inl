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

template <typename T>
bool NetHost::start(const std::string& address, std::uint16_t port, std::size_t maxClients, std::size_t maxChannels, std::uint32_t incoming, std::uint32_t outgoing)
{
    static_assert(std::is_base_of<NetHostImpl, T>(), "");
    m_impl = std::make_unique<T>();
    return m_impl->start(address, port, maxClients, maxChannels, incoming, outgoing);
}

template <typename T>
void NetHost::broadcastPacket(std::uint8_t id, const T& data, NetFlag flags, std::uint8_t channel)
{
    m_impl->broadcastPacket(id, (void*)&data, sizeof(T), flags, channel);
}

template <typename T>
void NetHost::sendPacket(const NetPeer& peer, std::uint8_t id, const T& data, NetFlag flags, std::uint8_t channel)
{
    m_impl->sendPacket(peer, id, (void*)&data, sizeof(T), flags, channel);
}