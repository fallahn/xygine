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

#include "EndingDirector.hpp"
#include "EndingMessages.hpp"
#include "SpringFlower.hpp"

#include <xyginext/resources/Resource.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/AudioEmitter.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <xyginext/util/Random.hpp>
#include <xyginext/util/Math.hpp>

namespace
{
    const sf::Vector2f PrincessPosition(1216.f, 40.f);
    const sf::Vector2f FoodPosition(44.f, 12.f);
    const sf::Vector2f PlayerPosition(720.f, 400.f);
    const sf::Vector2f PausePosition(128.f, 436.f);

    struct CallbackData final
    {
        xy::Scene* scene = nullptr;
        xy::MessageBus* messageBus = nullptr;
        xy::Entity foodEnt;
    };

    struct SpeechCallback final
    {
    public:
        explicit SpeechCallback(sf::Uint32 id, CallbackData data)
            : m_id(id), m_data(data)
        {
            XY_ASSERT(data.scene, "Missing Scene Pointer");
            XY_ASSERT(data.messageBus, "Missing Message Bus");
        }

        void operator()(xy::Entity entity, float dt)
        {
            auto oldTime = m_currentTime;
            m_currentTime += dt;
            if (m_currentTime > 0.6f && oldTime < 0.6f)
            {
                entity.getComponent<xy::Transform>().setScale(4.f, 4.f);
                entity.getComponent<xy::AudioEmitter>().play();
            }
            if (m_currentTime > MaxTime)
            {
                m_data.scene->destroyEntity(entity);
                m_data.scene->destroyEntity(m_data.foodEnt);

                auto* msg = m_data.messageBus->post<SpeechEvent>(Messages::SpeechMessage);
                msg->id = static_cast<SpeechEvent::Food>(m_id);
            }
        }

    private:
        sf::Uint32 m_id;
        CallbackData m_data;
        float m_currentTime = 0.f;
        static constexpr float MaxTime = 3.2f;
    };
}

EndingDirector::EndingDirector(xy::AudioResource& ar, xy::TextureResource& tr, xy::MessageBus& mb)
    : m_soundResource   (ar),
    m_textureResource   (tr),
    m_messageBus        (mb)
{
    m_spriteSheet.loadFromFile("assets/sprites/ending_food.spt", m_textureResource);
}

//public
void EndingDirector::handleMessage(const xy::Message& msg)
{
    if (msg.id == Messages::SpriteMessage)
    {
        const auto& data = msg.getData<SpriteEvent>();
        switch (data.event)
        {
        default: break;
        case SpriteEvent::ReachedTop:
            spawnBubble(SpeechEvent::Burger);
            break;
        case SpriteEvent::BurgerTaken:
            spawnBubble(SpeechEvent::Pizza);
            break;
        case SpriteEvent::PizzaTaken:
            spawnBubble(SpeechEvent::Icecream);
            break;
        case SpriteEvent::IcecreamTaken:
            spawnBubble(SpeechEvent::Cake);
            break;
        case SpriteEvent::Shout:
            //knock player off tower
        {
            xy::Command cmd;
            cmd.targetFlags = Command::Player;
            cmd.action = [&](xy::Entity entity, float)
            {
                entity.getComponent<xy::SpriteAnimation>().play(1);
                entity.getComponent<xy::Callback>().function = 
                    [&](xy::Entity ent, float  dt)
                {
                    static const float Gravity = 2600.f;
                    ent.getComponent<xy::Transform>().move(ent.getComponent<sf::Vector2f>() * dt);
                    ent.getComponent<sf::Vector2f>().y += Gravity * dt;

                    if (ent.getComponent<xy::Transform>().getPosition().y > xy::DefaultSceneSize.y)
                    {
                        //store this for later
                        m_playerSprite = ent.getComponent<xy::Sprite>();
                        
                        //we went out of bounds
                        getScene().destroyEntity(ent);

                        //raise message to switch to wide shot
                        auto* msg = m_messageBus.post<SpriteEvent>(Messages::SpriteMessage);
                        msg->event = SpriteEvent::ReachedBottom;
                    }
                };
                entity.getComponent<xy::Callback>().active = true;
            };
            sendCommand(cmd);

            auto entity = getScene().createEntity();
            entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
            entity.addComponent<xy::CommandTarget>().ID = Command::Clearable;
            playSound(SoundID::Shout, entity);
        }
            break;
        case SpriteEvent::ReachedBottom:
            spawnWideShot();
            break;
        }
    }
    else if (msg.id == Messages::SpeechMessage)
    {
        const auto& data = msg.getData<SpeechEvent>();
        switch (data.id)
        {
        default:break;
        case SpeechEvent::Burger:
        case SpeechEvent::Pizza:
        case SpeechEvent::Icecream:
            spawnFood(data.id);
            break;
        case SpeechEvent::Cake:
            spawnPause();
            break;
        }
    }
}

void EndingDirector::handleEvent(const sf::Event&) {}

void EndingDirector::process(float) {}

//private
void EndingDirector::spawnBubble(sf::Uint32 id)
{
    auto& scene = getScene();
    auto bubbleEnt = scene.createEntity();

    auto foodEnt = scene.createEntity();
    foodEnt.addComponent<xy::Drawable>().setDepth(2);
    foodEnt.addComponent<xy::Transform>().setPosition(FoodPosition);
    foodEnt.addComponent<xy::CommandTarget>().ID = Command::Clearable;
    switch (id)
    {
    default:break;
    case SpeechEvent::Burger:
        foodEnt.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("burger");
        bubbleEnt.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q1.wav"));
        break;
    case SpeechEvent::Pizza:
        foodEnt.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("pizza");
        bubbleEnt.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q2.wav"));
        break;
    case SpeechEvent::Icecream:
        foodEnt.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("icecream");
        bubbleEnt.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q1.wav"));
        break;
    case SpeechEvent::Cake:
        foodEnt.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("cake");
        bubbleEnt.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q3.wav"));
        break;
    }
    bubbleEnt.getComponent<xy::AudioEmitter>().setVolume(80.f);
    bubbleEnt.getComponent<xy::AudioEmitter>().setMinDistance(1920.f);
    bubbleEnt.addComponent<xy::CommandTarget>().ID = Command::Clearable;

    
    bubbleEnt.addComponent<xy::Transform>().setPosition(PrincessPosition);
    bubbleEnt.getComponent<xy::Transform>().setScale(0.f, 4.f);
    bubbleEnt.getComponent<xy::Transform>().addChild(foodEnt.getComponent<xy::Transform>());
    auto bounds = bubbleEnt.addComponent<xy::Sprite>(m_textureResource.get("assets/images/speech.png")).getTextureBounds();
    bounds.width /= 2.f;
    bounds.left = bounds.width;
    bubbleEnt.getComponent<xy::Sprite>().setTextureRect(bounds);
    bubbleEnt.addComponent<xy::Drawable>().setDepth(1);
    bubbleEnt.addComponent<xy::Callback>().active = true;

    CallbackData data;
    data.scene = &scene;
    data.messageBus = &m_messageBus;
    data.foodEnt = foodEnt;

    bubbleEnt.getComponent<xy::Callback>().function = SpeechCallback(id, data);
}

void EndingDirector::spawnFood(sf::Uint32 id)
{
    auto& scene = getScene();

    auto entity = scene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(PlayerPosition);
    entity.getComponent<xy::Transform>().setScale(2.f, 2.f);
    entity.addComponent<xy::Drawable>().setDepth(2);
    entity.addComponent<xy::CommandTarget>().ID = Command::Clearable;

    switch (id)
    {
    default:break;
    case SpeechEvent::Burger:
        entity.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("burger");
        break;
    case SpeechEvent::Pizza:
        entity.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("pizza");
        break;
    case SpeechEvent::Icecream:
        entity.addComponent<xy::Sprite>() = m_spriteSheet.getSprite("icecream");
        break;
    case SpeechEvent::Cake:

        break;
    }
    entity.addComponent<sf::Vector2f>(310.f, -1200.f);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = 
        [&, id](xy::Entity foodEnt, float dt)
    {
        static const float Gravity = 2800.f;
        foodEnt.getComponent<sf::Vector2f>().y += Gravity * dt;
        foodEnt.getComponent<xy::Transform>().move(foodEnt.getComponent<sf::Vector2f>() * dt);

        if (foodEnt.getComponent<xy::Transform>().getPosition().y > PlayerPosition.y)
        {
            scene.destroyEntity(foodEnt);

            //create particle / sound effect
            auto particleEnt = scene.createEntity();
            particleEnt.addComponent<xy::Transform>().setPosition(foodEnt.getComponent<xy::Transform>().getPosition() + sf::Vector2f(32.f, 32.f));
            particleEnt.addComponent<xy::ParticleEmitter>().settings.loadFromFile("assets/particles/score.xyp", m_textureResource);
            particleEnt.getComponent<xy::ParticleEmitter>().start();
            particleEnt.addComponent<xy::CommandTarget>().ID = Command::Clearable;
            playSound(SoundID::Pop, particleEnt);

            auto* msg = m_messageBus.post<SpriteEvent>(Messages::SpriteMessage);
            switch (id)
            {
            default:break;
            case SpeechEvent::Burger:
                msg->event = SpriteEvent::BurgerTaken;
                break;
            case SpeechEvent::Pizza:
                msg->event = SpriteEvent::PizzaTaken;
                break;
            case SpeechEvent::Icecream:
                msg->event = SpriteEvent::IcecreamTaken;
                break;
            }
        }
    };

    
    playSound(SoundID::Collect, entity);
}

void EndingDirector::spawnPause()
{
    auto& scene = getScene();
    auto entity = scene.createEntity();
    entity.addComponent<xy::CommandTarget>().ID = Command::Clearable;
    entity.addComponent<xy::Transform>().setPosition(PausePosition);
    entity.getComponent<xy::Transform>().setScale(4.f, 4.f);
    auto bounds = entity.addComponent<xy::Sprite>(m_textureResource.get("assets/images/speech.png")).getTextureBounds();
    bounds.width /= 2.f;
    entity.getComponent<xy::Sprite>().setTextureRect(bounds);
    entity.addComponent<xy::Drawable>().setDepth(2);
    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = 
        [&](xy::Entity bubbleEnt, float dt)
    {
        static const float MaxTime = 3.f;
        static float currentTime = 0.f;
        currentTime += dt;
        if (currentTime > MaxTime)
        {
            getScene().destroyEntity(bubbleEnt);

            xy::Command cmd;
            cmd.targetFlags = Command::Princess;
            cmd.action = [&](xy::Entity princessEnt, float)
            {
                princessEnt.getComponent<xy::ParticleEmitter>().stop();
                princessEnt.getComponent<xy::SpriteAnimation>().stop();
                princessEnt.getComponent<xy::SpriteAnimation>().play(3);

                princessEnt.getComponent<xy::Callback>().function = 
                    [&](xy::Entity ent, float dtt)
                {
                    static float move = 6.f;
                    static float moveTimer = 0.f;
                    static float shakeTimer = 0.f;
                    static const float moveTime = 0.025f;
                    static const float shakeTime = 2.f;

                    moveTimer += dtt;
                    if (moveTimer > moveTime)
                    {
                        moveTimer = 0.f;
                        ent.getComponent<xy::Transform>().move(move, 0.f);
                        move = -move;
                    }

                    shakeTimer += dtt;
                    if (shakeTimer > shakeTime)
                    {
                        ent.getComponent<xy::SpriteAnimation>().play(1);
                        ent.getComponent<xy::Callback>().active = false;

                        auto* msg = m_messageBus.post<SpriteEvent>(Messages::SpriteMessage);
                        msg->event = SpriteEvent::Shout;
                    }
                };
                princessEnt.getComponent<xy::Callback>().active = true;

                auto soundEnt = getScene().createEntity();
                soundEnt.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
                soundEnt.addComponent<xy::CommandTarget>().ID = Command::Clearable;
                playSound(SoundID::Angry, soundEnt);
            };
            sendCommand(cmd);
        }
    };
}

void EndingDirector::spawnWideShot()
{
    auto& scene = getScene();

    auto backgroundEnt = scene.createEntity();
    backgroundEnt.addComponent<xy::Transform>();
    backgroundEnt.addComponent<xy::Drawable>().setDepth(10);
    backgroundEnt.addComponent<xy::Sprite>(m_textureResource.get("assets/images/game_over.png"));

    auto grassEnt = scene.createEntity();
    auto& texture = m_textureResource.get("assets/images/grass.png");
    texture.setRepeated(true);
    auto bounds = grassEnt.addComponent<xy::Sprite>(texture).getTextureBounds();
    bounds.width = xy::DefaultSceneSize.x;
    grassEnt.getComponent<xy::Sprite>().setTextureRect(bounds);
    grassEnt.addComponent<xy::Transform>().setPosition(0.f, xy::DefaultSceneSize.y - bounds.height);
    grassEnt.addComponent<xy::Drawable>().setDepth(20);


    //flowers
    float xPos = xy::Util::Random::value(60.f, 90.f);
    sf::Uint8 darkness = 140;
    sf::Int32 depth = 13;
    for (auto i = 0; i < 12; ++i)
    {
        auto entity = scene.createEntity();
        entity.addComponent<SpringFlower>(-256.f).headPos.x += xy::Util::Random::value(-12.f, 14.f);
        entity.getComponent<SpringFlower>().textureRect = { 0.f, 0.f, 64.f, 256.f };
        entity.getComponent<SpringFlower>().colour = { darkness, darkness, darkness };
        entity.addComponent<xy::Drawable>(m_textureResource.get("assets/images/flower.png")).setDepth(depth);
        entity.addComponent<xy::Transform>().setPosition(xPos, xy::DefaultSceneSize.y + xy::Util::Random::value(0.f, 64.f));

        if ((i % 4) == 0)
        {
            xPos += xy::Util::Random::value(400.f, 560.f);
            darkness = 160;
            depth = 13;
        }
        else
        {
            auto value = xy::Util::Random::value(20.f, 80.f);
            xPos += value;
            darkness = xy::Util::Math::clamp(sf::Uint8(darkness + static_cast<sf::Uint8>(value - 20.f)), sf::Uint8(140), sf::Uint8(255));
            depth += 2;
        }
    }

    if (m_playerSprite.getAnimationCount() > 0)
    {
        auto playerEnt = scene.createEntity();
        playerEnt.addComponent<xy::Transform>().setPosition(235.f, 240.f);
        playerEnt.addComponent<xy::Sprite>() = m_playerSprite;
        playerEnt.addComponent<xy::Drawable>().setDepth(11);
        playerEnt.addComponent<xy::SpriteAnimation>().play(1);
        playerEnt.addComponent<sf::Vector2f>(-100.f, 500.f);
        playerEnt.addComponent<xy::Callback>().active = true;
        playerEnt.getComponent<xy::Callback>().function =
            [&](xy::Entity entity, float dt)
        {
            static const float Gravity = 2600.f;

            auto& tx = entity.getComponent<xy::Transform>();
            tx.move(entity.getComponent<sf::Vector2f>() * dt);
            entity.getComponent<sf::Vector2f>().y += Gravity * dt;

            if (tx.getPosition().y > xy::DefaultSceneSize.y)
            {

                //we went out of bounds
                getScene().destroyEntity(entity);

                //create dust particles
                auto dustEnt = scene.createEntity();
                dustEnt.addComponent<xy::Transform>().setPosition(tx.getPosition().x, xy::DefaultSceneSize.y - 64.f);
                dustEnt.addComponent<xy::ParticleEmitter>().settings.loadFromFile("assets/particles/dust_hit.xyp", m_textureResource);
                dustEnt.getComponent<xy::ParticleEmitter>().start();

                playSound(SoundID::Land, dustEnt);
            }
        };
    }

    xy::Command cmd;
    cmd.targetFlags = Command::Clearable;
    cmd.action = [&](xy::Entity entity, float)
    {
        getScene().destroyEntity(entity);
    };
    sendCommand(cmd);
}

void EndingDirector::playSound(sf::Uint32 id, xy::Entity entity)
{
    switch (id)
    {
    default: break;
    case SoundID::Collect:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/collect.wav"));
        break;
    case SoundID::Land:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/powerup_pop.wav"));
        break;
    case SoundID::Pop:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/pop.wav"));
        break;
    case SoundID::Q1:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q1.wav"));
        break;
    case SoundID::Q2:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q2.wav"));
        break;
    case SoundID::Q3:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/q3.wav"));
        break;
    case SoundID::Shout:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/shout.wav"));
        break;
    case SoundID::Angry:
        entity.addComponent<xy::AudioEmitter>().setSource(m_soundResource.get("assets/sound/angry.wav"));
        break;
    }    
    
    entity.getComponent<xy::AudioEmitter>().setVolume(80.f);
    entity.getComponent<xy::AudioEmitter>().setMinDistance(1920.f);
    //entity.getComponent<xy::AudioEmitter>().setRelativeTolistener(true);

    entity.getComponent<xy::AudioEmitter>().play();
}