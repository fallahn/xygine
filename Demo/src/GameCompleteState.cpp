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

#include <xyginext/ecs/components/AudioEmitter.hpp>
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
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/SpriteAnimator.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/TextRenderer.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>

#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/util/Vector.hpp>

#include <SFML/Window/Event.hpp>

GameCompleteState::GameCompleteState(xy::StateStack& stack, xy::State::Context ctx, SharedStateData& sd)
    : xy::State (stack, ctx),
    m_sharedData(sd),
    m_scene     (ctx.appInstance.getMessageBus())
{
    loadAssets();
    loadScene();
    loadUI();
}

//public
bool GameCompleteState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased
        || evt.type == sf::Event::JoystickButtonReleased)
    {
        //TODO make this skip ending and display summary

        requestStackClear();
        requestStackPush(StateID::MainMenu);
    }
    
    //forward scene event
    m_scene.forwardEvent(evt);

    //TODO forward UI event
    return false;
}

void GameCompleteState::handleMessage(const xy::Message& msg)
{
    m_scene.forwardMessage(msg);
}

bool GameCompleteState::update(float dt)
{
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
    m_scene.addSystem<xy::SpriteAnimator>(mb);
    m_scene.addSystem<xy::CallbackSystem>(mb);
    m_scene.addSystem<xy::SpriteSystem>(mb);
    m_scene.addSystem<xy::RenderSystem>(mb);
    m_scene.addSystem<xy::ParticleSystem>(mb);
    m_scene.addSystem<xy::TextRenderer>(mb);
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
        currentTime -= dt;
        float alpha = std::max(0.f, 255.f * (currentTime / fadeTime));
        ent.getComponent<xy::Sprite>().setColour({ 255, 255, 255, static_cast<sf::Uint8>(alpha) });

        if (alpha == 0)
        {
            ent.getComponent<xy::Callback>().active = false;
        }
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
        [princessEnt](xy::Entity playerEnt, float dt) mutable
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
        }
    };

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
    entity.addComponent<xy::Text>(font).setString("Press Any Key To Continue");
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Red);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
}