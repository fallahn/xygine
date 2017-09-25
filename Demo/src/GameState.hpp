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

#ifndef DEMO_GAME_STATE_HPP_
#define DEMO_GAME_STATE_HPP_

#include <xyginext/core/State.hpp>
#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/resources/Resource.hpp>

#include <xyginext/network/NetClient.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "StateIDs.hpp"
#include "Server.hpp"
#include "SharedStateData.hpp"
#include "PlayerInput.hpp"
#include "SpriteIDs.hpp"
#include "AnimationController.hpp"

#include <array>

namespace tmx
{
    class Map;
    class Layer;
}

struct ActorEvent;
struct ClientData;

class GameState final : public xy::State
{
public:
    GameState(xy::StateStack&, xy::State::Context, SharedStateData&);

    xy::StateID stateID() const override { return StateID::Game; }

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

private:

    xy::Scene m_scene;
    xy::TextureResource m_textureResource;
    xy::FontResource m_fontResource;

    std::array<xy::Sprite, SpriteID::Count> m_sprites;
    std::array<AnimationController, SpriteID::Count> m_animationControllers;

    xy::NetClient m_client;
    GameServer m_server;
    sf::Clock m_clientTimeout;
    SharedStateData& m_sharedData;

    ClientData m_clientData;
    PlayerInput m_playerInput;

    xy::ConfigFile m_scores;

    void loadAssets();
    bool loadScene(const MapData&);
    void loadUI();

    void handlePacket(const xy::NetEvent&);
    void handleTimeout();

    sf::RenderTexture m_mapTexture;

    sf::Int32 parseObjLayer(const std::unique_ptr<tmx::Layer>&);
    sf::Int32 parseTileLayer(const std::unique_ptr<tmx::Layer>&, const tmx::Map& map);

    void spawnActor(const ActorEvent&);
    void spawnClient(const ClientData&);
    void killActor(const ActorEvent&);

    void updateUI(const InventoryUpdate&);
};

#endif //DEMO_GAME_STATE_HPP_