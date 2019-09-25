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

#include "xyginext/network/NetImpl.hpp"
#include "xyginext/Config.hpp"

struct _ENetHost;

namespace xy
{
    /*!
    \brief Default implementation of the networking library.
    This should never be used directly.
    */
    class XY_EXPORT_API EnetHostImpl final : public NetHostImpl
    {
    public:
        EnetHostImpl();
        ~EnetHostImpl();
        EnetHostImpl(const EnetHostImpl&) = delete;
        EnetHostImpl(EnetHostImpl&&) = delete;
        EnetHostImpl& operator = (const EnetHostImpl&) = delete;
        EnetHostImpl& operator = (EnetHostImpl&&) = delete;

        bool start(const std::string& address, sf::Uint16 port, std::size_t maxClient, std::size_t maxChannels, sf::Uint32 incoming, sf::Uint32 outgoing) override;
        void stop() override;
        bool pollEvent(NetEvent&) override;
        void broadcastPacket(sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) override;
        void sendPacket(const NetPeer& peer, sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) override;

        std::size_t getConnectedPeerCount() const override;
        std::uint32_t getAddress() const override;
        std::uint16_t getPort() const override;

    private:
        _ENetHost * m_host;
    };
}
