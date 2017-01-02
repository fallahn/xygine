/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <CommandIds.hpp>
#include <TilemapDemoState.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/util/Vector.hpp>

#include <xygine/components/TileMapLayer.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/shaders/Tilemap.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Keyboard::Key upKey = sf::Keyboard::W;
    const sf::Keyboard::Key downKey = sf::Keyboard::S;
    const sf::Keyboard::Key leftKey = sf::Keyboard::A;
    const sf::Keyboard::Key rightKey = sf::Keyboard::D;
    const sf::Keyboard::Key fireKey = sf::Keyboard::Space;

    const float joyDeadZone = 25.f;
    const float joyMaxAxis = 100.f;

    enum Input
    {
        Up = 0x1,
        Down = 0x2,
        Left = 0x4,
        Right = 0x8
    };
    const float moveSpeed = 1500.f;

    xy::Entity* ent = nullptr;
    sf::Uint8 input = 0;

    bool drawOverlay = false;
}

TilemapDemoState::TilemapDemoState(xy::StateStack& stateStack, Context context)
    : State     (stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus),
    m_physWorld (m_messageBus)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);
    m_shaderResource.preload(xy::Shader::Tilemap, xy::Shader::tmx::vertex, xy::Shader::tmx::fragment);
    m_physWorld.setGravity({ 0.f, 980.f });

    buildScene();

    REPORT("Q", "Show Debug");
    //xy::StatsReporter::show();

    quitLoadingScreen();
}

bool TilemapDemoState::update(float dt)
{    
    sf::Vector2f vel;
    if (input & Input::Up)
    {
        vel.y = -1.f;
    }
    if (input & Input::Down)
    {
        vel.y = 1.f;
    }
    if (input & Input::Right)
    {
        vel.x = 1.f;
    }
    if (input & Input::Left)
    {
        vel.x = -1.f;
    }
    if(input) ent->move(xy::Util::Vector::normalise(vel) * moveSpeed * dt);
    
    m_scene.update(dt);
    return true;
}

void TilemapDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(m_scene.getView());
    if (drawOverlay) rw.draw(m_physWorld);
}

bool TilemapDemoState::handleEvent(const sf::Event& evt)
{   
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        //const auto& rw = getContext().renderWindow;
        //auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    }
    break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case upKey:
            input |= Input::Up;
            break;
        case downKey:
            input |= Input::Down;
            break;
        case leftKey:
            input |= Input::Left;
            break;
        case rightKey:
            input |= Input::Right;
            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case sf::Keyboard::Escape:
        case sf::Keyboard::BackSpace:
            requestStackPop();
            requestStackPush(States::ID::MenuMain);
            break;
        case sf::Keyboard::P:
            //requestStackPush(States::ID::MenuPaused);
            break;
        case upKey:
            input &= ~Input::Up;
            break;
        case downKey:
            input &= ~Input::Down;
            break;
        case leftKey:
            input &= ~Input::Left;
            break;
        case rightKey:
            input &= ~Input::Right;
            break;
        case fireKey:

            break;
        case sf::Keyboard::Q:
            drawOverlay = !drawOverlay;
            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonPressed:
        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonReleased:

        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        case 7:

            break;
        default: break;
        }
        break;
    default: break;
    }

    return true;
}

void TilemapDemoState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void TilemapDemoState::buildScene()
{
    if (m_tilemap.load("assets/maps/platform.tmx"))
    {
        auto entity = xy::Entity::create(m_messageBus);
        const auto& layers = m_tilemap.getLayers();
        for (const auto& l : layers)
        {
            if (l->getType() == xy::tmx::Layer::Type::Object)
            {
                xy::Logger::log("found object layer - attempting to create physics components", xy::Logger::Type::Info);
                auto rb = m_tilemap.createRigidBody(m_messageBus, *l);
                entity->addComponent(rb);
            }
            else
            {
                auto drawable = m_tilemap.getDrawable(m_messageBus, *l, m_textureResource, m_shaderResource);
                if (drawable)
                {
                    xy::Logger::log("created layer drawable, adding to scene...");
                    entity->addComponent(drawable);
                }
            }
        }
        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

        static const float radius = 30.f;

        auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
        auto cs = xy::Physics::CollisionCircleShape(radius);
        cs.setDensity(0.9f);
        cs.setRestitution(1.f);
        body->addCollisionShape(cs);

        auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
        drawable->getDrawable().setRadius(radius);
        drawable->getDrawable().setOrigin({ radius, radius });
        drawable->getDrawable().setFillColor({ 255, 255, 255, 200 });
        drawable->getDrawable().setOutlineThickness(2.f);

        auto cam = xy::Component::create<xy::Camera>(m_messageBus, getContext().defaultView);
        cam->lockTransform(xy::Camera::TransformLock::Rotation, true);
        cam->lockBounds(m_tilemap.getBounds());
        
        entity = xy::Entity::create(m_messageBus);
        entity->setPosition(800.f, 400.f);
        entity->addComponent(body);
        entity->addComponent(drawable);
        auto camPtr = entity->addComponent(cam);

        ent = m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
        m_scene.setActiveCamera(camPtr);
    }
}