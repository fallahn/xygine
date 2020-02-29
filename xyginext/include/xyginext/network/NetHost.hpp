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

#pragma once

#include "xyginext/Config.hpp"
#include <SFML/Config.hpp>
#include "xyginext/network/NetData.hpp"
#include "xyginext/network/EnetHostImpl.hpp"

#include <string>
#include <memory>

namespace xy
{
    struct NetEvent;
    struct NetPeer;
    
    /*!
    \brief Creates a network host.
    Network hosts, or servers, can have multiple clients connected
    to them, via a reliable UDP stream.
    */
    class XY_EXPORT_API NetHost final
    {
    public:
        NetHost() = default;
        ~NetHost();

        NetHost(const NetHost&) = delete;
        NetHost& operator = (const NetHost&) = delete;
        NetHost(NetHost&&) = delete;
        NetHost& operator = (NetHost&&) = delete;

        /*!
        \brief Creates a host listening on the given address and port
        \param address String representing an IPv4 address in the form "x.x.x.x".
        This may be left empty to listen on any available address.
        \param port An unsigned short representing the port on which to listen
        \param maxClient Maximum number of connections to allow to the host
        \param maxChannels Maximum number of channels allowed (indexed from 0)
        \param incoming Limit the incoming bandwidth in bytes per second. 0
        is no limit (default)
        \param outgoing Limit the outgoing bandwidth in bytes per second. 0
        is no limit (default)
        \returns true if created successfully, else false.
        NOTE Although this function is a template it is generally not required
        to pass a type here, unless specifying a custom network implementation.
        This needs to be called at least once before attempting to use any of
        the other functions in this class
        */
        template <typename T = EnetHostImpl>
        bool start(const std::string& address, sf::Uint16 port, std::size_t maxClient, std::size_t maxChannels, sf::Uint32 incoming = 0, sf::Uint32 outgoing = 0);

        /*!
        \brief Stops the host, if it is running
        */
        void stop();

        /*!
        \brief Polls the connection for events.
        This must be called at least once per frame to make sure all
        received packets are parsed and pending data is sent. Any data
        received is placed in the given event object. Make sure this
        happens on both ends of the connection (NetHost and NetClient)
        - this is the most common reason communication fails.
        \returns true if there is incoming data in the buffer, else false
        */
        bool pollEvent(NetEvent&);

        /*!
        \brief Broadcasts a packet to all connected clients.
        Note that all packets are queued until the next time pollEvent() is called.
        \param id unique ID for this packet
        \param data Struct of simple data to send. Structs are serialised
        and sent out as an array of bytes - thus members such as pointers
        are effectively useless, as the pointers themselves will be sent,
        and not the data pointed to.
        \param flags Used to denote reliability of packet sending
        \see NetFlag
        \param channel Stream channel on which to send the data. Lower number
        channels have higher priority, with 0 being highest.
        */
        template <typename T>
        void broadcastPacket(std::uint8_t id, const T& data, NetFlag flags, sf::Uint8 channel = 0);

        /*!
        \brief Broadcasts the given stream of bytes to all connected clients
        Use this for pre-serialised data.
        \param id Unique ID for this packet
        \param data Pointer to the data to send
        \param size Size of the data, in bytes
        \param flags Used to indicated the requested reliability of packet sent
        \see NetFlag
        \param channel Stream channel over which to send the data. Lower number
        channels have higher priority, with 0 being highest.
        */
        void broadcastPacket(std::uint8_t id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel = 0);

        /*!
        \brief Sends a packet to the given peer if a connection is
        established, else does nothing. NOTE: Packets are actually
        queued and not sent over the connection until the next time
        pollEvent() is called.
        \param peer The peer over which to send the packet.
        \param id unique ID for this packet
        \param data Struct of simple data to send. Structs are serialised
        and sent out as an array of bytes - thus members such as pointers
        are effectively useless, as the pointers themselves will be sent,
        and not the data pointed to.
        \param flags Used to denote reliability of packet sending
        \see NetFlag
        \param channel Stream channel on which to send the data. Lower number
        channels have higher priority, with 0 being highest.
        */
        template <typename T>
        void sendPacket(const NetPeer& peer, std::uint8_t id, const T& data, NetFlag flags, sf::Uint8 channel = 0);

        /*!
        \brief Sends the given array of bytes out over the given peer if it
        is active, else does nothing.
        Use this for pre-serialised data.
        \param peer The peer over which to send the packet.
        \param id Unique ID for this packet
        \param data Pointer to the data to send
        \param size Size of the data, in bytes
        \param flags Used to indicated the requested reliability of packet sent
        \see NetFlag
        \param channel Stream channel over which to send the data. Lower number
        channels have higher priority, with 0 being highest.
        */
        void sendPacket(const NetPeer& peer, std::uint8_t id, const void* data, std::size_t size, NetFlag flags, sf::Uint8 channel = 0);


        /*!
        \brief Returns the number of currently connected peers
        */
        std::size_t getConnectedPeerCount() const;

        /*
        \brief Returns a Uint32 containing the host's IP address
        in network byte order if it is running, else returns 0
        */
        std::uint32_t getAddress() const;

        /*
        \brief Returns a Uint16 containing the port of the host
        if it is running, else returns 0
        */
        std::uint16_t getPort() const;

    private:
        std::unique_ptr<NetHostImpl> m_impl;
    };

#include "NetHost.inl"
}
