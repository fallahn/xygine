/*********************************************************************
Matt Marchant 2014 - 2016
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

#ifndef NETDEMO_STATE_HPP_
#define NETDEMO_STATE_HPP_

#include <xygine/ui/Container.hpp>
#include <xygine/Resource.hpp>
#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/network/ClientConnection.hpp>

#include <StateIds.hpp>
#include <NetworkDemoServer.hpp>
#include <NetworkDemoPlayerInput.hpp>
#include <NetworkDemoCollisions.hpp>

#include <SFML/Graphics/Text.hpp>

class NetworkDemoState final : public xy::State
{
public:
    NetworkDemoState(xy::StateStack&, Context);
    ~NetworkDemoState() = default;


    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateId stateID() const { return States::NetworkDemo; }

private:

    struct PlayerInfo final
    {
        sf::Uint64 entID = 0;
        std::string name;
        sf::Uint32 score = 0;
        xy::ClientID clid = -1;
    };
    std::array<PlayerInfo, 2u> m_players; //local player always first

    Server m_server;
    xy::Network::ClientConnection m_connection;
    xy::Network::ClientConnection::PacketHandler m_packetHandler;

    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;

    sf::Text m_reportText;
    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;    
    CollisionWorld m_collisionWorld;

    NetDemo::Input m_playerInput;

    std::vector<sf::Uint64> m_spawnedIDs;

    xy::UI::Container m_waitingSign;
    xy::UI::Container m_menu;
    void buildMenu();
    void handlePacket(xy::Network::PacketType, sf::Packet&, xy::Network::ClientConnection*);

    void spawnBall(sf::Uint64 id, sf::Vector2f position, sf::Vector2f velocity);
    void spawnPlayer(xy::ClientID, sf::Uint64, sf::Vector2f);
};

#endif //NETDEMO_STATE_HPP_