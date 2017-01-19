/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_SERVER_HPP_
#define XY_SERVER_HPP_

#include <xygine/network/Config.hpp>
#include <xygine/network/AckSystem.hpp>

#include <SFML/System/Time.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Thread.hpp>

#include <SFML/Network/UdpSocket.hpp>

#include <atomic>
#include <memory>

namespace xy
{
    class MessageBus;
    namespace Network
    {
        /*!
        \brief Maintains a group of client connections.
        The server connection can handle multiple connections from the ClientConnection class,
        with a configurable maximum number. Connections are maintained via a heartbeat which
        detects and removes any expired clients, and support acknowledgement packets which
        can detect packet loss and optionally resend import lost packets.
        */
        class XY_EXPORT_API ServerConnection final
        {
        public:
            /*!
            \brief Contains information on connected client
            */
            struct ClientInfo final
            {
                friend class ServerConnection;
            public:
                ClientInfo() = default;
                ClientInfo(const sf::IpAddress& ip, PortNumber port, const sf::Time& heartBeat)
                    : ipAddress(ip),
                    portNumber(port),
                    lastHeartbeat(heartBeat)
                {
                    ackSystem = std::make_unique<Network::AckSystem>();
                }

                sf::IpAddress ipAddress;
                PortNumber portNumber = 0u;
                float ping() const { return ackSystem->getRoundTripTime(); }
            private:
                sf::Time lastHeartbeat;
                sf::Time heartbeatSent;
                bool heartbeatWaiting = false;
                sf::Uint16 heartbeatRetry = 0u;

                std::unique_ptr<Network::AckSystem> ackSystem;

                struct ResendAttempt final
                {
                    sf::Packet packet;
                    SeqID id = -1;
                    sf::Uint8 count = 3;
                };
                std::list<ResendAttempt> resendAttempts;
                void attemptResends(ClientID, ServerConnection&);
            };


            /*!
            \brief Custom packet handler for received packets
            */
            using PacketHandler = std::function<void(const sf::IpAddress&, PortNumber, Network::PacketType, sf::Packet&, ServerConnection*)>;
            /*!
            \brief Custom disconnect handler for updating the server when a client times out
            */
            using TimeoutHandler = std::function<void(ClientID)>;

            explicit ServerConnection(MessageBus&);
            ~ServerConnection();

            ServerConnection(const ServerConnection&) = delete;
            ServerConnection& operator = (const ServerConnection&) = delete;

            /*!
            \brief Set the custom packet handler.
            Allows flexible handling of custom packets by providing the connection
            with a custom packet handler.
            */
            void setPacketHandler(const PacketHandler&);
            /*!
            \brief Set the custom timeout handler.
            Allows flexible handling of clients which have timed out and disconnected from the server
            */
            void setTimeoutHandler(const TimeoutHandler&);
            /*!
            \brief Sends a packet to a client
            \param ClientID ID of client to send the packet to
            \param Packet Packet of data to send
            \param retry Retries sending this packet if the client connection doesn't acknowledge it when true
            \param retryCount Number of times to retry sending the packet before it is considered lost
            */
            bool send(ClientID, sf::Packet&, bool retry = false, sf::Uint8 retryCount = 3u);
            /*!
            \brief Sends a packet to a client
            \param IpAddress Address of the client to send the packet to
            \param PortNumber Port number of the client connection to send the packet to
            \param Packet Packet of data to send
            \param retry Retries sending this packet if the client connection doesn't acknowledge it when true
            \param retryCount Number of times to retry sending the packet before it is considered lost
            */
            bool send(const sf::IpAddress&, PortNumber, sf::Packet&, bool retry = false, sf::Uint8 retryCount = 3u);
            /*!
            \brief Sends a packet to all connected clients
            \param Packet Packet of data to send
            \param retry Retries sending this packet if the client connection doesn't acknowledge it when true
            \param retryCount Number of times to retry sending the packet before it is considered lost
            */
            void broadcast(sf::Packet&, bool retry = false, sf::Uint8 retryCount = 3u, ClientID ignore = Network::NullID);
            /*!
            \brief Adds a new client at the given address and port number
            \returns ClientID is the new ID of the connected client
            */
            ClientID addClient(const sf::IpAddress&, PortNumber);
            /*!
            \brief Looks up the ClientID associated with the give IP address and port
            \returns Returns the ClientID if found, else -1
            */
            ClientID getClientID(const sf::IpAddress&, PortNumber);
            /*!
            \brief Returns true if the given ClientID is found connected to the server else
            returns false
            */
            bool hasClient(ClientID) const;
            /*!
            \brief Returns true if a client is found connected with the given IP address
            and port number else returns false
            */
            bool hasClient(const sf::IpAddress&, PortNumber);
            /*!
            \brief Requests the ClientInfo data of the given ClientID
            \param ClientID ID of the client to request the info for
            \param ClientInfo pointer to struct to receive info if it is found
            \returns true if client is found and ClientInfo is valid else returns false
            */
            bool getClientInfo(ClientID, const ClientInfo*);
            /*!
            \brief Removes the client with the given ID
            \returns true if successful, else returns false
            */
            bool removeClient(ClientID);
            /*!
            \brief Removes  the client matching the given IP address and port number
            \returns true if successful else returns false
            */
            bool removeClient(const sf::IpAddress&, PortNumber);
            /*!
            \brief Disconnects all connected clients
            */
            void disconnectAll();
            /*!
            \brief Starts the connection listening for incoming client connections
            \param PortNumber Port to start listening for incoming connections on.
            */
            bool start(PortNumber = sf::Uint16(Network::ServerPort));
            /*!
            \brief Stops the connection listening for incoming client connections
            and disconnects all existing clients
            */
            bool stop();
            /*!
            \brief Updates the connection
            \param float Delta time in seconds since last update
            */
            void update(float);
            /*!
            \brief Returns true if connection is listening else returns false
            */
            bool running() const;
            /*!
            \brief Returns the number of currently connected clients
            */
            std::size_t getClientCount() const;
            /*!
            \brief Sets the maximum number of connected clients.
            Any connection reuests made after this number is met will be rejected
            */
            void setMaxClients(std::size_t);
            /*!
            \brief Returns the current number of client connections allowed to the server
            */
            std::size_t getMaxClients() const;
            /*!
            \brief Returns the mutex used by the connection's packet handler thread.
            This should be used in custom packet handlers when modifying data outside
            of the handler's thread.
            */
            sf::Mutex& getMutex() { return m_mutex; }

        private:

            using ClientList = std::unordered_map<ClientID, ClientInfo>;

            ClientID m_lastClientID;
            std::size_t m_maxClients;

            sf::UdpSocket m_incomingSocket;
            sf::UdpSocket m_outgoingSocket;

            PacketHandler m_packetHandler;
            TimeoutHandler m_timoutHandler;

            ClientList m_clients;
            sf::Time m_serverTime;

            std::atomic_bool m_running;

            sf::Thread m_listenThread;
            sf::Mutex m_mutex;

            std::atomic<size_t> m_totalBytesSent;
            std::atomic<size_t> m_totalBytesReceived;

            xy::MessageBus& m_messageBus;

            void listen();

            void init();
            void handlePacket(const sf::IpAddress&, PortNumber, Network::PacketType, sf::Packet&);
        };
    }
}
#endif //XY_SERVER_HPP_
