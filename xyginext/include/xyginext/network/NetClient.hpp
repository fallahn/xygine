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

#include "xyginext/Config.hpp"
#include <SFML/Config.hpp>

#include "xyginext/network/NetData.hpp"
#include "xyginext/network/EnetClientImpl.hpp"

#include <string>
#include <memory>

namespace xy
{
    class EnetClientImpl;

    /*!
    \brief Creates a clientside host which can be used to create
    a peer connected to a NetHost server.
    */
    class XY_EXPORT_API NetClient final
    {
    public:
        NetClient() = default;
        ~NetClient() = default;

        NetClient(const NetClient&) = delete;
        NetClient(NetClient&&) = delete;
        NetClient& operator = (const NetClient&) = delete;
        NetClient& operator = (NetClient&&) = delete;

        /*!
        brief Creates a client host ready for connection to a server.
        This must be called at least once before trying to use connect()
        \param maxChannels Maximum number of channels to create on the
        connection. This should match the number used when creating the
        NetHost instance to which this client will connect.
        \param maxClients Number of client connections this host will
        allow to a server. This is usually 1 (default).
        \param incoming Maximum incoming bandwidth allowed in bytes per second.
        \param outgoing Maximum outgoing bandwidth allow in bytes per second.
        A value of 0 sets throttling to automatic (default).
        \returns true if successful or false if something went wrong
        Calling this 2 or more times with different parameters will attempt to recreate the host.
        NOTE: this is a templated function which defaults to the ENet library implementation.
        Generally this type does not need to be specified, and is useful only when providing
        a custom netowrking implmentation.
        */
        template <typename T = EnetClientImpl>
        bool create(std::size_t maxChannels, std::size_t maxClients = 1, sf::Uint32 incoming = 0, sf::Uint32 outgoing = 0);

        /*!
        \brief Attempts to connect to a server on the given IPv4 address or host name.
        \param address Address or hostname to connect to.
        \param port The port number on which this client will attempt
        to connect to the server.
        \param timeout Number of milliseconds to wait before connection attempt times out.
        This function is blocking until the server either responds with a connection
        successful event, or the timeout is reached. The default timeout is 5 seconds,
        and should be greater than 0, which may falsely return true as the connection
        attempt will not wait for a response from the server.
        \returns true on success or false if the attempt timed out.
        */
        bool connect(const std::string& address, sf::Uint16 port, sf::Uint32 timeout = 5000);

        /*!
        \brief Returns true if connected to a NetHost and is ready to poll for events
        else returns false.
        */
        bool connected() const;

        /*!
        \brief Closes any active connections.
        This is blocking for up to 3 seconds while waiting for graceful disconnection
        before forcefully closing the connection and causing a timeout on the remote server
        */
        void disconnect();

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
        \brief Sends a packet to the server if a connection is
        established, else does nothing. NOTE: Packets are actually
        queued and not sent over the connection until the next time
        pollEvent() is called.
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
        void sendPacket(sf::Uint32 id, const T& data, NetFlag flags, sf::Uint8 channel = 0);

        /*!
        \brief Sends the given array of bytes out over the connection if it
        is active, else does nothing.
        Use this for pre-serialised data.
        \param id Unique ID for this packet
        \param data Pointer to the data to send
        \param size Size of the data, in bytes
        \param flags Used to indicated the requested reliability of packet sent
        \see NetFlag
        \param channel Stream channel over which to send the data. Lower number
        channels have higher priority, with 0 being highest.
        */
        void sendPacket(sf::Uint32 id, void* data, std::size_t size, NetFlag flags, sf::Uint8 channel = 0);

        /*!
        \brief Returns a reference to the client's peer.
        Peers are only valid when connected to a server.
        \see NetPeer
        */
        const NetPeer& getPeer() const;

    private:

        std::unique_ptr<NetClientImpl> m_impl;
    };

#include "NetClient.inl"
}
