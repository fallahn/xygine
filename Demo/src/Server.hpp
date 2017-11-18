/*********************************************************************
(c) Matt Marchant 2017
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

#ifndef GAME_DEMO_SERVER_HPP_
#define GAME_DEMO_SERVER_HPP_

#include "MapData.hpp"

#include <xyginext/network/NetHost.hpp>
#include <xyginext/core/MessageBus.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <SFML/System/Thread.hpp>

#include <atomic>
#include <array>

class GameServer final
{
public:
    GameServer();
    ~GameServer();
    GameServer(const GameServer&) = delete;
    GameServer(GameServer&&) = delete;
    GameServer& operator = (const GameServer&) = delete;
    GameServer& operator = (GameServer&&) = delete;

    void start();
    void stop();

    bool ready() { return m_ready; }

private:
    xy::NetHost m_host;
    std::atomic<bool> m_ready;

    std::atomic<bool> m_running;
    sf::Thread m_thread;
    void update();

    sf::Clock m_serverTime;

    void handleConnect(const xy::NetEvent&);
    void handleDisconnect(const xy::NetEvent&);

    void handlePacket(const xy::NetEvent&);

    xy::MessageBus m_messageBus;
    xy::Scene m_scene;
    MapData m_mapData;
    sf::Uint32 m_mapSkipCount;

    std::vector<std::string> m_mapFiles;
    std::size_t m_currentMap;
    float m_endOfRoundPauseTime;
    float m_currentRoundTime;
    float m_roundTimeout;
    void checkRoundTime(float);
    void checkMapStatus(float);


    std::unique_ptr<xy::NetPeer> m_queuedClient;

    enum StateFlags
    {
        GameOver,
        ChangingMaps,
        Paused,
        PendingPause,
        Count
    };
    std::bitset<StateFlags::Count> m_stateFlags;
    static std::bitset<StateFlags::Count> GameOverOrPaused;

    void initMaplist();
    void initScene();
    void loadMap();
    void beginNewRound();

    sf::Int32 spawnPlayer(std::size_t);

    xy::Entity spawnNPC(sf::Int32, sf::Vector2f);
    void spawnCrate(sf::Vector2f, sf::Uint8);

    void handleMessage(const xy::Message&);

    struct Client final
    {
        ClientData data;
        xy::NetPeer peer;
        bool ready = false;
        sf::Uint8 level = 1;
    };
    std::array<Client, 2u> m_clients;
};

#endif //GAME_DEMO_SERVER_HPP_