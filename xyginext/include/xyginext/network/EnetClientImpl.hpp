/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#pragma once

//implements the NetClient interface with enet
#include "xyginext/network/NetImpl.hpp"
#include "xyginext/network/NetData.hpp"
#include "xyginext/Config.hpp"

struct _ENetHost;

namespace xy
{

    /*!
    \brief Default networking library implmentation.
    This should never be used directly.
    */
    class XY_EXPORT_API EnetClientImpl final : public NetClientImpl
    {
    public:
        EnetClientImpl();
        ~EnetClientImpl();
        EnetClientImpl(const EnetClientImpl&) = delete;
        EnetClientImpl(EnetClientImpl&&) = delete;
        EnetClientImpl& operator = (const EnetClientImpl&) = delete;
        EnetClientImpl& operator = (EnetClientImpl&&) = delete;

        bool create(std::size_t maxChannels, std::size_t maxClients, sf::Uint32 incoming, sf::Uint32 outgoing) override;
        bool connect(const std::string& address, sf::Uint16 port, sf::Uint32 timeout) override;
        bool connected() const override;
        void disconnect() override;

        bool pollEvent(NetEvent&) override;
        void sendPacket(sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) override;

        const NetPeer& getPeer() const override { return m_peer; }

    private:

        _ENetHost * m_client;
        NetPeer m_peer;
    };
}
