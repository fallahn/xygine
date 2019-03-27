/*********************************************************************
(c) Matt Marchant 2019

This file is part of the xygine tutorial found at
https://github.com/fallahn/xygine

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
#include "States.hpp"
#include "CommandIDs.hpp"
#include "ResourceIDs.hpp"
#include "BallSystem.hpp"
#include "CollisionSystem.hpp"
#include "MessageIDs.hpp"
#include "ShapeUtils.hpp"
#include "FXDirector.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/components/BroadPhaseComponent.hpp>
#include <xyginext/ecs/components/Text.hpp>

#include <xyginext/ecs/systems/DynamicTreeSystem.hpp>
#include <xyginext/ecs/systems/SpriteSystem.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>
#include <xyginext/ecs/systems/RenderSystem.hpp>
#include <xyginext/ecs/systems/TextSystem.hpp>
#include <xyginext/ecs/systems/AudioSystem.hpp>
#include <xyginext/ecs/systems/ParticleSystem.hpp>

#include <xyginext/core/App.hpp>
#include <xyginext/util/Vector.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/gui/Gui.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

GameState::GameState(xy::StateStack& ss, xy::State::Context ctx) 
    : xy::State     (ss, ctx),
    m_gameScene     (ctx.appInstance.getMessageBus()),
    m_uiScene       (ctx.appInstance.getMessageBus()),
    m_blockCount    (0),
    m_score         (0),
    m_lives         (3)
{
    createScene();
    
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_gameScene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

    m_uiScene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_uiScene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

    ctx.appInstance.setMouseCursorVisible(false);
}

bool GameState::handleEvent(const sf::Event& evt)
{
    if (xy::Nim::wantsKeyboard() || xy::Nim::wantsMouse())
    {
        //prevents the game being updated when navigating an open console window
        return true;
    }

    //look for mouse move events and send them to the paddle
    if(evt.type == sf::Event::MouseMoved)
    {
        auto worldMousePosition = xy::App::getRenderWindow()->mapPixelToCoords({evt.mouseMove.x, evt.mouseMove.y});
        xy::Command cmd;
        cmd.targetFlags = CommandID::Paddle;
        cmd.action = [worldMousePosition](xy::Entity entity, float)
        {
            //clamp the X position in the screen area minus the sprite width
            float posX = worldMousePosition.x;
            auto spriteWidth = entity.getComponent<xy::Sprite>().getTextureBounds().width / 2.f;
            posX = std::max(spriteWidth, worldMousePosition.x);
            posX = std::min(posX, xy::DefaultSceneSize.x - spriteWidth);

            auto& tx = entity.getComponent<xy::Transform>();
            auto currentPosition = tx.getPosition();
            currentPosition.x = posX;
            tx.setPosition(currentPosition);
        };
        m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }
    else if(evt.type == sf::Event::MouseButtonReleased)
    {
        if(evt.mouseButton.button == sf::Mouse::Left)
        {
            //send a command to the paddle to launch the ball if it has one
            //else spawn a new ball
            xy::Command cmd;
            cmd.targetFlags = CommandID::Paddle;
            cmd.action = [&](xy::Entity entity, float)
            {
                auto& paddle = entity.getComponent<Paddle>();
                if(paddle.ball.isValid())
                {
                    paddle.ball.getComponent<Ball>().state = Ball::State::Active;
                    paddle.ball.getComponent<Collider>().dynamic = true;
                    auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
                    paddle.ball.getComponent<xy::Transform>().setPosition(entity.getComponent<xy::Transform>().getPosition() + sf::Vector2f(0.f, -ballBounds.height / 2.f));
                    entity.getComponent<xy::Transform>().removeChild(paddle.ball.getComponent<xy::Transform>());
                    paddle.ball = {};
                }
            };
            m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }
    else if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::Escape:
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
            requestStackPush(States::PauseState);
            break;
        case sf::Keyboard::Q:
            //quits of we're in game over
            if (m_lives == 0)
            {
                requestStackClear();
                requestStackPush(States::MyFirstState);
            }
            break;
        }
    }

    m_gameScene.forwardEvent(evt);
    m_uiScene.forwardEvent(evt);
    return true;
}

void GameState::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::BallMessage)
    {
        const auto& data = msg.getData<BallEvent>();
        if (data.action == BallEvent::Despawned)
        {
            m_lives--;

            xy::Command cmd;
            cmd.targetFlags = CommandID::Lives;
            cmd.action = [&](xy::Entity e, float)
            {
                e.getComponent<xy::Text>().setString("Lives: " + std::to_string(m_lives));
            };
            m_uiScene.getSystem<xy::CommandSystem>().sendCommand(cmd);

            if (m_lives == 0)
            {
                showEndScreen();
            }
            else
            {
                spawnBall();
            }
        }
    }
    else if (msg.id == MessageID::BlockMessage)
    {
        const auto& data = msg.getData<BlockEvent>();
        if (data.action == BlockEvent::Added)
        {
            m_blockCount++;
        }
        else if (data.action == BlockEvent::Destroyed)
        {
            m_blockCount--;

            if (m_blockCount == 0)
            {
                createBlocks();
            }

            //update the score
            m_score += 100; //TODO we could include a block value as part of the message to calculate the score

            xy::Command cmd;
            cmd.targetFlags = CommandID::Score;
            cmd.action = [&](xy::Entity e, float)
            {
                e.getComponent<xy::Text>().setString("Score: " + std::to_string(m_score));
            };
            m_uiScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }

    m_gameScene.forwardMessage(msg);
    m_uiScene.forwardMessage(msg);
}

bool GameState::update(float dt)
{
    m_gameScene.update(dt);
    m_uiScene.update(dt);
    return true;
}

void GameState::draw()
{
    auto rw = getContext().appInstance.getRenderWindow();
    rw->draw(m_gameScene);
    rw->draw(m_uiScene);
}

xy::StateID GameState::stateID() const
{
    return States::GameState;
}

void GameState::createScene()
{
    //add the systems
    auto& messageBus = getContext().appInstance.getMessageBus();
    m_gameScene.addSystem<BallSystem>(messageBus);
    m_gameScene.addSystem<xy::DynamicTreeSystem>(messageBus);
    m_gameScene.addSystem<CollisionSystem>(messageBus);
    m_gameScene.addSystem<xy::CommandSystem>(messageBus);
    m_gameScene.addSystem<xy::SpriteSystem>(messageBus);
    m_gameScene.addSystem<xy::RenderSystem>(messageBus);
    m_gameScene.addSystem<xy::ParticleSystem>(messageBus);
    m_gameScene.addSystem<xy::AudioSystem>(messageBus);

    m_gameScene.addDirector<FXDirector>(m_resources);

    loadResources();

    //create the paddle
    auto entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x / 2.f, xy::DefaultSceneSize.y - 40.f);
    entity.addComponent<xy::Sprite>() = SpriteID::sprites[SpriteID::Paddle];
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Paddle;
    entity.addComponent<Paddle>();

    auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Transform>().setOrigin(paddleBounds.width / 2.f, paddleBounds.height / 2.f);
    entity.addComponent<xy::BroadphaseComponent>(paddleBounds);
    entity.addComponent<Collider>();

    spawnBall();

    createBlocks();

    //create the colliders on 3 sides
    sf::FloatRect wallBounds = { 0.f, 0.f, xy::DefaultSceneSize.x, 20.f };
    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>();
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });

    wallBounds.width = 20.f;
    wallBounds.height = xy::DefaultSceneSize.y;
    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(0.f, 20.f);
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });

    entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x - wallBounds.width, 20.f);
    entity.addComponent<xy::BroadphaseComponent>(wallBounds);
    entity.addComponent<Collider>();
    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });


    //load up the UI
    FontID::handles[FontID::ScoreFont] = m_resources.load<sf::Font>("ProggyClean.ttf");

    m_uiScene.addSystem<xy::CommandSystem>(messageBus);
    m_uiScene.addSystem<xy::TextSystem>(messageBus);
    m_uiScene.addSystem<xy::RenderSystem>(messageBus);

    entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(20.f, 20.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(50);
    entity.getComponent<xy::Text>().setString("Score: 0");
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Score;

    entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(20.f, 60.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(50);
    entity.getComponent<xy::Text>().setString("Lives: 3");
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Lives;
}

void GameState::loadResources()
{
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/sprites.spt", m_resources);

    SpriteID::sprites[SpriteID::Ball] = spriteSheet.getSprite("Ball");
    SpriteID::sprites[SpriteID::Paddle] = spriteSheet.getSprite("Paddle");
    SpriteID::sprites[SpriteID::Block] = spriteSheet.getSprite("Block");
}

void GameState::spawnBall()
{
    xy::Command cmd;
    cmd.targetFlags = CommandID::Paddle;
    cmd.action = [&](xy::Entity entity, float)
    {
        auto& paddle = entity.getComponent<Paddle>();
        paddle.ball = m_gameScene.createEntity();
        paddle.ball.addComponent<xy::Transform>();
        paddle.ball.addComponent<xy::Sprite>() = SpriteID::sprites[SpriteID::Ball];
        paddle.ball.addComponent<xy::Drawable>();
        paddle.ball.addComponent<Ball>();

        auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
        auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
        paddle.ball.getComponent<xy::Transform>().setOrigin(ballBounds.width / 2.f, ballBounds.height / 2.f);
        paddle.ball.getComponent<xy::Transform>().setPosition(paddleBounds.width / 2.f, -ballBounds.height / 2.f);
        paddle.ball.addComponent<xy::BroadphaseComponent>(ballBounds);
        paddle.ball.addComponent<Collider>().callback = 
            [](xy::Entity e, xy::Entity other, Manifold man)
        {
            //if we hit the paddle change the velocity angle
            if (other.hasComponent<Paddle>())
            {
                auto newVel = e.getComponent<xy::Transform>().getPosition() - other.getComponent<xy::Transform>().getPosition();
                e.getComponent<Ball>().velocity = xy::Util::Vector::normalise(newVel);
            }
            else
            {
                //reflect the ball's velocity around the collision normal
                auto vel = e.getComponent<Ball>().velocity;
                vel = xy::Util::Vector::reflect(vel, man.normal);
                e.getComponent<Ball>().velocity = vel;
            }
        };

        entity.getComponent<xy::Transform>().addChild(paddle.ball.getComponent<xy::Transform>());
    };
    m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
}

void GameState::createBlocks()
{
    static const std::array<sf::Color, 6u> colours =
    {
        sf::Color::Red, sf::Color::Magenta, sf::Color::Yellow,
        sf::Color::Green, sf::Color::Cyan, sf::Color::Blue
    };

    static const sf::FloatRect BlockSize = SpriteID::sprites[SpriteID::Block].getTextureBounds();

    auto addBlock = [&](sf::Vector2f position, sf::Color colour)
    {
        auto entity = m_gameScene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(position);
        entity.addComponent<xy::Drawable>();
        entity.addComponent<xy::Sprite>() = SpriteID::sprites[SpriteID::Block];
        entity.getComponent<xy::Sprite>().setColour(colour);
        entity.addComponent<xy::BroadphaseComponent>(BlockSize);
        entity.addComponent<Collider>().callback = 
            [&](xy::Entity e, xy::Entity, Manifold)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<BlockEvent>(MessageID::BlockMessage);
            msg->action = BlockEvent::Destroyed;
            msg->position = e.getComponent<xy::Transform>().getPosition();
            msg->position.x += BlockSize.width / 2.f;
            msg->position.y += BlockSize.height / 2.f;

            m_gameScene.destroyEntity(e);
        };

        auto* msg = getContext().appInstance.getMessageBus().post<BlockEvent>(MessageID::BlockMessage);
        msg->action = BlockEvent::Added;
        msg->position = position;
        msg->position.x += BlockSize.width / 2.f;
        msg->position.y += BlockSize.height / 2.f;
    };

    static const std::size_t BlockXCount = 12;
    static const std::size_t BlockYCount = 10;
    
    sf::Vector2f offset = xy::DefaultSceneSize;
    offset.x -= BlockXCount * BlockSize.width;
    offset.x /= 2.f;

    offset.y -= BlockYCount * BlockSize.height;
    offset.y /= 2.f;

    for (auto y = 0u; y < BlockYCount; ++y)
    {
        for (auto x = 0u; x < BlockXCount; ++x)
        {
            sf::Vector2f position(x * BlockSize.width, y * BlockSize.height);
            auto colour = colours[y % colours.size()];

            addBlock(offset + position, colour);
        }
    }
}

void GameState::showEndScreen()
{
    auto entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 200.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(120);
    entity.getComponent<xy::Text>().setString("GAME OVER");
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>();

    entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 360.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(40);
    entity.getComponent<xy::Text>().setString("Press Q to Quit");
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>();
}