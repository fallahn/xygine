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
#include "ClientServerShared.hpp"
#include "CollisionSystem.hpp"
#include "sha1.hpp"
#include "SpriteIDs.hpp"
#include "MessageIDs.hpp"
#include "ParticleDirector.hpp"
#include "FXDirector.hpp"
#include "MapAnimator.hpp"
#include "ScoreTag.hpp"

#include <xyginext/core/App.hpp>
#include <xyginext/core/FileSystem.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/NetInterpolation.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/AudioListener.hpp>

#include <xyginext/ecs/systems/SpriteRenderer.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/InterpolationSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CameraSystem.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/graphics/postprocess/ChromeAb.hpp>

#include <xyginext/network/NetData.hpp>
#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>

namespace
{
    const float clientTimeout = 20.f;
    const std::string scoreFile("scores.txt");

    //flashes text
    class Flasher final
    {   
    private:
        static constexpr float flashTime = 0.25f;

    public:
        Flasher(xy::Scene& scene) : m_scene(scene) {}
        void operator ()(xy::Entity entity, float dt)
        {
            m_flashTime -= dt;

            if (m_flashTime < 0)
            {
                m_flashTime = flashTime;
                m_colour.a = (m_colour.a == 255) ? 0 : 255;
                entity.getComponent<xy::Text>().setFillColour(m_colour);
            }

            auto& tx = entity.getComponent<xy::Transform>();
            tx.move(-600.f * dt, 0.f);
            if (tx.getPosition().x < -1000.f)
            {
                m_scene.destroyEntity(entity);

                xy::Command cmd;
                cmd.targetFlags = CommandID::Music;
                cmd.action = [](xy::Entity entity, float)
                {
                    entity.getComponent<xy::AudioEmitter>().play();
                    entity.getComponent<xy::AudioEmitter>().setPitch(1.1f);
                };
                m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
            }
        }
    private:
        xy::Scene& m_scene;
        float m_flashTime = flashTime;
        sf::Color m_colour = sf::Color::Red;   
    };
}

GameState::GameState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData)
    : xy::State         (stack, ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_sharedData        (sharedData),
    m_playerInput       (m_client),
    m_currentMapTexture (0)
{
    launchLoadingScreen();
    loadAssets();
    loadUI();
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
        /*case sf::Keyboard::Insert:
            m_client.disconnect();
            break;
        case sf::Keyboard::Home:
            m_server.stop();
            break;*/
        case sf::Keyboard::P:
        case sf::Keyboard::Escape:
        case sf::Keyboard::Pause:
            requestStackPush(StateID::Pause);
            break;
        }
    }
    return false;
}

void GameState::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::MapMessage)
    {
        const auto& data = msg.getData<MapEvent>();
        if (data.type == MapEvent::AnimationComplete)
        {
            spawnMapActors();
            
            m_playerInput.setEnabled(true);
            m_scene.getSystem<CollisionSystem>().setEnabled(true);

            //send OK to server to continue game
            m_client.sendPacket(PacketID::MapReady, m_clientData.actor.type, xy::NetFlag::Reliable, 1);
        }
    }

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
}

//private
void GameState::loadAssets()
{
    auto& mb = getContext().appInstance.getMessageBus();

    m_scene.addSystem<xy::QuadTree>(mb, MapBounds);
    m_scene.addSystem<CollisionSystem>(mb);
    m_scene.addSystem<PlayerSystem>(mb);   
    m_scene.addSystem<xy::InterpolationSystem>(mb);
    m_scene.addSystem<AnimationControllerSystem>(mb);
    m_scene.addSystem<MapAnimatorSystem>(mb);
    m_scene.addSystem<ScoreTagSystem>(mb);
    m_scene.addSystem<xy::AudioSystem>(mb);
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::CameraSystem>(mb);
    m_scene.addSystem<xy::CommandSystem>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteRenderer>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    m_scene.addSystem<xy::AudioSystem>(mb);
    
    //m_scene.addPostProcess<xy::PostChromeAb>();
    m_scene.addDirector<ParticleDirector>(m_textureResource);
    m_scene.addDirector<FXDirector>();

    //preload textures
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/bubble.spt", m_textureResource);
    m_sprites[SpriteID::BubbleOne] = spriteSheet.getSprite("player_one");
    m_sprites[SpriteID::BubbleTwo] = spriteSheet.getSprite("player_two");

    spriteSheet.loadFromFile("assets/sprites/player.spt", m_textureResource);
    m_sprites[SpriteID::PlayerOne] = spriteSheet.getSprite("player_one");
    m_sprites[SpriteID::PlayerTwo] = spriteSheet.getSprite("player_two");

    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Shoot] = spriteSheet.getAnimationIndex("shoot", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::JumpDown] = spriteSheet.getAnimationIndex("jump_down", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_one");

    spriteSheet.loadFromFile("assets/sprites/npcs.spt", m_textureResource);
    m_sprites[SpriteID::WhirlyBob] = spriteSheet.getSprite("whirlybob");
    m_sprites[SpriteID::Clocksy] = spriteSheet.getSprite("clocksy");

    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "clocksy");

    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "whirlybob");
    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "whirlybob");
    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "whirlybob");

    m_sprites[SpriteID::FruitSmall] = spriteSheet.getSprite("fruit");

    //load background
    auto ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/background.png")).setDepth(-50);
    ent.addComponent<xy::Transform>().setScale(4.f, 4.f);
    ent.getComponent<xy::Transform>().setPosition((-(xy::DefaultSceneSize.x / 2.f) + MapBounds.width / 2.f), 0.f);

    ent.addComponent<xy::AudioEmitter>().setSource("assets/sound/music/02.ogg");
    ent.getComponent<xy::AudioEmitter>().play();
    ent.getComponent<xy::AudioEmitter>().setLooped(true);
    ent.getComponent<xy::AudioEmitter>().setVolume(0.25f);
    ent.getComponent<xy::AudioEmitter>().setChannel(1);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::Music;

    if (!m_scores.loadFromFile(xy::FileSystem::getConfigDirectory("demo_game") + scoreFile))
    {
        //create one
        m_scores.addProperty("hiscore", "0");
        m_scores.save(xy::FileSystem::getConfigDirectory("demo_game") + scoreFile);
    }
}

bool GameState::loadScene(const MapData& data, sf::Vector2f position)
{
    std::string mapName(data.mapName);
    std::string remoteSha(data.mapSha);
    
    //check the local sha1 to make sure we have the same file version
    std::string mapSha = getSha("assets/maps/" + mapName);
    if (mapSha != remoteSha)
    {
        m_sharedData.error = "Local copy of " + mapName + " is different version to server's";
        requestStackPush(StateID::Error);
        return false;
    }

    tmx::Map map;
    if (!map.load("assets/maps/" + mapName))
    {
        //disconnect from server and push disconnected state
        m_sharedData.error = "Could not find map " + mapName;
        requestStackPush(StateID::Error);
        return false;
    }

    auto size = map.getTileCount() * map.getTileSize();

    m_mapTextures[m_currentMapTexture].create(size.x, size.y);
    m_mapTextures[m_currentMapTexture].clear(sf::Color::Transparent);
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
    m_mapTextures[m_currentMapTexture].display();
    
    if (flags != MapFlags::Client)
    {
        //disconnect and bail
        m_sharedData.error = std::string(data.mapName) + ": Missing or corrupt map data.";
        requestStackPush(StateID::Error);
        return false;
    }

    //create the map sprite
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_mapTextures[m_currentMapTexture].getTexture());
    entity.getComponent<xy::Sprite>().setDepth(-10);
#ifdef DDRAW
    entity.getComponent<xy::Sprite>().setColour({ 255,255,255,120 });
#endif
    entity.addComponent<xy::Transform>().setPosition(position);
    entity.addComponent<MapAnimator>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapBackground;

    m_currentMapTexture = (m_currentMapTexture + 1) % m_mapTextures.size();
    
    m_scene.getActiveCamera().getComponent<xy::Transform>().setPosition(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    m_scene.getActiveCamera().getComponent<xy::AudioListener>().setVolume(1.f);
    //m_scene.getActiveCamera().getComponent<xy::Camera>().setZoom(0.5f);

    m_mapData = data;

    return true;
}

void GameState::loadUI()
{
    //timeout msg
    auto ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(-410.f, 1010.f);
    ent.addComponent<xy::Text>(m_fontResource.get("assets/fonts/VeraMono.ttf"));
    ent.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::Timeout;

    //title texts
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color(255, 212, 0));
    ent.getComponent<xy::Text>().setString("PLAYER ONE");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 140.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    ent.getComponent<xy::Text>().setString("HIGH SCORE");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 260.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color(255, 0, 212));
    ent.getComponent<xy::Text>().setString("PLAYER TWO");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    
    //score texts
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString("0");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::ScoreOne;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 140.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString(m_scores.getProperties()[0].getValue<std::string>());
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::HighScore;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 260.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString("0");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::ScoreTwo;


    //lives display
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/ui.spt", m_textureResource);

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, MapBounds.height - 128.f);
    ent.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_one_lives");
    ent.addComponent<xy::SpriteAnimation>().play(0);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::LivesOne;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 10.f, MapBounds.height - 128.f);
    ent.getComponent<xy::Transform>().setScale(-1.f, 1.f);
    ent.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_two_lives");
    ent.addComponent<xy::SpriteAnimation>().play(0);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::LivesTwo;
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

        if (idx == MessageIdent::MapFailed)
        {
            m_client.disconnect();
            m_sharedData.error = "Server Failed To Load Map";
            requestStackPush(StateID::Error);
        }
    }
        break;
    case PacketID::ActorAbsolute:
        //set absolute state of actor
    {
        const auto& state = evt.packet.as<ActorState>();

        xy::Command cmd;
        cmd.targetFlags = CommandID::NetActor;
        cmd.action = [&, state](xy::Entity entity, float)
        {
            if (entity.getComponent<Actor>().id == state.actor.id)
            {
                entity.getComponent<xy::Transform>().setPosition(state.x, state.y);

                auto msg = getContext().appInstance.getMessageBus().post<SceneEvent>(MessageID::SceneMessage);
                msg->entity = entity;
                msg->type = SceneEvent::ActorSpawned;
                msg->actorID = state.actor.type;
                msg->x = state.x;
                msg->y = state.y;
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::ActorEvent:
    {
        const auto actorEvent = evt.packet.as<ActorEvent>();
        switch (actorEvent.type)
        {
        case ActorEvent::Spawned:
            spawnActor(actorEvent);
            break;
        case ActorEvent::Died:
            killActor(actorEvent);
            break;
        case ActorEvent::GotAngry:
        {
            auto id = actorEvent.actor.id;
            xy::Command cmd;
            cmd.targetFlags = CommandID::NetActor;
            cmd.action = [id](xy::Entity entity, float)
            {
                if (entity.getComponent<Actor>().id == id)
                {
                    entity.getComponent<xy::Sprite>().setColour(sf::Color(255, 160, 160));
                }
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
            break;
        default: break;
        }
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
                auto& anim = entity.getComponent<AnimationController>();
                anim.nextAnimation = static_cast<AnimationController::Animation>(state.animationID);
                anim.direction = state.animationDirection;
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::MapJoin:
    {
        MapData data = evt.packet.as<MapData>();

        //load new actors
        if (loadScene(data))
        {
            spawnMapActors();

            //send ready signal
            m_client.sendPacket(PacketID::ClientReady, 0, xy::NetFlag::Reliable, 1);
            m_playerInput.setEnabled(true);
        }
        else
        {
            m_client.disconnect();
        }
    }
        break;
    case PacketID::MapChange:
    {
        const auto data = evt.packet.as<MapData>();
        switchMap(data);
    }
        break;
    case PacketID::ClientUpdate:
    {
        const auto& state = evt.packet.as<ClientState>();

        //reconcile
        if (m_playerInput.isEnabled())
        {
            m_scene.getSystem<PlayerSystem>().reconcile(state, m_playerInput.getPlayerEntity());
        }
    }
        break;
    case PacketID::ClientData:
    {
        ClientData data = evt.packet.as<ClientData>();
        spawnClient(data);        
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
    case PacketID::InventoryUpdate:
    {
        auto data = evt.packet.as<InventoryUpdate>();
        updateUI(data);
    }
        break;
    case PacketID::RoundWarning:
        spawnWarning();
        break;
    case PacketID::GameOver:
        m_sharedData.error = "Game Over";
        requestStackPush(StateID::Error);
        break;
    }
}

void GameState::handleTimeout()
{
    float currTime = m_clientTimeout.getElapsedTime().asSeconds();
    if (currTime > clientTimeout / 5.f)
    {
        float displayTime = clientTimeout - currTime;
        
        xy::Command cmd;
        cmd.targetFlags = CommandID::Timeout;
        cmd.action = [displayTime](xy::Entity entity, float)
        {
            entity.getComponent<xy::Text>().setString("WARNING: Connection Problem\nAuto Disconnect in: " + std::to_string(displayTime));
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
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
    auto name = xy::Util::String::toLower(layer->getName());
    if (name == "platform")
    {
        const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
        for (const auto& obj : objs)
        {
            createCollisionObject(m_scene, obj, CollisionType::Platform);
        }
        
        return MapFlags::Platform;
    }
    else if (name == "solid")
    {
        const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
        for (const auto& obj : objs)
        {
            createCollisionObject(m_scene, obj, CollisionType::Solid);
        }
        return MapFlags::Solid;
    }
    else if (name == "teleport")
    {
        const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
        for (const auto& obj : objs)
        {
            createCollisionObject(m_scene, obj, CollisionType::Teleport);
        }
        return MapFlags::Teleport;
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
        m_mapTextures[m_currentMapTexture].draw(v.second.data(), v.second.size(), sf::Quads, v.first);
    }

    return MapFlags::Graphics;
}

void GameState::spawnActor(const ActorEvent& actorEvent)
{
    auto msg = getContext().appInstance.getMessageBus().post<SceneEvent>(MessageID::SceneMessage);
    
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(actorEvent.x, actorEvent.y);
    entity.addComponent<Actor>() = actorEvent.actor;
    entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor | CommandID::MapItem;
    entity.addComponent<xy::NetInterpolate>();
    entity.addComponent<AnimationController>();

    msg->entity = entity;
    msg->type = SceneEvent::ActorSpawned;
    msg->actorID = actorEvent.actor.type;
    msg->x = actorEvent.x;
    msg->y = actorEvent.y;

    switch (actorEvent.actor.type)
    {
    default: break;
    case ActorID::BubbleOne:
    case ActorID::BubbleTwo:
        entity.addComponent<xy::Sprite>() =
            (actorEvent.actor.type == ActorID::BubbleOne) ? m_sprites[SpriteID::BubbleOne] : m_sprites[SpriteID::BubbleTwo];
        entity.addComponent<xy::SpriteAnimation>().play(0);
        entity.getComponent<xy::Sprite>().setDepth(-2);
        entity.getComponent<xy::Transform>().setOrigin(BubbleSize / 2.f, BubbleSize / 2.f);

        entity.addComponent<CollisionComponent>().addHitbox({ 0.f, 0.f, BubbleSize, BubbleSize }, CollisionType::Bubble);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Bubble);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Player);
        entity.addComponent<xy::QuadTreeItem>().setArea({ 0.f, 0.f, BubbleSize, BubbleSize });

        {
            xy::Command cmd;
            cmd.targetFlags = (actorEvent.actor.type == ActorID::BubbleOne) ? CommandID::PlayerOne : CommandID::PlayerTwo;
            cmd.action = [](xy::Entity entity, float)
            {
                auto& controller = entity.getComponent<AnimationController>();
                controller.currentAnim = AnimationController::Shoot;
                entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[AnimationController::Shoot]);
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
        break;
    case ActorID::FruitSmall:
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::FruitSmall];
        entity.addComponent<xy::SpriteAnimation>().play(xy::Util::Random::value(0, 1));
        entity.getComponent<xy::Sprite>().setDepth(2);
        entity.getComponent<xy::Transform>().setOrigin(SmallFruitSize / 2.f, SmallFruitSize / 2.f);
        break;
    }
}

void GameState::spawnClient(const ClientData& data)
{
    //create the local ent
    //set sprite based on actor type (player one or two)
    auto entity = m_scene.createEntity();
    entity.addComponent<Actor>() = data.actor;
    entity.addComponent<xy::Transform>().setPosition(data.spawnX, data.spawnY);
    entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::PlayerOne];

    if (data.actor.type == ActorID::PlayerOne)
    {
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::PlayerOne];
        entity.getComponent<xy::Transform>().setScale(-1.f, 1.f);
        entity.addComponent<xy::CommandTarget>().ID = CommandID::PlayerOne;
    }
    else
    {
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::PlayerTwo];
        entity.addComponent<xy::CommandTarget>().ID = CommandID::PlayerTwo;
    }

    entity.getComponent<xy::Transform>().setOrigin(PlayerSize / 2.f, PlayerSize);
    entity.addComponent<xy::SpriteAnimation>().play(0);
    entity.addComponent<MapAnimator>().state = MapAnimator::State::Static;

    if (data.peerID == m_client.getPeer().getID())
    {
        //this is us, stash the info
        m_clientData = data;

        //add a local controller
        entity.addComponent<Player>().playerNumber = (data.actor.type == ActorID::PlayerOne) ? 0 : 1;
        entity.getComponent<Player>().spawnPosition = { data.spawnX, data.spawnY };
        m_playerInput.setPlayerEntity(entity);

        entity.addComponent<CollisionComponent>().addHitbox({ PlayerSizeOffset, PlayerSizeOffset, PlayerSize, PlayerSize }, CollisionType::Player);
        entity.getComponent<CollisionComponent>().addHitbox({ -PlayerSizeOffset, PlayerSize + PlayerSizeOffset, PlayerSize + (PlayerSizeOffset * 2.f), PlayerFootSize }, CollisionType::Foot);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Player);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PlayerMask);
        entity.addComponent<xy::QuadTreeItem>().setArea(entity.getComponent<CollisionComponent>().getLocalBounds());

        //entity.addComponent<xy::Camera>() = m_scene.getActiveCamera().getComponent<xy::Camera>();
        //m_scene.setActiveCamera(entity);

        //temp for now just to flash player when invincible
        entity.addComponent<xy::Callback>().function = 
            [](xy::Entity entity, float)
        {
            static sf::Color colour = sf::Color::White;
            if (entity.getComponent<Player>().timer > 0
                && entity.getComponent<Player>().state != Player::State::Dying)
            {
                colour.a = (colour.a == 0) ? 255 : 0;
                entity.getComponent<xy::Sprite>().setColour(colour);
            }
            else
            {
                entity.getComponent<xy::Sprite>().setColour(sf::Color::White);
            }
        };
        entity.getComponent<xy::Callback>().active = true;
    }
    else
    {
        //add interp controller
        entity.getComponent<xy::CommandTarget>().ID |= CommandID::NetActor;
        entity.addComponent<xy::NetInterpolate>();
    }
}

void GameState::killActor(const ActorEvent& actorEvent)
{
    //kill the scene entity
    //auto id = actorEvent.actor.id;
    xy::Command cmd;
    cmd.targetFlags = CommandID::NetActor;
    cmd.action = [&, actorEvent](xy::Entity entity, float)
    {
        if (entity.getComponent<Actor>().id == actorEvent.actor.id)
        {
            m_scene.destroyEntity(entity);

            //raise a message to say who died
            auto msg = getContext().appInstance.getMessageBus().post<SceneEvent>(MessageID::SceneMessage);
            msg->entity = entity;
            msg->type = SceneEvent::ActorRemoved;
            msg->actorID = actorEvent.actor.type;
            msg->x = actorEvent.x;
            msg->y = actorEvent.y;
        }
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

void GameState::switchMap(const MapData& data)
{
    m_playerInput.setEnabled(false);

    //clear remaining actors (should just be collectables / bubbles)
    //as well as any geometry
    xy::Command cmd;
    cmd.targetFlags = CommandID::MapItem;
    cmd.action = [&](xy::Entity entity, float)
    {
        m_scene.destroyEntity(entity);
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    
    cmd.targetFlags = CommandID::Music;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::AudioEmitter>().setPitch(1.f);
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    
    m_scene.update(0.f); //force the command right away


    if (loadScene(data, { 0.f, MapBounds.height - 128.f }))
    {
        //init transition
        xy::Command cmd;
        cmd.targetFlags = CommandID::MapBackground;
        cmd.action = [&](xy::Entity entity, float)
        {
            auto& animator = entity.getComponent<MapAnimator>();
            if (animator.state == MapAnimator::State::Static)
            {
                animator.state = MapAnimator::State::Active;
                animator.dest.y = -(MapBounds.height - 128.f);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        //move player sprites to position
        cmd.targetFlags = CommandID::PlayerOne | CommandID::PlayerTwo;
        cmd.action = [&](xy::Entity entity, float)
        {
            auto& animator = entity.getComponent<MapAnimator>();
            animator.dest = entity.getComponent<xy::CommandTarget>().ID == CommandID::PlayerOne ? playerOneSpawn : playerTwoSpawn;
            animator.state = MapAnimator::State::Active;
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        m_scene.getSystem<CollisionSystem>().setEnabled(false);
    }
}

void GameState::spawnMapActors()
{
    //xy::EmitterSettings emitterSettings;
    //emitterSettings.loadFromFile("assets/particles/angry.xyp", m_textureResource);
    
    for (auto i = 0; i < m_mapData.actorCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setOrigin(NPCSize / 2.f, NPCSize / 2.f);
        entity.addComponent<Actor>() = m_mapData.actors[i];
        entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor | CommandID::MapItem;
        entity.addComponent<xy::NetInterpolate>();

        switch (m_mapData.actors[i].type)
        {
        default:
            //add missing texture or placeholder
            break;
        case ActorID::Whirlybob:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::WhirlyBob];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::WhirlyBob];
            break;
        case ActorID::Clocksy:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Clocksy];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::Clocksy];
            break;
        }
        entity.getComponent<xy::Sprite>().setDepth(-3); //behind bubbles
        entity.addComponent<xy::SpriteAnimation>().play(0);
        //entity.addComponent<xy::ParticleEmitter>().settings = emitterSettings;
        //entity.getComponent<xy::ParticleEmitter>().start();
    }
}

void GameState::spawnWarning()
{
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(m_scene.getActiveCamera().getComponent<xy::Transform>().getPosition());
    entity.getComponent<xy::Transform>().move(xy::DefaultSceneSize.x / 1.8f, -180.f);
    entity.addComponent<xy::Text>(m_fontResource.get("assets/fonts/Cave-Story.ttf")).setString("Hurry Up!");
    entity.getComponent<xy::Text>().setCharacterSize(200);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = Flasher(m_scene);

    xy::Command cmd;
    cmd.targetFlags = CommandID::Music;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::AudioEmitter>().pause();
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    getContext().appInstance.getMessageBus().post<MapEvent>(MessageID::MapMessage)->type = MapEvent::HurryUp;
}

void GameState::updateUI(const InventoryUpdate& data)
{
    xy::Command scoreCmd;
    scoreCmd.action = [data](xy::Entity entity, float)
    {
        entity.getComponent<xy::Text>().setString(std::to_string(data.score));
    };

    //check if greater than high score and update
    if (data.score > m_scores.getProperties()[0].getValue<sf::Int32>())
    {
        m_scores.findProperty("hiscore")->setValue(static_cast<sf::Int32>(data.score));
        m_scores.save(xy::FileSystem::getConfigDirectory("demo_game") + scoreFile);

        xy::Command cmd;
        cmd.targetFlags = CommandID::HighScore;
        cmd.action = [data](xy::Entity entity, float)
        {
            entity.getComponent<xy::Text>().setString(std::to_string(data.score));
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }

    xy::Command livesCommand;
    livesCommand.action = [data](xy::Entity entity, float)
    {
        entity.getComponent<xy::SpriteAnimation>().play(data.lives);
    };


    xy::Command textCommand; //displays the score bubble
    textCommand.action =
        [&, data](xy::Entity entity, float)
    {
        auto pos = entity.getComponent<xy::Transform>().getPosition();

        auto scoreEnt = m_scene.createEntity();
        scoreEnt.addComponent<xy::Transform>().setPosition(pos);
        scoreEnt.addComponent<xy::Text>(m_fontResource.get("assets/fonts/Cave-Story.ttf")).setString(std::to_string(data.amount));
        scoreEnt.getComponent<xy::Text>().setCharacterSize(40);
        scoreEnt.addComponent<ScoreTag>();
    };

    if (data.playerID == 0)
    {
        scoreCmd.targetFlags = CommandID::ScoreOne;
        livesCommand.targetFlags = CommandID::LivesOne;
        textCommand.targetFlags = CommandID::PlayerOne;
    }
    else
    {
        scoreCmd.targetFlags = CommandID::ScoreTwo;
        livesCommand.targetFlags = CommandID::LivesTwo;
        textCommand.targetFlags = CommandID::PlayerTwo;
    }
    m_scene.getSystem<xy::CommandSystem>().sendCommand(scoreCmd);
    m_scene.getSystem<xy::CommandSystem>().sendCommand(livesCommand);

    if (data.amount)
    {
        m_scene.getSystem<xy::CommandSystem>().sendCommand(textCommand);
    }
}