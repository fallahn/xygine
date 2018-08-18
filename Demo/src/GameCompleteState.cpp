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

#include "GameCompleteState.hpp"
#include "SharedStateData.hpp"
#include "StateIDs.hpp"
#include "EndingDirector.hpp"
#include "EndingMessages.hpp"
#include "SpringFlower.hpp"
#include "Localisation.hpp"

#include <xyginext/gui/Gui.hpp>

#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/AudioListener.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>

#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/CallbackSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/systems/QuadTree.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    float inputDelay = 0.f;
}

GameCompleteState::GameCompleteState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sd)
    : xy::State     (stack, ctx),
    m_sharedData    (sd),
    m_scene         (ctx.appInstance.getMessageBus()),
    m_summaryShown  (false)
{
    inputDelay = 0.f;
    
    loadAssets();
    loadScene();
    loadUI();

    xy::App::setMouseCursorVisible(true);
}

//public
bool GameCompleteState::handleEvent(const sf::Event& evt)
{
    // Don't process events which the gui wants
    if (xy::Nim::wantsMouse() || xy::Nim::wantsKeyboard())
    {
        return true;
    }
    
    if (evt.type == sf::Event::KeyReleased
        || (evt.type == sf::Event::JoystickButtonReleased
            && inputDelay > 2.f))
    {
        //skips to end screen
        if (!m_summaryShown)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<SpriteEvent>(Messages::SpriteMessage);
            msg->event = SpriteEvent::ReachedBottom;
        }
    }
    
    //forward scene event
    m_scene.forwardEvent(evt);

    //forward UI event
    m_scene.getSystem<xy::UISystem>().handleEvent(evt);
    return false;
}

void GameCompleteState::handleMessage(const xy::Message& msg)
{
    if (msg.id == Messages::SpriteMessage)
    {
        const auto& data = msg.getData<SpriteEvent>();
        if (data.event == SpriteEvent::ReachedBottom)
        {
            showSummary();
        }
    }
    
    m_scene.forwardMessage(msg);
}

bool GameCompleteState::update(float dt)
{
    inputDelay += dt;
    m_scene.update(dt);
    return false;
}

void GameCompleteState::draw()
{
    auto& rt = getContext().renderWindow;

    rt.draw(m_scene);
}

//private
void GameCompleteState::loadAssets()
{
    auto& mb = getContext().appInstance.getMessageBus();
    m_scene.addSystem<SpringFlowerSystem>(mb);
    m_scene.addSystem<xy::QuadTree>(mb, sf::FloatRect({}, xy::DefaultSceneSize));
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::UISystem>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::TextSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);   
    m_scene.addSystem<xy::AudioSystem>(mb);

    m_scene.addDirector<EndingDirector>(m_soundResource, m_textureResource, mb);

    m_scene.getActiveCamera().getComponent<xy::AudioListener>().setVolume(1.f);
}

void GameCompleteState::loadScene()
{
    //background
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/end_scene.png"));
    entity.addComponent<xy::Drawable>().setDepth(-4);
    entity.addComponent<xy::Transform>().setScale(4.f, 4.f);

    m_textureResource.setFallbackColour(sf::Color::Black);
    entity = m_scene.createEntity();
    auto bounds = entity.addComponent<xy::Sprite>(m_textureResource.get("black")).getTextureBounds();
    entity.addComponent<xy::Drawable>().setDepth(10);
    entity.addComponent<xy::Transform>().setScale(xy::DefaultSceneSize.x / bounds.width, xy::DefaultSceneSize.y / bounds.height);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = [](xy::Entity ent, float dt)
    {
        static const float fadeTime = 5.f;
        static float currentTime = fadeTime;
        
        float alpha = std::max(0.f, 255.f * (currentTime / fadeTime));
        ent.getComponent<xy::Sprite>().setColour({ 255, 255, 255, static_cast<sf::Uint8>(alpha) });

        if (alpha == 0)
        {
            ent.getComponent<xy::Callback>().active = false;
        }
        currentTime -= dt;
    };

    //characters
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/princess.spt", m_textureResource);
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_one");
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::SpriteAnimation>().play(2);
    entity.addComponent<xy::Transform>().setOrigin(bounds.width / 2.f, bounds.height);
    entity.getComponent<xy::Transform>().setPosition(1090.f, 560.f);
    entity.getComponent<xy::Transform>().setScale(4.f, 4.f);
    entity.addComponent<xy::ParticleEmitter>().settings.loadFromFile("assets/particles/heart.xyp", m_textureResource);
    entity.addComponent<xy::CommandTarget>().ID = Command::Princess | Command::Clearable;
    entity.addComponent<xy::Callback>(); //used later by ending director

    auto princessEnt = entity;

    spriteSheet.loadFromFile("assets/sprites/tower_sprites.spt", m_textureResource);
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>() = spriteSheet.getSprite("player_one");
    entity.addComponent<xy::Drawable>().setDepth(1);
    entity.addComponent<xy::SpriteAnimation>().play(0);
    entity.addComponent<xy::Transform>().setScale(4.f, 4.f);
    entity.getComponent<xy::Transform>().setPosition(720.f, 980.f);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = 
        [&, princessEnt](xy::Entity playerEnt, float dt) mutable
    {
        static const float target = 340.f;
        playerEnt.getComponent<xy::Transform>().move(0.f, -140.f * dt);

        float diff = playerEnt.getComponent<xy::Transform>().getPosition().y - target;
        if (std::abs(diff) < 5.f)
        {
            playerEnt.getComponent<xy::Transform>().move(0.f, diff);
            playerEnt.getComponent<xy::SpriteAnimation>().stop();

            princessEnt.getComponent<xy::Transform>().setScale(-4.f, 4.f);
            princessEnt.getComponent<xy::ParticleEmitter>().start();

            playerEnt.getComponent<xy::Callback>().active = false;

            auto* msg = getContext().appInstance.getMessageBus().post<SpriteEvent>(Messages::SpriteMessage);
            msg->event = SpriteEvent::ReachedTop;
        }
    };
    entity.addComponent<xy::CommandTarget>().ID = Command::Player | Command::Clearable;
    entity.addComponent<sf::Vector2f>(-300.f, -200.f); //used for fall velocity

    entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.addComponent<xy::AudioEmitter>().setSource("assets/sound/music/night.ogg");
    entity.getComponent<xy::AudioEmitter>().setVolume(0.7f);
    entity.getComponent<xy::AudioEmitter>().setLooped(true);
    entity.getComponent<xy::AudioEmitter>().play();

    //apply the default view
    auto view = getContext().defaultView;
    auto& camera = m_scene.getActiveCamera().getComponent<xy::Camera>();
    camera.setView(view.getSize());
    camera.setViewport(view.getViewport());
}

void GameCompleteState::loadUI()
{
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");

    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x / 2.f, 1000.f);
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::SkipText]);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>().setDepth(2);
    entity.addComponent<xy::CommandTarget>().ID = Command::Clearable;
}

void GameCompleteState::showSummary()
{
    //show score
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");

    //score text
    for (auto i = 0u; i < m_sharedData.playerCount; ++i)
    {
        auto entity = m_scene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
        entity.addComponent<xy::Text>(font).setFillColour(sf::Color::Red);
        entity.addComponent<xy::Drawable>().setDepth(2);

        if (m_sharedData.playerCount == 1)
        {
            entity.getComponent<xy::Transform>().move(-360.f, -180.f);
            entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::Score] + m_sharedData.scores[0]);
            entity.getComponent<xy::Text>().setCharacterSize(140);
        }
        else
        {
            entity.getComponent<xy::Text>().setCharacterSize(100);
            if (i == 0)
            {
                entity.getComponent<xy::Transform>().move(-400.f, -180.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerOneScore] + m_sharedData.scores[0]);
            }
            else
            {
                entity.getComponent<xy::Transform>().move(-400.f, -100.f);
                entity.getComponent<xy::Text>().setString(Locale::Strings[Locale::PlayerTwoScore] + m_sharedData.scores[1]);
            }
        }
    }

    auto selectedID = m_scene.getSystem<xy::UISystem>().addSelectionCallback(
        [](xy::Entity entity)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        auto rect = sprite.getTextureRect();
        rect.top = 256.f;
        sprite.setTextureRect(rect);
    });
    auto unselectedID = m_scene.getSystem<xy::UISystem>().addSelectionCallback(
        [](xy::Entity entity)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        auto rect = sprite.getTextureRect();
        rect.top = 0.f;
        sprite.setTextureRect(rect);
    });

    //OK button text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString("OK");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(32.f, 45.f);

    //OK button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    auto bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 256.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>().setDepth(12);
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 256.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            requestStackClear();
            requestStackPush(StateID::MainMenu);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            requestStackClear();
            requestStackPush(StateID::MainMenu);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            requestStackClear();
            requestStackPush(StateID::MainMenu);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;


    m_summaryShown = true;
}
