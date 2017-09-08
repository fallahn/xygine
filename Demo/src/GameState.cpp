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

#include "GameState.hpp"
#include "PacketIDs.hpp"
#include "MapData.hpp"
#include "CommandIDs.hpp"
#include "PlayerSystem.hpp"
#include "ServerMessages.hpp"

#include <xyginext/core/App.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/NetInterpolation.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/InterpolationSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/graphics/postprocess/ChromeAb.hpp>

#include <xyginext/network/NetData.hpp>
#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

namespace
{
    struct TestPacket
    {
        int a = 2;
        float z = 54.f;
    };

    const float clientTimeout = 20.f;

    enum MapFlags
    {
        Solid = 0x1,
        Platform = 0x2,
        Graphics = 0x4
    };
}

GameState::GameState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData)
    : xy::State     (stack, ctx),
    m_scene         (ctx.appInstance.getMessageBus()),
    m_sharedData    (sharedData),
    m_playerInput   (m_client)
{
    launchLoadingScreen();
    loadAssets();
    m_client.create(2);
    if (sharedData.hostState == SharedStateData::Host)
    {
        sf::Clock joinTimer;
        m_server.start();
        while (!m_server.ready() && joinTimer.getElapsedTime().asSeconds() < 8.f) {}
        m_client.connect("localhost", 40003);
    }
    else
    {
        m_client.connect(sharedData.remoteIP, 40003);
    }

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());

    //TODO replace this
    m_timeoutText.setFillColor(sf::Color::Red);
    m_timeoutText.setFont(m_fontResource.get("buns"));
    quitLoadingScreen();
}

//public
bool GameState::handleEvent(const sf::Event& evt)
{
    m_playerInput.handleEvent(evt);
    m_scene.forwardEvent(evt);

    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Insert:
            m_client.disconnect();
            break;
        case sf::Keyboard::Home:
            m_server.stop();
            break;
        }
    }
    return false;
}

void GameState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool GameState::update(float dt)
{   
    xy::NetEvent evt;
    while (m_client.pollEvent(evt))
    {
        if (evt.type == xy::NetEvent::PacketReceived)
        {
            handlePacket(evt);
            m_clientTimeout.restart();
        }
    }
    handleTimeout();
    
    m_playerInput.update();
    m_scene.update(dt);
    return false;
}

void GameState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.draw(m_timeoutText);
}

//private
void GameState::loadAssets()
{
    auto& mb = getContext().appInstance.getMessageBus();

    m_scene.addSystem<PlayerSystem>(mb);
    m_scene.addSystem<xy::InterpolationSystem>(mb);
    m_scene.addSystem<xy::AudioSystem>(mb);
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::CameraSystem>(mb);
    m_scene.addSystem<xy::CommandSystem>(mb);
    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    
    m_scene.addPostProcess<xy::PostChromeAb>();

    //preload textures
    m_textureResource.get("assets/images/bubble.png");
    m_textureResource.get("assets/images/target.png");

    //audio
    //m_soundResource.get("assets/boop_loop.wav");
}

void GameState::loadScene(const MapData& data)
{
    tmx::Map map;
    if (!map.load("assets/maps/" + std::string(data.mapName)))
    {
        //TODO disconnect from server and push disconnected state
    }

    //TODO crc or something on map file to compare with server's
    auto size = map.getTileCount() * map.getTileSize();

    m_mapTexture.create(size.x, size.y);
    m_mapTexture.clear(sf::Color::Red);
    sf::Uint8 flags = 0;

    const auto& layers = map.getLayers();
    for (const auto& layer : layers)
    {
        if (layer->getType() == tmx::Layer::Type::Object)
        {
            //create map collision
            flags |= parseObjLayer(layer);
        }
        else if (layer->getType() == tmx::Layer::Type::Tile)
        {
            //create map drawable
            flags |= parseTileLayer(layer, map);
        }
    }
    m_mapTexture.display();
    
    if (flags != (Solid | Platform | Graphics))
    {
        //TODO disconnect and bail
    }

    //create the background sprite
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_mapTexture.getTexture());
    entity.addComponent<xy::Transform>().setPosition((xy::DefaultSceneSize.x - static_cast<float>(size.x)) / 2.f, xy::DefaultSceneSize.y - static_cast<float>(size.y));

    for (auto i = 0; i < data.actorCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>();
        entity.addComponent<Actor>() = data.actors[i];
        entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/bubble.png"));
        auto bounds = entity.getComponent<xy::Sprite>().getLocalBounds();
        bounds.width /= 2.f;
        if (xy::Util::Random::value(0, 1) == 1)
        {
            bounds.left += bounds.width;
        }
        entity.getComponent<xy::Sprite>().setTextureRect(bounds);
        entity.getComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
        entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor;
        entity.addComponent<xy::NetInterpolate>();
    }
}

void GameState::handlePacket(const xy::NetEvent& evt)
{
    switch (evt.packet.getID())
    {
    default: break;
    case PacketID::ServerMessage:
    {
        sf::Int32 idx = evt.packet.as<sf::Int32>();
        xy::Logger::log(serverMessages[idx], xy::Logger::Type::Info);
    }
        break;
    case PacketID::ActorAbsolute:
        //set absolute state of actor
    {
        const auto& state = evt.packet.as<ActorState>();

        xy::Command cmd;
        cmd.targetFlags = CommandID::NetActor;
        cmd.action = [state](xy::Entity entity, float)
        {
            if (entity.getComponent<Actor>().id == state.actor.id)
            {
                entity.getComponent<xy::Transform>().setPosition(state.x, state.y);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::ActorUpdate:
        //do actor interpolation
    {
        const auto& state = evt.packet.as<ActorState>();

        xy::Command cmd;
        cmd.targetFlags = CommandID::NetActor;
        cmd.action = [state, this](xy::Entity entity, float)
        {
            if (entity.getComponent<Actor>().id == state.actor.id)
            {
                entity.getComponent<xy::NetInterpolate>().setTarget({ state.x, state.y }, state.serverTime);
                //DPRINT("Timestamp", std::to_string(state.timestamp));
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::MapData:
    {
        MapData data = evt.packet.as<MapData>();

        //TODO clear old actors
        //load new actors
        loadScene(data);

        //send ready signal
        m_client.sendPacket(PacketID::ClientReady, 0, xy::NetFlag::Reliable, 1);
    }
        break;
    case PacketID::ClientUpdate:
    {
        const auto& state = evt.packet.as<ActorState>();

        //reconcile
        m_scene.getSystem<PlayerSystem>().reconcile(state.x, state.y, state.clientTime, m_playerInput.getPlayerEntity());
    }
        break;
    case PacketID::ClientData:
    {
        ClientData data = evt.packet.as<ClientData>();
        
        //create the local ent
        //set sprite based on actor type (player one or two)
        auto entity = m_scene.createEntity();
        entity.addComponent<Actor>() = data.actor;
        entity.addComponent<xy::Transform>().setPosition(data.spawnX, data.spawnY);

        xy::SpriteSheet spritesheet;
        spritesheet.loadFromFile("assets/sprites/player.spt", m_textureResource);

        if (data.actor.type == ActorID::PlayerOne)
        {
            entity.addComponent<xy::Sprite>() = spritesheet.getSprite("player_one");
        }
        else
        {
            entity.addComponent<xy::Sprite>() = spritesheet.getSprite("player_two");
        }
        entity.getComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
        entity.addComponent<xy::SpriteAnimation>().play(0);

        if (data.peerID == m_client.getPeer().getID())
        {
            //this is us, stash the info
            m_clientData = data;

            //add a local controller
            entity.addComponent<Player>().playerNumber = (data.actor.type == ActorID::PlayerOne) ? 0 : 1;
            m_playerInput.setPlayerEntity(entity);
        }
        else
        {
            //add interp controller
            entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor;
            entity.addComponent<xy::NetInterpolate>();
        }
    }
        break;
    case PacketID::ClientDisconnected:
    {
        const auto& data = evt.packet.as<ClientData>();
        auto actorID = data.actor.id;

        //scrub the client from the scene
        xy::Command cmd;
        cmd.targetFlags = CommandID::NetActor;
        cmd.action = [&, actorID](xy::Entity entity, float)
        {
            const auto& actor = entity.getComponent<Actor>();
            if (actor.id == actorID)
            {
                m_scene.destroyEntity(entity);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    }
}

void GameState::handleTimeout()
{
    float currTime = m_clientTimeout.getElapsedTime().asSeconds();
    if (currTime > clientTimeout / 5.f)
    {
        m_timeoutText.setString("WARNING: Connection Problem\nAuto Disconnect in: " + std::to_string(clientTimeout - currTime));
    }
    
    if (currTime > clientTimeout)
    {
        //push a message state
        m_sharedData.error = "Disconnected from server.";
        requestStackPush(StateID::Error);
    }
}

sf::Int32 GameState::parseObjLayer(const std::unique_ptr<tmx::Layer>& layer)
{
    auto name = layer->getName();
    if (name == "platform")
    {
        return Platform;
    }
    else if (name == "solid")
    {
        return Solid;
    }
    return 0;
}

sf::Int32 GameState::parseTileLayer(const std::unique_ptr<tmx::Layer>& layer, const tmx::Map& map)
{
    const auto& tilesets = map.getTilesets();
    std::vector<std::unique_ptr<sf::Texture>> textures(tilesets.size());
    for (auto i = 0u; i < tilesets.size(); ++i)
    {
        textures[i] = std::make_unique<sf::Texture>();
        if (!textures[i]->loadFromFile(tilesets[i].getImagePath()))
        {
            return 0;
        }
    }
    
    const auto& tiles = dynamic_cast<tmx::TileLayer*>(layer.get())->getTiles();
    std::vector<std::pair<sf::Texture*, std::vector<sf::Vertex>>> vertexArrays;
    
    const auto tileCount = map.getTileCount();
    const sf::Vector2f tileSize = { static_cast<float>(map.getTileSize().x), static_cast<float>(map.getTileSize().y) };
    //this assumes starting in top left - we ought to check the map property really
    for (auto y = 0u; y < tileCount.y; ++y)
    {
        for (auto x = 0u; x < tileCount.x; ++x)
        {
            auto idx = y * tileCount.x + x;
            if (tiles[idx].ID > 0)
            {
                //create the vertices
                std::array<sf::Vertex, 4u> verts;
                verts[0].position = { x * tileSize.x, y * tileSize.y };
                verts[1].position = { verts[0].position.x + tileSize.x, verts[0].position.y };
                verts[2].position = verts[0].position + tileSize;
                verts[3].position = { verts[0].position.x, verts[0].position.y + tileSize.y };

                std::size_t i = 0;
                sf::Texture* currTexture = nullptr;
                for (; i < tilesets.size(); ++i)
                {
                    if (tiles[idx].ID >= tilesets[i].getFirstGID() && tiles[idx].ID <= tilesets[i].getLastGID())
                    {
                        //get the texcoords
                        auto tileIdx = tiles[idx].ID - tilesets[i].getFirstGID(); //tile relative to first in set
                        auto tileX = tileIdx % tilesets[i].getColumnCount();
                        auto tileY = tileIdx / tilesets[i].getColumnCount();

                        verts[0].texCoords = { tileX * tileSize.x, tileY * tileSize.y };
                        verts[1].texCoords = { verts[0].texCoords.x + tileSize.x, verts[0].texCoords.y };
                        verts[2].texCoords = verts[0].texCoords + tileSize;
                        verts[3].texCoords = { verts[0].texCoords.x, verts[0].texCoords.y + tileSize.y };

                        //and ref to the texture
                        currTexture = textures[i].get();
                        break;
                    }
                }

                XY_ASSERT(currTexture, "Something went wrong loading texture!");

                //find which vertex array they belong and add if not yet existing
                auto result = std::find_if(vertexArrays.begin(), vertexArrays.end(),
                    [currTexture](const std::pair<sf::Texture*, std::vector<sf::Vertex>>& v)
                {
                    return v.first == currTexture;
                });

                if (result != vertexArrays.end())
                {
                    //add to existing
                    for (const auto& vertex : verts)
                    {
                        result->second.push_back(vertex);
                    }
                }
                else
                {
                    //create new array
                    vertexArrays.emplace_back(std::make_pair(currTexture, std::vector<sf::Vertex>()));
                    for (const auto& vertex : verts)
                    {
                        vertexArrays.back().second.push_back(vertex);
                    }
                }
            }
        }
    }


    for (const auto& v : vertexArrays)
    {
        m_mapTexture.draw(v.second.data(), v.second.size(), sf::Quads, v.first);
    }

    return Graphics;
}