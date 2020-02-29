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
inline bool NetClient::create(std::size_t maxChannels, std::size_t maxClients, sf::Uint32 incoming, sf::Uint32 outgoing)
{
    static_assert(std::is_base_of<NetClientImpl, T>(), "");
    m_impl = std::make_unique<T>();
    return m_impl->create(maxChannels, maxClients, incoming, outgoing);
}

template <typename T>
void NetClient::sendPacket(std::uint8_t id, const T& data, NetFlag flags, sf::Uint8 channel)
{
    m_impl->sendPacket(id, (void*)&data, sizeof(T), flags, channel);
}