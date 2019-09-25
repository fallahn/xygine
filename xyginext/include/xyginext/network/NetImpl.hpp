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

#include <SFML/Config.hpp>
#include "xyginext/Config.hpp"

#include <cstddef>
#include <string>

/*!
/brief Interface for networking implementations.
By default the networking classes wrap the enet library, although in some
cases it may be desirable to use 3rd party networking such as that available
in the Steam API. This interface is designed to allow flexible switching
of services. For NetClient and NetHost the type can be provided by the
create<T>() and start<T>() functions respectively.
*/

namespace xy
{
    struct NetEvent;
    struct NetPeer;

    /*!
    \brief Reliability enum.
    These are used to flag sent packets with a requested reliability.
    */
    enum class NetFlag
    {
        Reliable = 0x1, //! <packet must be received by the remote connection, and resend attemps are made until delivered
        Unsequenced = 0x2, //! <packet will not be sequenced with other packets. Not supported on reliable packets
        Unreliable = 0x4 //! <packet will be fragmented and sent unreliably if it exceeds MTU
    };

    class XY_EXPORT_API NetClientImpl
    {
    public:
        NetClientImpl() = default;
        virtual ~NetClientImpl() = default;

        NetClientImpl(const NetClientImpl&) = delete;
        NetClientImpl(NetClientImpl&&) = delete;
        NetClientImpl& operator = (const NetClientImpl&) = delete;
        NetClientImpl& operator = (NetClientImpl&&) = delete;

        virtual bool create(std::size_t maxChannels, std::size_t maxClients, sf::Uint32 incoming, sf::Uint32 outgoing) = 0;
        virtual bool connect(const std::string& address, sf::Uint16 port, sf::Uint32 timeout) = 0;
        virtual bool connected() const = 0;
        virtual void disconnect() = 0;

        virtual bool pollEvent(NetEvent&) = 0;
        virtual void sendPacket(sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) = 0;

        virtual const NetPeer& getPeer() const = 0;
    };

    class XY_EXPORT_API NetHostImpl
    {
    public:
        NetHostImpl() = default;
        virtual ~NetHostImpl() = default;

        NetHostImpl(const NetHostImpl&) = delete;
        NetHostImpl(NetHostImpl&&) = delete;
        NetHostImpl& operator = (const NetHostImpl&) = delete;
        NetHostImpl& operator = (NetHostImpl&&) = delete;

        virtual bool start(const std::string& address, sf::Uint16 port, std::size_t maxClient, std::size_t maxChannels, sf::Uint32 incoming, sf::Uint32 outgoing) = 0;
        virtual void stop() = 0;
        virtual bool pollEvent(NetEvent&) = 0;
        virtual void broadcastPacket(sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) = 0;
        virtual void sendPacket(const NetPeer& peer, sf::Uint32 id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel) = 0;

        virtual std::size_t getConnectedPeerCount() const = 0;
        virtual std::uint32_t getAddress() const { return 0; }
        virtual std::uint16_t getPort() const { return 0; }
    };
}
