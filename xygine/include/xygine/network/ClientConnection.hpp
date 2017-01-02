/*********************************************************************
Matt Marchant 2014 - 2017
http://trederia.blogspot.com

xygine - Zlib license.

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

#ifndef XY_NET_CLIENT_HPP_
#define XY_NET_CLIENT_HPP_

#include <xygine/network/Config.hpp>
#include <xygine/network/AckSystem.hpp>
#include <xygine/network/FlowControl.hpp>

#include <SFML/System/Mutex.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Network/UdpSocket.hpp>

#include <atomic>

namespace xy
{
    namespace Network
    {
        /*!
        \brief Allows creating a connection to a server application running 'ServerConnection'.
        The client/server connection is maintained over UDP using challenge/response system to
        keep it alive. Packets are all ID'd and acknowledged by the remote connections so missing
        packets may be identified and resent if necessary. Using the Acknowledge ID it is also
        possible to determine packet order and discard out of date information. Connections
        allow the provision of custom packet handlers with custom IDs so that they are flexible
        enough to use in multiple applications, whilst still maintaining reliability.
        */
        class XY_EXPORT_API ClientConnection final
        {
        public:
            /*!
            \brief Custom function for handling packets received by this connection
            */
            using PacketHandler = std::function<void(PacketType, sf::Packet&, ClientConnection*)>;

            ClientConnection();
            ~ClientConnection();

            ClientConnection(const ClientConnection&) = delete;
            ClientConnection& operator = (const ClientConnection&) = delete;

            /*!
            \brief Attempt to connect using the current server settings
            \returns true if successful, else false on failure
            */
            bool connect();
            /*!
            \brief Gracefully disconnects from a server, if it is disconnected, by
            sending a notification before closing the socket.
            */
            bool disconnect();
            /*!
            \brief Updates the state of the connection.
            \param float The delta time (in seconds) since the last update)
            */
            void update(float);
            /*!
            \brief Sends a packet of data to the server
            \param retry If true will retry to send the packet if it is not acknowledged
            by the server. false by default
            \param retryCount Number of times to try resending the packet. 3 by default
            */
            bool send(sf::Packet&, bool retry = false, sf::Uint8 retryCount = 3u);
            /*!
            \brief Set the server details to attempt to connect to
            \param sf::IpAddress Address of server to connect to
            \param PortNumber Port number of server to attempt to connect to
            */
            void setServerInfo(const sf::IpAddress&, PortNumber);
            /*!
            \brief Set a custom packet handler.
            \param PacketHandler A std::function used to handle incoming packets
            received by this connection. Function signature is
            void(PacketType, sf::Packet&, ClientConnection*)
            */
            void setPacketHandler(const PacketHandler&);
            /*!
            \brief Removes the current packet handler, if it exists
            */
            void removePacketHandler();
            /*!
            \brief Returns true if this connection is connected to a server and
            is active, else returns false.
            */
            bool connected() const;
            /*!
            \brief Returns the ClientID of this connection as assigned by the server
            upon connection.
            This should be re-aquired if connection is lost and then regained
            */
            ClientID getClientID() const;
            /*!
            \brief Gets the recommended send rate based on the current connection ping.
            This optional value is returned by the flow control and returns the recommended
            send rate to prevent flooding narrow or contended connections. The value is in
            packets per second (default 30).
            */
            float getSendRate() const;
            /*!
            \brief returns the mutex belonging to this connection.
            This should be used to lock the thread when modifying data
            from the packet handler for example.
            */
            sf::Mutex& getMutex() { return m_mutex; }
            /*!
            \brief Returns the time elapsed accoring to the server
            */
            const sf::Time& getTime() const;
        private:

            sf::UdpSocket m_socket;
            sf::IpAddress m_serverIp;
            PortNumber m_serverPort;
            ClientID m_clientID;

            PacketHandler m_packetHandler;
            std::atomic_bool m_connected;

            sf::Time m_serverTime;
            sf::Time m_interpolatedServerTime;
            sf::Time m_lastHeartbeat;

            sf::Thread m_listenThread;
            sf::Mutex m_mutex;

            AckSystem m_ackSystem;
            FlowControl m_flowControl;

            struct ResendAttempt final
            {
                sf::Packet packet;
                SeqID id = -1;
                sf::Uint8 count = 3;
            };
            std::list<ResendAttempt> m_resendAttempts;
            void attemptResends();
            
            const sf::Time& getLastHeartbeat() const;
            //void setTime(const sf::Time&);

            void handlePacket(PacketType, sf::Packet&);
            void listen();
        };
    }
}
#endif //XY_NET_CLIENT_HPP_