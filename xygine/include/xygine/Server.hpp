/*********************************************************************
Matt Marchant 2014 - 2015
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

//game server class

#ifndef XY_SERVER_HPP_
#define XY_SERVER_HPP_

#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Vector2.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <queue>

namespace xy
{
    class GameServer final
    {
    public:
        GameServer();
        ~GameServer();

        GameServer(const GameServer&) = delete;
        GameServer& operator = (const GameServer&) = delete;

        static sf::Uint16 getPort();
        void setPort(sf::Uint16);

        void setMaxPlayers(sf::Uint32);

    private:
        struct RemoteConnection
        {
            using Ptr = std::unique_ptr<RemoteConnection>;
            RemoteConnection();

            sf::TcpSocket socket;
            float lastPacketTime;
            bool ready;
            bool timeout;
            sf::Int16 uid;
        };

        sf::Thread m_thread;
        sf::Clock m_clock;
        sf::TcpListener m_listener;
        bool m_listening;
        float m_clientTimeoutTime;
        sf::Uint32 m_maxPlayers;
        sf::Uint32 m_connectedPlayers;
        std::vector<RemoteConnection::Ptr> m_connections;
        bool m_waitingThreadEnd;
        sf::Mutex m_mutex;

        MessageBus m_messageBus;
        Scene m_scene;

        void setListening(bool);
        void executionThread();
        void update(float);
        void tick();
        float now() const;

        void handlePackets();
        void handlePacket(sf::Packet&, RemoteConnection&, bool&);

        void createConnection();
        void handleConnections();
        void handleDisconnections();

        void sendToAll(sf::Packet&);
        void pingClients();

        void handleMessage(const Message&);

        std::function<void()> updateClientState;
        void updateClientGameState();
        void updateClientLobbyState();
    };
}
#endif //XY_SERVER_HPP_