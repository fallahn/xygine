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
#include "BackgroundShader.hpp"
#include "MusicCallback.hpp"
#include "TowerDirector.hpp"
#include "TowerGuyCallback.hpp"
#include "BonusSystem.hpp"
#include "ClientNotificationCallbacks.hpp"
#include "LoadingScreen.hpp"
#include "SpringFlower.hpp"
#include "HatSystem.hpp"

#include <xyginext/core/App.hpp>
#include <xyginext/core/FileSystem.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/NetInterpolation.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/AudioListener.hpp>

#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
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
#include <xyginext/graphics/postprocess/OldSchool.hpp>

#include <xyginext/network/NetData.hpp>
#include <xyginext/util/Random.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>

#include <cstring>

namespace
{
    const float clientTimeout = 20.f;
    const std::string scoreFile("scores.txt");

    struct BonusUI final
    {
        Bonus::Value value = Bonus::B;
        sf::Uint8 playerID = 0;
    };

#ifdef XY_DEBUG
    sf::Uint8 debugActorCount = 0;
    sf::Uint8 debugPlayerState = 0;
    sf::Uint8 debugActorUpdate = 0;
    sf::Vector2f debugCrownVel;
    sf::CircleShape debugShape;
#endif

}

GameState::GameState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sharedData, LoadingScreen& ls)
    : xy::State         (stack, ctx),
    m_scene             (ctx.appInstance.getMessageBus()),
    m_sharedData        (sharedData),
    m_loadingScreen     (ls),
    m_playerInputs      ({ PlayerInput(m_client, sharedData.inputBindings[0]), PlayerInput(m_client, sharedData.inputBindings[1]) }),
    m_currentMapTexture (0)
{
    launchLoadingScreen();
    loadAssets();
    loadTower();
    loadUI();
    m_client.create(2);

    bool connected = false;
    if (sharedData.hostState == SharedStateData::Host)
    {
        sf::Clock joinTimer;
        m_server.start();
        while (!m_server.ready() && joinTimer.getElapsedTime().asSeconds() < 8.f) {}
        connected = m_client.connect("localhost", 40003);
    }
    else
    {
        connected = m_client.connect(sharedData.remoteIP, 40003);
    }

    if (!connected)
    {
        sharedData.error = "Failed to connect to server";
        requestStackPush(StateID::Error);
    }

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());

    ctx.renderWindow.setMouseCursorVisible(false);

    //sharedData.playerCount = 2;

#ifdef XY_DEBUG
    debugShape.setRadius(16.f);
    debugShape.setOrigin(16.f, 48.f);
    debugShape.setFillColor(sf::Color(255, 255, 255, 20));
    debugShape.setOutlineColor(sf::Color::Magenta);
    debugShape.setOutlineThickness(1.f);
#endif
    quitLoadingScreen();
}

//public
bool GameState::handleEvent(const sf::Event& evt)
{
    for (auto i = 0u; i < m_sharedData.playerCount; ++i)
    {
        m_playerInputs[i].handleEvent(evt);
    }

    m_scene.forwardEvent(evt);

    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        /*case sf::Keyboard::Insert:
            spawnRoundSkip();
            break;
        case sf::Keyboard::Home:
            m_server.stop();
            break;*/
        case sf::Keyboard::P:
        case sf::Keyboard::Escape:
        case sf::Keyboard::Pause:
            requestStackPush(StateID::Pause);
            m_client.sendPacket(PacketID::RequestServerPause, sf::Uint8(0), xy::NetFlag::Reliable, 1);
            break;
        }
    }
    else if (evt.type == sf::Event::JoystickButtonReleased)
    {
        //if (evt.joystickButton.joystickId == 0)
        {
            if (evt.joystickButton.button == 7) //start on xbox
            {
                requestStackPush(StateID::Pause);
                m_client.sendPacket(PacketID::RequestServerPause, sf::Uint8(0), xy::NetFlag::Reliable, 1);
            }
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
            
            for (auto i = 0u; i < m_sharedData.playerCount; ++i)
            {
                m_playerInputs[i].setEnabled(true);
            }
            m_scene.setSystemActive<CollisionSystem>(true);
            m_scene.setSystemActive<xy::InterpolationSystem>(true);

            //send OK to server to continue game
            for (auto i = 0u; i < m_sharedData.playerCount; ++i)
            {
                m_client.sendPacket(PacketID::MapReady, m_clientData[i].actor.type, xy::NetFlag::Reliable, 1);
            }
        }
    }
    else if (msg.id == MessageID::MenuMessage)
    {
        const auto& data = msg.getData<MenuEvent>();
        switch (data.action)
        {
        default: break;
        case MenuEvent::QuitGameClicked:
            m_client.disconnect();
            requestStackClear();
            requestStackPush(StateID::MainMenu);            
            
            //kill server if we're hosting
            if (m_sharedData.hostState == SharedStateData::Host)
            {
                m_server.stop();
            }
            break;
        case MenuEvent::ContinueGameClicked:
            requestStackPop();
           
            for (auto i = 0u; i < m_sharedData.playerCount; ++i)
            {
                auto actor = m_playerInputs[i].getPlayerEntity().getComponent<Actor>();
                m_client.sendPacket(PacketID::ClientContinue, actor.id, xy::NetFlag::Reliable, 1);

                spawnTowerDude(actor.type);
            }
            break;
        case MenuEvent::UnpauseGame:
            requestStackPop();
            m_client.sendPacket(PacketID::RequestServerPause, sf::Uint8(1), xy::NetFlag::Reliable, 1);
            break;
        }
    }
    else if (msg.id == xy::Message::WindowMessage)
    {
        const auto& data = msg.getData<xy::Message::WindowEvent>();
        if (data.type == xy::Message::WindowEvent::LostFocus)
        {
            requestStackPush(StateID::Pause);
            m_client.sendPacket(PacketID::RequestServerPause, sf::Uint8(0), xy::NetFlag::Reliable, 1);
        }
    }

    m_scene.forwardMessage(msg);
}

bool GameState::update(float dt)
{   
    DPRINT("Actor count", std::to_string(debugActorCount));
    //DPRINT("Actor Update Count", std::to_string(debugActorUpdate));
    //DPRINT("Player Server State", std::to_string(debugPlayerState));
    //DPRINT("Crown Vel", std::to_string(debugCrownVel.x) + ", " + std::to_string(debugCrownVel.y));

#ifdef XY_DEBUG
    debugActorUpdate = 0;
    /*switch (Player::State(debugPlayerState))
    {
    default:
        DPRINT("Player state", std::to_string(debugPlayerState));
        break;
    case Player::State::Disabled:
        DPRINT("Player server state", "Disabled");
        break;
    case Player::State::Dying:
        DPRINT("Player server state", "Dying");
        break;
    case Player::State::Jumping:
        DPRINT("Player server state", "Jumping");
        break;
    case Player::State::Walking:
        DPRINT("Player server state", "Walking");
        break;
    }*/

#endif //XY_DEBUG
    
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
    
    for (auto i = 0u; i < m_sharedData.playerCount; ++i)
    {
        m_playerInputs[i].update();
    }
    m_scene.update(dt);
    return false;
}

void GameState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
#ifdef XY_DEBUG
    rw.draw(debugShape);
#endif
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
    m_scene.addSystem<SpringFlowerSystem>(mb);
    m_scene.addSystem<xy::AudioSystem>(mb);
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::CameraSystem>(mb);
    m_scene.addSystem<xy::CommandSystem>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
    m_scene.addSystem<xy::AudioSystem>(mb);
    
    //m_scene.addPostProcess<xy::PostOldSchool>();
    m_scene.addDirector<ParticleDirector>(m_textureResource);
    m_scene.addDirector<FXDirector>();
    m_scene.addDirector<TowerDirector>();

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
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::JumpUp] = spriteSheet.getAnimationIndex("jump_up", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_one");
    m_animationControllers[SpriteID::PlayerOne].animationMap[AnimationController::Dead] = spriteSheet.getAnimationIndex("dead", "player_one");

    m_sprites[SpriteID::MagicHat] = spriteSheet.getSprite("crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::Shoot] = spriteSheet.getAnimationIndex("shoot", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::JumpUp] = spriteSheet.getAnimationIndex("jump_up", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::JumpDown] = spriteSheet.getAnimationIndex("jump_down", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "crown");
    m_animationControllers[SpriteID::MagicHat].animationMap[AnimationController::Dead] = spriteSheet.getAnimationIndex("dead", "crown");

    //NPCs
    spriteSheet.loadFromFile("assets/sprites/npcs.spt", m_textureResource);
    m_sprites[SpriteID::WhirlyBob] = spriteSheet.getSprite("whirlybob");
    m_sprites[SpriteID::Clocksy] = spriteSheet.getSprite("clocksy");
    m_sprites[SpriteID::Goobly] = spriteSheet.getSprite("goobly");
    m_sprites[SpriteID::Balldock] = spriteSheet.getSprite("balldock");
    m_sprites[SpriteID::Squatmo] = spriteSheet.getSprite("squatmo");

    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "clocksy");
    m_animationControllers[SpriteID::Clocksy].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "clocksy");

    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "whirlybob");
    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "whirlybob");
    m_animationControllers[SpriteID::WhirlyBob].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "whirlybob");

    m_animationControllers[SpriteID::Goobly].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "goobly");

    m_animationControllers[SpriteID::Balldock].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "balldock");
    m_animationControllers[SpriteID::Balldock].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "balldock");
    m_animationControllers[SpriteID::Balldock].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "balldock");
    m_animationControllers[SpriteID::Balldock].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "balldock");
    m_animationControllers[SpriteID::Balldock].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "balldock");

    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "squatmo");
    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::JumpDown] = spriteSheet.getAnimationIndex("jump_down", "squatmo");
    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::JumpUp] = spriteSheet.getAnimationIndex("jump_up", "squatmo");
    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "squatmo");
    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::TrappedOne] = spriteSheet.getAnimationIndex("bubble_one", "squatmo");
    m_animationControllers[SpriteID::Squatmo].animationMap[AnimationController::TrappedTwo] = spriteSheet.getAnimationIndex("bubble_two", "squatmo");

    //dem fruits
    spriteSheet.loadFromFile("assets/sprites/food.spt", m_textureResource);
    m_sprites[SpriteID::FruitSmall] = spriteSheet.getSprite("food");

    //power ups
    spriteSheet.loadFromFile("assets/sprites/power_ups.spt", m_textureResource);
    m_sprites[SpriteID::FlameOne] = spriteSheet.getSprite("player_one_flame");
    m_sprites[SpriteID::FlameTwo] = spriteSheet.getSprite("player_two_flame");
    m_sprites[SpriteID::LightningOne] = spriteSheet.getSprite("player_one_star");
    m_sprites[SpriteID::LightningTwo] = spriteSheet.getSprite("player_two_star");

    m_animationControllers[SpriteID::FlameOne].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "player_one_flame");
    m_animationControllers[SpriteID::FlameOne].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_one_flame");
    m_animationControllers[SpriteID::FlameOne].animationMap[AnimationController::Shoot] = spriteSheet.getAnimationIndex("shoot", "player_one_flame");
    m_animationControllers[SpriteID::FlameOne].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_one_flame");

    m_animationControllers[SpriteID::FlameTwo].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "player_two_flame");
    m_animationControllers[SpriteID::FlameTwo].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_two_flame");
    m_animationControllers[SpriteID::FlameTwo].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_two_flame");

    m_animationControllers[SpriteID::LightningOne].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "player_one_star");
    m_animationControllers[SpriteID::LightningOne].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_one_star");
    m_animationControllers[SpriteID::LightningOne].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_one_star");

    m_animationControllers[SpriteID::LightningTwo].animationMap[AnimationController::Idle] = spriteSheet.getAnimationIndex("idle", "player_two_star");
    m_animationControllers[SpriteID::LightningTwo].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_two_star");
    m_animationControllers[SpriteID::LightningTwo].animationMap[AnimationController::Die] = spriteSheet.getAnimationIndex("die", "player_two_star");

    m_hatEmitter.loadFromFile("assets/particles/magic_hat.xyp", m_textureResource);

    //dudes to climb tower
    spriteSheet.loadFromFile("assets/sprites/tower_sprites.spt", m_textureResource);
    m_sprites[SpriteID::TowerDudeOne] = spriteSheet.getSprite("player_one");
    m_sprites[SpriteID::TowerDudeTwo] = spriteSheet.getSprite("player_two");

    m_animationControllers[SpriteID::TowerDudeOne].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_one");
    m_animationControllers[SpriteID::TowerDudeTwo].animationMap[AnimationController::Walk] = spriteSheet.getAnimationIndex("walk", "player_two");

    //bonus bubbles
    spriteSheet.loadFromFile("assets/sprites/ui.spt", m_textureResource);
    m_sprites[SpriteID::Bonus] = spriteSheet.getSprite("bonus");

    m_animationControllers[SpriteID::Bonus].animationMap[0] = spriteSheet.getAnimationIndex("b", "bonus");
    m_animationControllers[SpriteID::Bonus].animationMap[1] = spriteSheet.getAnimationIndex("o", "bonus");
    m_animationControllers[SpriteID::Bonus].animationMap[2] = spriteSheet.getAnimationIndex("n", "bonus");
    m_animationControllers[SpriteID::Bonus].animationMap[3] = spriteSheet.getAnimationIndex("u", "bonus");
    m_animationControllers[SpriteID::Bonus].animationMap[4] = spriteSheet.getAnimationIndex("s", "bonus");

    //load background
    float startX = (-(xy::DefaultSceneSize.x / 2.f) + MapBounds.width / 2.f);

    auto ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/background.png"));
    ent.addComponent<xy::Drawable>().setDepth(-50);
    ent.addComponent<xy::Transform>().setScale(4.f, 4.f);
    ent.getComponent<xy::Transform>().setPosition(startX, 0.f);

    if (m_backgroundShader.loadFromMemory(BackgroundFragment, sf::Shader::Fragment))
    {
        m_backgroundShader.setUniform("u_diffuseMap", m_textureResource.get("assets/images/background.png"));
        ent.getComponent<xy::Sprite>();
        ent.getComponent<xy::Drawable>().setShader(&m_backgroundShader);
        ent.addComponent<xy::Callback>().function = ColourRotator(m_backgroundShader);
    }
    ent.addComponent<xy::CommandTarget>().ID = CommandID::SceneBackground;

    //flowers
    float flowerPos = startX + xy::Util::Random::value(20.f, 36.f);
    for (auto i = 0; i < 2; ++i)
    {
        for (auto j = 0; j < 4; ++j)
        {
            ent = m_scene.createEntity();
            ent.addComponent<SpringFlower>(-192.f).headPos.x += xy::Util::Random::value(-15.f, 15.f);
            ent.getComponent<SpringFlower>().colour = { 120, 120, 120 };
            ent.getComponent<SpringFlower>().textureRect = { 0.f, 0.f, 64.f, 192.f };
            ent.getComponent<SpringFlower>().stiffness += xy::Util::Random::value(-2.f, 2.f);
            ent.addComponent<xy::Drawable>(m_textureResource.get("assets/images/flower.png")).setDepth(-5);
            ent.addComponent<xy::Transform>().setPosition(flowerPos, xy::DefaultSceneSize.y + xy::Util::Random::value(12.f, 27.f));
            flowerPos += xy::Util::Random::value(60.f, 90.f);
        }
        flowerPos += MapBounds.width * 1.3f;
    }

    //grass
    ent = m_scene.createEntity();
    m_textureResource.get("assets/images/grass.png").setRepeated(true);
    auto spriteSize = ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/grass.png")).getSize();
    ent.getComponent<xy::Sprite>().setColour({ 120, 120 ,120 });
    ent.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, xy::DefaultSceneSize.x, spriteSize.y });
    ent.addComponent<xy::Drawable>();
    ent.addComponent<xy::Transform>().setPosition(startX, xy::DefaultSceneSize.y - spriteSize.y);

    //music player
    ent = m_scene.createEntity();
    ent.addComponent<xy::AudioEmitter>().setSource("assets/sound/music/01.ogg");
    ent.getComponent<xy::AudioEmitter>().play();
    ent.getComponent<xy::AudioEmitter>().setLooped(true);
    ent.getComponent<xy::AudioEmitter>().setVolume(0.25f);
    ent.getComponent<xy::AudioEmitter>().setChannel(1);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::SceneMusic;

    ent.addComponent<xy::Callback>().function = MusicCallback();
    ent.getComponent<xy::Callback>().active = true;
    ent.addComponent<xy::Transform>();

    m_bubbleParticles.loadFromFile("assets/particles/shoot.xyp", m_textureResource);

    if (!m_scores.loadFromFile(xy::FileSystem::getConfigDirectory(dataDir) + scoreFile))
    {
        //create one
        m_scores.addProperty("hiscore", "0");
        m_scores.save(xy::FileSystem::getConfigDirectory(dataDir) + scoreFile);
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
    entity.addComponent<xy::Drawable>().setDepth(-10);
#ifdef DDRAW
    entity.getComponent<xy::Sprite>().setColour({ 255,255,255,120 });
#endif
    entity.addComponent<xy::Transform>().setPosition(position);
    entity.addComponent<MapAnimator>();
    if (position.y < 0) entity.getComponent<MapAnimator>().state = MapAnimator::State::Active;
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MapBackground;

    m_currentMapTexture = (m_currentMapTexture + 1) % m_mapTextures.size();
    
    m_scene.getActiveCamera().getComponent<xy::Transform>().setPosition(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    m_scene.getActiveCamera().getComponent<xy::AudioListener>().setVolume(1.f);
    //m_scene.getActiveCamera().getComponent<xy::Camera>().setZoom(0.5f);

    m_mapData = data;

    return true;
}

void GameState::loadTower()
{
    xy::SpriteSheet princessSpriteSheet;    
    princessSpriteSheet.loadFromFile("assets/sprites/princess.spt", m_textureResource);

    //xy::SpriteSheet towerSpriteSheet;
    //towerSpriteSheet.loadFromFile("assets/sprites/tower_sprites.spt", m_textureResource);

    auto ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/wall.png"));
    ent.addComponent<xy::Drawable>().setDepth(5);
    ent.addComponent<xy::Transform>();

    ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/wall_bottom.png"));
    ent.addComponent<xy::Drawable>().setDepth(5);
    ent.addComponent<xy::Transform>().setPosition(0.f, MapBounds.height - ent.getComponent<xy::Sprite>().getSize().y);

    ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/tower.png"));
    ent.addComponent<xy::Drawable>().setDepth(5);
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width, 0.f);

    auto princessEnt = m_scene.createEntity();
    princessEnt.addComponent<xy::Sprite>() = princessSpriteSheet.getSprite("player_two");
    princessEnt.getComponent<xy::Sprite>();
    princessEnt.addComponent<xy::Drawable>().setDepth(6);
    ent.getComponent<xy::Transform>().addChild(princessEnt.addComponent<xy::Transform>());
    princessEnt.getComponent<xy::Transform>().setPosition(128.f, 12.f);
    princessEnt.addComponent<xy::SpriteAnimation>().play(0);
    princessEnt.addComponent<xy::CommandTarget>().ID = CommandID::Princess;
    princessEnt.addComponent<AnimationController>().animationMap[AnimationController::Shoot] = princessSpriteSheet.getAnimationIndex("angry", "player_two");
    princessEnt.getComponent<AnimationController>().nextAnimation = AnimationController::Idle;
    princessEnt.getComponent<AnimationController>().direction = -1.f;
    princessEnt.addComponent<Actor>().id = princessEnt.getIndex();
    princessEnt.getComponent<Actor>().type = ActorID::PrincessTwo;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Sprite>(m_textureResource.get("assets/images/tower.png"));
    ent.addComponent<xy::Drawable>().setDepth(5);
    ent.addComponent<xy::Transform>().setPosition(-ent.getComponent<xy::Sprite>().getSize().x, 0.f);

    princessEnt = m_scene.createEntity();
    princessEnt.addComponent<xy::Sprite>() = princessSpriteSheet.getSprite("player_one");
    princessEnt.getComponent<xy::Sprite>();
    princessEnt.addComponent<xy::Drawable>().setDepth(6);
    ent.getComponent<xy::Transform>().addChild(princessEnt.addComponent<xy::Transform>());
    princessEnt.getComponent<xy::Transform>().setPosition(64.f, 12.f);
    princessEnt.addComponent<xy::SpriteAnimation>().play(0);
    princessEnt.addComponent<xy::CommandTarget>().ID = CommandID::Princess;
    princessEnt.addComponent<AnimationController>().animationMap[AnimationController::Shoot] = princessSpriteSheet.getAnimationIndex("angry", "player_one");
    princessEnt.getComponent<AnimationController>().nextAnimation = AnimationController::Idle;
    princessEnt.addComponent<Actor>().id = princessEnt.getIndex();
    princessEnt.getComponent<Actor>().type = ActorID::PrincessOne;
}

void GameState::loadUI()
{
    //timeout msg
    auto ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(-410.f, 1010.f);
    ent.addComponent<xy::Text>(m_fontResource.get("assets/fonts/VeraMono.ttf"));
    ent.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::Timeout | CommandID::UIElement;

    //title texts
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color(255, 212, 0));
    ent.getComponent<xy::Text>().setString("PLAYER ONE");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::UIElement;
    
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 140.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    ent.getComponent<xy::Text>().setString("HIGH SCORE");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::UIElement;
    
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 260.f, 10.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setFillColour(sf::Color(255, 0, 212));
    ent.getComponent<xy::Text>().setString("PLAYER TWO");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::UIElement;
    
    //score texts
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString("0");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::ScoreOne | CommandID::UIElement;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 140.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString(m_scores.getProperties()[0].getValue<std::string>());
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::HighScore | CommandID::UIElement;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 260.f, 46.f);
    ent.addComponent<xy::Text>(font);
    ent.getComponent<xy::Text>().setString("0");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::ScoreTwo | CommandID::UIElement;


    //lives display
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/ui.spt", m_textureResource);

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(10.f, MapBounds.height - 96.f);
    ent.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_one_lives");
    ent.addComponent<xy::Drawable>().setDepth(6);
    ent.addComponent<xy::SpriteAnimation>().play(0);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::LivesOne;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition(MapBounds.width - 10.f, MapBounds.height - 96.f);
    ent.getComponent<xy::Transform>().setScale(-1.f, 1.f);
    ent.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_two_lives");
    ent.addComponent<xy::Drawable>().setDepth(6);
    ent.addComponent<xy::SpriteAnimation>().play(0);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::LivesTwo;

    //level counter
    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 16.f, MapBounds.height - 108.f);
    ent.addComponent<xy::Text>(font).setString("1");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::LevelCounter | CommandID::UIElement;

    ent = m_scene.createEntity();
    ent.addComponent<xy::Transform>().setPosition((MapBounds.width / 2.f) - 60.f, MapBounds.height - 148.f);
    ent.addComponent<xy::Text>(font).setString("LEVEL");
    ent.getComponent<xy::Text>().setCharacterSize(60);
    ent.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    ent.addComponent<xy::CommandTarget>().ID = CommandID::UIElement;

    //bonus display
    float startY = (xy::DefaultSceneSize.y / 2.f) - (2.5f * BubbleBounds.height);
    sf::Vector2f bonusPosition(0.f, startY);
    for (auto i = 0; i < 2; ++i)
    {
        for (auto j = 0; j < 5; ++j)
        {
            ent = m_scene.createEntity();
            ent.addComponent<xy::Transform>().setPosition(bonusPosition);
            ent.addComponent<xy::Sprite>() = m_sprites[SpriteID::Bonus];
            ent.addComponent<xy::Drawable>().setDepth(6);
            ent.getComponent<xy::Sprite>().setColour(sf::Color::Transparent);
            ent.addComponent<xy::SpriteAnimation>().play(j);
            ent.addComponent<xy::CommandTarget>().ID = CommandID::BonusBall;
            ent.addComponent<BonusUI>().value = Bonus::valueMap[j];
            ent.getComponent<BonusUI>().playerID = i;
            bonusPosition.y += BubbleBounds.height;
        }
        bonusPosition.x += (MapBounds.width - BubbleBounds.width);
        bonusPosition.y = startY;
    }
}

void GameState::handlePacket(const xy::NetEvent& evt)
{
    switch (evt.packet.getID())
    {
    default: break;
#ifdef XY_DEBUG
    case PacketID::DebugMapCount:
        debugActorCount = evt.packet.as<sf::Uint8>();
        break;
    case PacketID::DebugCrownVelocity:
        debugCrownVel = evt.packet.as<sf::Vector2f>();
        break;
#endif
    case PacketID::ServerFull:
        m_sharedData.error = "Could not connect to server, reason: Server full";
        requestStackPush(StateID::Error);
        return;
    case PacketID::HatChange:
    {
        auto info = evt.packet.as<sf::Uint8>();
        switch (info)
        {
        case HatFlag::OneOff:
            takeHat(0);
            break;
        case HatFlag::OneOn:
            giveHat(0);
            break;
        case HatFlag::TwoOff:
            takeHat(1);
            break;
        case HatFlag::TwoOn:
            giveHat(1);
            break;
        }
    }
        break;
    case PacketID::RequestClientPause: //other player paused server
    {
        auto pause = evt.packet.as<sf::Uint8>();
        if (pause == 0)
        {
            //show screen
            if (getStackSize() == 1)
            {
                requestStackPush(StateID::RemotePause);
            }
        }
        else
        {
            auto size = getStackSize();
            while (size-- > 1)
            {
                requestStackPop();
            }
        }
    }
        break;
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
                    entity.getComponent<xy::ParticleEmitter>().start();
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

#ifdef XY_DEBUG
        debugActorUpdate++;
#endif
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
            m_client.sendPacket(PacketID::ClientReady, sf::Uint8(m_sharedData.playerCount), xy::NetFlag::Reliable, 1);
            
            for (auto i = 0u; i < m_sharedData.playerCount; ++i)
            {
                m_playerInputs[i].setEnabled(true);
            }
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
        /*ClientState state;
        std::memcpy(&state, evt.packet.getData(), sizeof(state));
        auto size = evt.packet.getSize() - sizeof(state);
        std::vector<sf::Uint8> collisionData(size);
        std::memcpy(collisionData.data(), (sf::Uint8*)evt.packet.getData() + sizeof(state), size);*/
       
#ifdef XY_DEBUG
        debugShape.setPosition(state.x, state.y);
        debugPlayerState = sf::Uint8(state.sync.state);
#endif 
        
        //reconcile - seems a bit contrived, but must match the player input with 2 local players
        for (auto i = 0u; i < m_sharedData.playerCount; ++i)
        {
            if(m_clientData[i].actor.id == state.actor.id && m_playerInputs[i].isEnabled())
            {
                m_scene.getSystem<PlayerSystem>().reconcile(state, m_playerInputs[i].getPlayerEntity());
            }
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

        
        cmd.targetFlags = CommandID::TowerDude;
        cmd.action = [&, actorID](xy::Entity entity, float)
        {
            auto id = (actorID == ActorID::PlayerOne) ? ActorID::TowerOne : ActorID::TowerTwo;
            if (entity.getComponent<Actor>().type == id)
            {
                entity.getComponent<xy::SpriteAnimation>().play(1);
                entity.getComponent<xy::Callback>().active = true;
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
    case PacketID::LevelUpdate:
    {
        auto level = evt.packet.as<sf::Uint8>();
        updateLevelDisplay(level);
    }
        break;
    case PacketID::RoundWarning:
        spawnWarning();
        break;
    case PacketID::RoundSkip:
        spawnRoundSkip();
        break;
    case PacketID::GameOver:
        requestStackPush(StateID::GameOver);
        break;
    case PacketID::CollisionFlag:
    {
        auto data = evt.packet.as<CollisionFlagsUpdate>();
        xy::Command cmd;
        cmd.targetFlags = CommandID::NetActor;
        cmd.action = [data](xy::Entity entity, float)
        {
            if (entity.getComponent<Actor>().id == data.actor)
            {
                entity.getComponent<CollisionComponent>().setCollisionMaskBits(data.newflags);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
        break;
    case PacketID::GameComplete:
        transitionToEnd();
        break;
    }
}

void GameState::handleTimeout()
{
    if (!m_client.connected()) return;
    
    float currTime = m_clientTimeout.getElapsedTime().asSeconds();
    static bool wasError = false;
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

        wasError = true;
    }
    
    if (currTime > clientTimeout)
    {
        //push a message state
        m_sharedData.error = "Disconnected from server.";
        requestStackPush(StateID::Error);
    }
    else if (currTime < clientTimeout && wasError)
    {
        //reset the text
        xy::Command cmd;
        cmd.targetFlags = CommandID::Timeout;
        cmd.action = [](xy::Entity entity, float)
        {
            entity.getComponent<xy::Text>().setString("");
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        wasError = false;
    }
}

sf::Int32 GameState::parseObjLayer(const std::unique_ptr<tmx::Layer>& layer)
{
    sf::Int32 flags = 0;
    
    auto name = xy::Util::String::toLower(layer->getName());
    if (name == "geometry")
    {
        const auto& objs = dynamic_cast<tmx::ObjectGroup*>(layer.get())->getObjects();
        if (objs.empty())
        {
            return 0;
        }
        
        for (const auto& obj : objs)
        {
            auto type = xy::Util::String::toLower(obj.getType());
            if (type == "solid")
            {
                createCollisionObject(m_scene, obj, CollisionType::Solid);
                flags |= MapFlags::Solid;
            }
            else if (type == "platform")
            {
                createCollisionObject(m_scene, obj, CollisionType::Platform);
                flags |= MapFlags::Platform;
            }
            else if (type == "teleport")
            {
                createCollisionObject(m_scene, obj, CollisionType::Teleport);
            }
        }
    }
    
    return flags;
}

sf::Int32 GameState::parseTileLayer(const std::unique_ptr<tmx::Layer>& layer, const tmx::Map& map)
{
    const auto& tilesets = map.getTilesets();
    if (tilesets.empty())
    {
        return 0;
    }

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
    if (tiles.empty())
    {
        return 0;
    }

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

    //attempt at making a 'shadow' - needs darkening without changing the colour of every vertex
    //sf::RenderStates states;
    //states.transform.translate(16.f, 16.f);
    //for (const auto& v : vertexArrays)
    //{
    //    states.texture = v.first;
    //    m_mapTextures[m_currentMapTexture].draw(v.second.data(), v.second.size(), sf::Quads, states);
    //}

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

    auto addSprite = [&](xy::Entity sprEnt, sf::Int32 id)
    {
        sprEnt.addComponent<xy::Sprite>() = m_sprites[id];
        sprEnt.addComponent<xy::Drawable>();
        sprEnt.addComponent<xy::SpriteAnimation>();
        sprEnt.getComponent<AnimationController>() = m_animationControllers[id];
        sprEnt.getComponent<xy::Transform>().setOrigin(BubbleOrigin);
    };

    switch (actorEvent.actor.type)
    {
    default: break;
    case ActorID::Explosion:
        break;
    case ActorID::MagicHat:
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::MagicHat];
        entity.addComponent<xy::Drawable>().setDepth(3);
        entity.addComponent<xy::SpriteAnimation>();
        entity.getComponent<AnimationController>() = m_animationControllers[SpriteID::MagicHat];
        entity.getComponent<xy::Transform>().setOrigin(PlayerOrigin);
        break;
    case ActorID::BubbleOne:
    case ActorID::BubbleTwo:
        entity.addComponent<xy::Sprite>() =
            (actorEvent.actor.type == ActorID::BubbleOne) ? m_sprites[SpriteID::BubbleOne] : m_sprites[SpriteID::BubbleTwo];
        entity.addComponent<xy::SpriteAnimation>().play(0);
        entity.addComponent<xy::Drawable>().setDepth(-2);
        entity.getComponent<xy::Transform>().setOrigin(BubbleOrigin);

        entity.addComponent<CollisionComponent>().addHitbox(BubbleBounds, CollisionType::Bubble);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Bubble);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(0); //collision is added later
        entity.addComponent<xy::QuadTreeItem>().setArea(BubbleBounds);

        entity.addComponent<xy::ParticleEmitter>().settings = m_bubbleParticles;
        entity.getComponent<xy::ParticleEmitter>().settings.colour = (actorEvent.actor.type == ActorID::BubbleOne) ? BubbleColourOne : BubbleColourTwo;
        entity.getComponent<xy::ParticleEmitter>().start();

        {
            xy::Command cmd;
            cmd.targetFlags = (actorEvent.actor.type == ActorID::BubbleOne) ? CommandID::PlayerOne : CommandID::PlayerTwo;
            cmd.action = [](xy::Entity entity, float)
            {
                auto& controller = entity.getComponent<AnimationController>();
                controller.currentAnim = AnimationController::Shoot;
                entity.getComponent<xy::SpriteAnimation>().stop(); //rewinds the frame position
                entity.getComponent<xy::SpriteAnimation>().play(controller.animationMap[AnimationController::Shoot]);
            };
            m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
        break;
    case ActorID::FruitSmall:
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::FruitSmall];
        entity.addComponent<xy::SpriteAnimation>().play(xy::Util::Random::value(0, m_sprites[SpriteID::FruitSmall].getAnimationCount() - 1));
        entity.addComponent<xy::Drawable>().setDepth(2);
        entity.getComponent<xy::Transform>().setOrigin(SmallFoodOrigin);
        break;
    case ActorID::Goobly:
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Goobly];
        entity.addComponent<xy::Drawable>();
        entity.addComponent<xy::SpriteAnimation>().play(0);
        entity.getComponent<AnimationController>() = m_animationControllers[SpriteID::Goobly];
        entity.getComponent<xy::Transform>().setOrigin(GooblyOrigin);
        entity.addComponent<CollisionComponent>().addHitbox(GooblyBounds, CollisionType::NPC);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::NPC);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Player);
        break;
    case ActorID::LightningOne:
        addSprite(entity, SpriteID::LightningOne);
        break;
    case ActorID::LightningTwo:
        addSprite(entity, SpriteID::LightningTwo);
        break;
    case ActorID::FlameOne:
        addSprite(entity, SpriteID::FlameOne);
        break;
    case ActorID::FlameTwo:
        addSprite(entity, SpriteID::FlameTwo);
        break;
    case ActorID::Bonus:
        addSprite(entity, SpriteID::Bonus);
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

    std::size_t playerIndex = 0; //index of the client/player input array
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

        if (m_sharedData.playerCount == 2)
        {
            playerIndex = 1;
        }
    }

    entity.getComponent<xy::Transform>().setOrigin(PlayerOrigin);
    entity.addComponent<xy::SpriteAnimation>().play(0);
    entity.addComponent<MapAnimator>().state = MapAnimator::State::Static;
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::ParticleEmitter>().settings = m_hatEmitter;
    //entity.getComponent<xy::ParticleEmitter>().start();

    spawnTowerDude(data.actor.type);

    if (data.peerID == m_client.getPeer().getID())
    {
        //this is us, stash the info
        m_clientData[playerIndex] = data;

        //add a local controller
        entity.addComponent<Player>().playerNumber = (data.actor.type == ActorID::PlayerOne) ? 0 : 1;
        entity.getComponent<Player>().spawnPosition = { data.spawnX, data.spawnY };
        m_playerInputs[playerIndex].setPlayerEntity(entity);

        entity.addComponent<CollisionComponent>().addHitbox(PlayerBounds, CollisionType::Player);
        entity.getComponent<CollisionComponent>().addHitbox(PlayerFoot, CollisionType::Foot);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Player);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::PlayerMask);
        entity.addComponent<xy::QuadTreeItem>().setArea(entity.getComponent<CollisionComponent>().getLocalBounds());


        //temp for now just to flash player when invincible
        entity.addComponent<xy::Callback>().function = 
            [](xy::Entity entity, float dt)
        {
            static sf::Color colour = sf::Color::White;
            if (entity.getComponent<Player>().sync.timer > 0
                && entity.getComponent<Player>().sync.state != Player::State::Dying)
            {
                static float flashTime = 0.0625f;
                flashTime -= dt;

                if (flashTime < 0)
                {
                    colour.a = (colour.a == 20) ? 255 : 20;
                    entity.getComponent<xy::Sprite>().setColour(colour);
                    flashTime = 0.0625f;
                }
            }
            else
            {
                entity.getComponent<xy::Sprite>().setColour(sf::Color::White);
                //entity.getComponent<xy::Callback>().active = false;
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
    for (auto i = 0u; i < m_sharedData.playerCount; ++i)
    {
        m_playerInputs[i].setEnabled(false);
    }

    //clear remaining actors (should just be collectables / bubbles)
    //as well as any geometry
    xy::Command cmd;
    cmd.targetFlags = CommandID::MapItem;
    cmd.action = [&](xy::Entity entity, float)
    {
        m_scene.destroyEntity(entity);
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    
    cmd.targetFlags = CommandID::SceneBackground;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::Callback>().active = true;
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
   
    cmd.targetFlags = CommandID::SceneMusic;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::AudioEmitter>().setPitch(1.f);
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    cmd.targetFlags = CommandID::PlayerOne | CommandID::PlayerTwo;
    cmd.action = [&](xy::Entity entity, float)
    {
        entity.getComponent<xy::ParticleEmitter>().stop();
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    m_scene.update(0.f); //force the command right away


    if (loadScene(data, { 0.f, -(MapBounds.height) }))
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
                animator.dest.y = (MapBounds.height + 128.f);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        //make sure we don't jump mid animation
        //because there's no collision to stop us
        cmd.targetFlags = CommandID::PlayerOne | CommandID::PlayerTwo;
        cmd.action = [](xy::Entity entity, float)
        {
            if (entity.hasComponent<Player>())
            {
                entity.getComponent<Player>().sync.state = Player::State::Disabled;
            }
            entity.getComponent<AnimationController>().nextAnimation = 
                (entity.getComponent<AnimationController>().currentAnim == AnimationController::Dead) ? 
                AnimationController::Dead : AnimationController::Idle;
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        cmd.targetFlags = CommandID::TowerDude;
        cmd.action = [](xy::Entity entity, float)
        {
            const auto& tx = entity.getComponent<xy::Transform>();
            auto pos = tx.getPosition();
            const float travel = xy::DefaultSceneSize.y / MapsToWin;
            if (pos.y > travel)
            {
                pos.y -= travel;
                entity.getComponent<MapAnimator>().dest = pos;
                entity.getComponent<MapAnimator>().state = MapAnimator::State::Active;
                entity.getComponent<xy::SpriteAnimation>().play(0);
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        m_scene.setSystemActive<CollisionSystem>(false);
        m_scene.setSystemActive<xy::InterpolationSystem>(false);
    }
}

void GameState::spawnMapActors()
{
    xy::EmitterSettings emitterSettings;
    emitterSettings.loadFromFile("assets/particles/angry.xyp", m_textureResource);
    
    for (auto i = 0; i < m_mapData.NPCCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setOrigin(WhirlyBobOrigin);
        entity.addComponent<Actor>() = m_mapData.NPCs[i];
        entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor | CommandID::MapItem;
        entity.addComponent<xy::NetInterpolate>();      

        /*
        Even though these are only actors NPCs require collision with the player to
        ensure the player state is properly updated during reconciliation
        */
        entity.addComponent<xy::QuadTreeItem>().setArea(WhirlyBobBounds);
        switch (m_mapData.NPCs[i].type)
        {
        default:
            //add missing texture or placeholder
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::WhirlyBob];
            entity.getComponent<xy::Sprite>().setColour(sf::Color::Black);
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::WhirlyBob];
            break;
        case ActorID::Whirlybob:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::WhirlyBob];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::WhirlyBob];
            entity.addComponent<CollisionComponent>().addHitbox(WhirlyBobBounds, CollisionType::NPC);
            entity.getComponent<xy::Transform>().setOrigin(WhirlyBobOrigin);
            break;
        case ActorID::Clocksy:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Clocksy];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::Clocksy];
            entity.addComponent<CollisionComponent>().addHitbox(ClocksyBounds, CollisionType::NPC);
            entity.getComponent<xy::Transform>().setOrigin(ClocksyOrigin);
            break;
        case ActorID::Balldock:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Balldock];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::Balldock];
            entity.addComponent<CollisionComponent>().addHitbox(BalldockBounds, CollisionType::NPC);
            entity.getComponent<xy::Transform>().setOrigin(BalldockOrigin);
            break;
        case ActorID::Squatmo:
            entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Squatmo];
            entity.addComponent<AnimationController>() = m_animationControllers[SpriteID::Squatmo];
            entity.addComponent<CollisionComponent>().addHitbox(SquatmoBounds, CollisionType::NPC);
            entity.getComponent<xy::Transform>().setOrigin(SquatmoOrigin);
            break;
        }
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::NPC);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Player);

        entity.addComponent<xy::Drawable>().setDepth(-3); //behind bubbles
        entity.addComponent<xy::SpriteAnimation>().play(0);
        entity.addComponent<xy::ParticleEmitter>().settings = emitterSettings;
    }

    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/power_ups.spt", m_textureResource);

    for (auto i = 0; i < m_mapData.crateCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setOrigin(CrateOrigin);
        entity.addComponent<Actor>() = m_mapData.crates[i];
        entity.addComponent<xy::CommandTarget>().ID = CommandID::NetActor | CommandID::MapItem;
        entity.addComponent<xy::NetInterpolate>();
        entity.addComponent<xy::Sprite>() = spriteSheet.getSprite("crate");
        entity.addComponent<xy::Drawable>().setDepth(-1);
        entity.addComponent<CollisionComponent>().addHitbox(CrateBounds, CollisionType::Crate);
        entity.getComponent<CollisionComponent>().setCollisionCategoryBits(CollisionFlags::Crate);
        entity.getComponent<CollisionComponent>().setCollisionMaskBits(CollisionFlags::Player);
        entity.addComponent<xy::SpriteAnimation>().play(0);
        entity.addComponent<AnimationController>();
        entity.addComponent<xy::QuadTreeItem>().setArea(CrateBounds);

        std::cout << "spawned crate" << std::endl;
    }
}

void GameState::spawnWarning()
{
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(MapBounds.width / 2.f, MapBounds.height /  2.f);
    entity.getComponent<xy::Transform>().move(xy::DefaultSceneSize.x / 1.8f, -180.f);
    entity.addComponent<xy::Text>(m_fontResource.get("assets/fonts/Cave-Story.ttf")).setString("Hurry Up!");
    entity.getComponent<xy::Text>().setCharacterSize(200);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = Flasher(m_scene);

    xy::Command cmd;
    cmd.targetFlags = CommandID::SceneMusic;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::AudioEmitter>().pause();
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    getContext().appInstance.getMessageBus().post<MapEvent>(MessageID::MapMessage)->type = MapEvent::HurryUp;
}

void GameState::spawnRoundSkip()
{
    float spacing = MapBounds.width / 5.f;
    float scale = spacing / BubbleBounds.width;
    
    for (auto i = 0; i < 5; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(i * spacing, xy::Util::Random::value(-180.f, 0.f));
        entity.getComponent<xy::Transform>().setScale(scale, scale);
        entity.addComponent<xy::Sprite>() = m_sprites[SpriteID::Bonus];
        entity.addComponent<xy::Drawable>();
        entity.addComponent<xy::SpriteAnimation>().play(i);
        entity.addComponent<xy::Callback>().active = true;
        entity.getComponent<xy::Callback>().function = BallDropper(m_scene);
    }

    xy::Command cmd;
    cmd.targetFlags = CommandID::SceneMusic;
    cmd.action = [](xy::Entity entity, float)
    {
        entity.getComponent<xy::AudioEmitter>().pause();
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    auto* msg = getContext().appInstance.getMessageBus().post<MapEvent>(MessageID::MapMessage);
    msg->type = MapEvent::BonusSwitch;
}

void GameState::spawnTowerDude(sf::Int16 actorType)
{
    auto towerEnt = m_scene.createEntity(); //little dude climbing tower
    towerEnt.addComponent<xy::Transform>();

    if (actorType == ActorID::PlayerOne)
    {
        towerEnt.addComponent<xy::Sprite>() = m_sprites[SpriteID::TowerDudeOne];
        towerEnt.getComponent<xy::Transform>().setPosition(TowerSpawnOne);
        towerEnt.addComponent<Actor>().type = ActorID::TowerOne;
    }
    else
    {
        towerEnt.addComponent<xy::Sprite>() = m_sprites[SpriteID::TowerDudeTwo];
        towerEnt.getComponent<xy::Transform>().setPosition(TowerSpawnTwo);
        towerEnt.addComponent<Actor>().type = ActorID::TowerTwo;
    }
    towerEnt.addComponent<xy::Drawable>().setDepth(10);
    towerEnt.addComponent<xy::SpriteAnimation>();
    towerEnt.addComponent<MapAnimator>().state = MapAnimator::State::Static;
    towerEnt.getComponent<MapAnimator>().speed = 50.f;
    towerEnt.addComponent<xy::CommandTarget>().ID = CommandID::TowerDude;
    towerEnt.addComponent<xy::Callback>().function = TowerGuyCallback(m_scene);
}

void GameState::updateUI(const InventoryUpdate& data)
{
    xy::Command scoreCmd;
    scoreCmd.action = [data](xy::Entity entity, float)
    {
        entity.getComponent<xy::Text>().setString(std::to_string(data.score));
    };

    //keep a copy of our score for the scoreboard
    if (m_sharedData.playerCount == 1)
    {
        if (data.playerID == m_clientData[0].playerNumber)
        {
            m_sharedData.scores[0] = std::to_string(data.score);
        }
    }
    else
    {
        m_sharedData.scores[data.playerID] = std::to_string(data.score);
    }

    //check if greater than high score and update
    if (data.score > m_scores.getProperties()[0].getValue<sf::Int32>())
    {
        m_scores.findProperty("hiscore")->setValue(static_cast<sf::Int32>(data.score));
        m_scores.save(xy::FileSystem::getConfigDirectory(dataDir) + scoreFile);

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
        scoreEnt.addComponent<xy::Text>(m_fontResource.get("assets/fonts/Cave-Story.ttf"));
        scoreEnt.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);

        if (data.amount == std::numeric_limits<sf::Uint32>::max())
        {
            //this is a 1up bonus
            scoreEnt.getComponent<xy::Text>().setString("1 UP!");
            scoreEnt.getComponent<xy::Text>().setCharacterSize(56);
            scoreEnt.addComponent<ScoreTag>().colour = sf::Color::Red;

            auto* msg = getContext().appInstance.getMessageBus().post<PlayerEvent>(MessageID::PlayerMessage);
            msg->type = PlayerEvent::GotExtraLife;
            msg->entity = entity;
        }
        else
        {
            scoreEnt.getComponent<xy::Text>().setString(std::to_string(data.amount));
            scoreEnt.getComponent<xy::Text>().setCharacterSize(40);
            scoreEnt.addComponent<ScoreTag>();
        }
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

    //only show score if there's an amount to see
    if (data.amount)
    {
        m_scene.getSystem<xy::CommandSystem>().sendCommand(textCommand);
    }

    if (data.lives == 0)
    {
        //kill the climbing dude :(
        auto playerNumber = data.playerID;

        xy::Command cmd;
        cmd.targetFlags = CommandID::TowerDude;
        cmd.action = [&, playerNumber](xy::Entity entity, float)
        {
            auto id = (playerNumber == 0) ? ActorID::TowerOne : ActorID::TowerTwo;
            if (entity.getComponent<Actor>().type == id)
            {
                entity.getComponent<xy::SpriteAnimation>().play(1);
                entity.getComponent<xy::Callback>().active = true;
            }
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }


    xy::Command bonusCommand;
    bonusCommand.targetFlags = CommandID::BonusBall;
    bonusCommand.action = [data](xy::Entity entity, float)
    {
        const auto& bonus = entity.getComponent<BonusUI>();
        if (bonus.playerID == data.playerID)
        {
            if (bonus.value & data.bonusFlags)
            {
                entity.getComponent<xy::Sprite>().setColour(sf::Color::White);
            }
            else
            {
                entity.getComponent<xy::Sprite>().setColour(sf::Color::Transparent);
            }
        }
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(bonusCommand);
}

void GameState::giveHat(sf::Uint8 player)
{
    xy::Command cmd;
    cmd.targetFlags = player == 0 ? CommandID::PlayerOne : CommandID::PlayerTwo;
    cmd.action = [&](xy::Entity entity, float)
    {
        auto hatEnt = m_scene.createEntity();
        hatEnt.addComponent<xy::Transform>();// .setOrigin(PlayerOrigin);
        hatEnt.addComponent<xy::Sprite>() = m_sprites[SpriteID::MagicHat];
        hatEnt.addComponent<xy::Drawable>().setDepth(1);
        hatEnt.addComponent<xy::SpriteAnimation>();
        hatEnt.addComponent<xy::CommandTarget>().ID = CommandID::Hat | CommandID::MapItem;
        hatEnt.addComponent<AnimationController>() = m_animationControllers[SpriteID::MagicHat];
        hatEnt.addComponent<xy::Callback>().active = true;
        hatEnt.getComponent<xy::Callback>().function = [entity](xy::Entity hat, float)
        {
            hat.getComponent<AnimationController>().nextAnimation = entity.getComponent<AnimationController>().nextAnimation;
        };
        hatEnt.addComponent<Actor>().id = hatEnt.getIndex();
        hatEnt.getComponent<Actor>().type = ActorID::MagicHat;

        entity.getComponent<xy::Transform>().addChild(hatEnt.getComponent<xy::Transform>());
        entity.getComponent<xy::ParticleEmitter>().start();

        //let the world know what happened
        auto* msg = getContext().appInstance.getMessageBus().post<PlayerEvent>(MessageID::PlayerMessage);
        msg->entity = entity;
        msg->type = PlayerEvent::GotHat;
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

void GameState::takeHat(sf::Uint8 player)
{
    xy::Command cmd;
    cmd.targetFlags = player == 0 ? CommandID::PlayerOne : CommandID::PlayerTwo;
    cmd.action = [&](xy::Entity entity, float)
    {
        //let the world know what happened
        auto* msg = getContext().appInstance.getMessageBus().post<PlayerEvent>(MessageID::PlayerMessage);
        msg->entity = entity;
        msg->type = PlayerEvent::LostHat;

        entity.getComponent<xy::ParticleEmitter>().stop();
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

    //delete hat
    cmd.targetFlags = CommandID::Hat;
    cmd.action = [&](xy::Entity entity, float)
    {
        m_scene.destroyEntity(entity);
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

void GameState::updateLevelDisplay(sf::Uint8 level)
{
    xy::Command cmd;
    cmd.targetFlags = CommandID::LevelCounter;
    cmd.action = [&, level](xy::Entity entity, float)
    {
        entity.getComponent<xy::Text>().setString(std::to_string(level));

        //sending a command 1 frame later allows text time to update bounds
        xy::Command c;
        c.targetFlags = CommandID::LevelCounter;
        c.action = [](xy::Entity ent, float)
        {
            auto bounds = ent.getComponent<xy::Text>().getLocalBounds();
            auto pos = ent.getComponent<xy::Transform>().getPosition();
            pos.x = (MapBounds.width - bounds.width) / 2.f;
            ent.getComponent<xy::Transform>().setPosition(pos);
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(c); //meta.
    };
    m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

void GameState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingScreen.update(dt);
    rw.draw(m_loadingScreen);
}

void GameState::transitionToEnd()
{
    m_client.disconnect();
    
    //fade to black
    auto entity = m_scene.createEntity();
    m_textureResource.setFallbackColour(sf::Color::Black);
    auto bounds = entity.addComponent<xy::Sprite>(m_textureResource.get("black")).getTextureBounds();
    entity.getComponent<xy::Sprite>().setColour({ 255,255,255,0 });

    entity.addComponent<xy::Transform>().setPosition(m_scene.getActiveCamera().getComponent<xy::Transform>().getPosition());
    entity.getComponent<xy::Transform>().setScale(xy::DefaultSceneSize.x / bounds.width, xy::DefaultSceneSize.y / bounds.height);
    entity.getComponent<xy::Transform>().move(-xy::DefaultSceneSize / 2.f);
    entity.addComponent<xy::Drawable>().setDepth(12);

    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = 
        [&](xy::Entity ent, float dt)
    {
        static const float fadeTime = 2.f;
        static float currentTime = 0.f;

        currentTime += dt;
        float alpha = std::min(1.f, currentTime / fadeTime);
        
        sf::Uint8 alphaB = static_cast<sf::Uint8>(alpha * 255.f);
        ent.getComponent<xy::Sprite>().setColour({ 255,255,255,alphaB });

        if (alphaB == 255)
        {
            requestStackPop();
            requestStackPush(StateID::GameComplete);
        }
        
        //fade out music
        xy::Command cmd;
        cmd.targetFlags = CommandID::SceneMusic;
        cmd.action = [alpha](xy::Entity soundEnt, float)
        {
            soundEnt.getComponent<xy::AudioEmitter>().setVolume((1.f - alpha) * 0.25f);
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);

        //and text
        cmd.targetFlags = CommandID::UIElement;
        cmd.action = [alphaB](xy::Entity textEnt, float)
        {
            auto colour = textEnt.getComponent<xy::Text>().getFillColour();
            colour.a = 255 - alphaB;
            textEnt.getComponent<xy::Text>().setFillColour(colour);
        };
        m_scene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    };
}