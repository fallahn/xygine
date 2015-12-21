/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <PhysicsDemoState.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>
#include <xygine/SfDrawableComponent.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/Util.hpp>

#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/physics/JointDistance.hpp>
#include <xygine/physics/JointFriction.hpp>
#include <xygine/physics/JointHinge.hpp>
#include <xygine/physics/JointSlider.hpp>
#include <xygine/physics/JointWheel.hpp>
#include <xygine/physics/JointRope.hpp>
#include <xygine/physics/JointWeld.hpp>
#include <xygine/physics/JointMotor.hpp>
#include <xygine/physics/AffectorConstantForce.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

namespace
{
    const sf::Keyboard::Key upKey = sf::Keyboard::W;
    const sf::Keyboard::Key downKey = sf::Keyboard::S;
    const sf::Keyboard::Key leftKey = sf::Keyboard::A;
    const sf::Keyboard::Key rightKey = sf::Keyboard::D;
    const sf::Keyboard::Key fireKey = sf::Keyboard::Space;

    const float joyDeadZone = 25.f;
    const float joyMaxAxis = 100.f;
}

PhysicsDemoState::PhysicsDemoState(xy::StateStack& stateStack, Context context)
    : State         (stateStack, context),
    m_physWorld     (context.appInstance.getMessageBus()),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus)
{
    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);

    m_reportText.setFont(context.appInstance.getFont("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);


    createBodies();
}

bool PhysicsDemoState::update(float dt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));
    
    m_scene.update(dt);

    m_reportText.setString(xy::StatsReporter::reporter.getString());

    return true;
}

void PhysicsDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    rw.draw(m_physWorld);
    rw.draw(m_reportText);
}

bool PhysicsDemoState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case upKey:
            
            break;
        case downKey:
            
            break;
        case leftKey:
            
            break;
        case rightKey:
            
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
            
            break;
        case downKey:
            
            break;
        case leftKey:
            
            break;
        case rightKey:
            
            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonPressed:
        switch (evt.joystickButton.button)
        {
        case 0:
            
            break;
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
    }

    return true;
}

void PhysicsDemoState::handleMessage(const xy::Message& msg)
{ 
    m_scene.handleMessage(msg);
}

//private
void PhysicsDemoState::createBodies()
{
    auto groundEntity = xy::Entity::create(m_messageBus);
    groundEntity->setWorldPosition({ 0.f, 1000.f });
    auto groundBody = xy::Physics::RigidBody::create(m_messageBus, xy::Physics::BodyType::Static);

    xy::Physics::CollisionRectangleShape groundShape({ 1920.f, 80.f });
    groundBody->addCollisionShape(groundShape);

    std::vector<sf::Vector2f> points = 
    {
        {800.f, -150.f},
        {880.f, -130.f},
        {940.f, -120.f},
        {1100.f, -105.f}
    };
    xy::Physics::CollisionEdgeShape edgeShape(points, xy::Physics::CollisionEdgeShape::Option::Loop);
    groundBody->addCollisionShape(edgeShape);

    points =
    {
        { 400.f, -350.f },
        { 480.f, -330.f },
        { 540.f, -320.f },
        { 700.f, -305.f }
    };
    edgeShape.setPoints(points);
    groundBody->addCollisionShape(edgeShape);

    points =
    {
        { 1800.f, -50.f },
        { 1880.f, -30.f },
        { 1920.f, -20.f },
        { 1880.f, -185.f },
        { 1800.f, -100.f }
    };
    xy::Physics::CollisionPolygonShape polyShape(points);
    groundBody->addCollisionShape(polyShape);

    points =
    {
        { 300.f, -50.f },
        { 380.f, -30.f },
        { 420.f, -20.f },
        { 380.f, -185.f },
        { 300.f, -100.f }
    };
    polyShape.setPoints(points);
    polyShape.setRestitution(1.f);
    groundBody->addCollisionShape(polyShape);

    groundEntity->addComponent<xy::Physics::RigidBody>(groundBody);

    m_scene.addEntity(groundEntity, xy::Scene::Layer::BackMiddle);

    //------------//
    auto ballEntityA = xy::Entity::create(m_messageBus);
    ballEntityA->setWorldPosition({ 960.f, 540.f });
    auto ballBodyA = xy::Physics::RigidBody::create(m_messageBus, xy::Physics::BodyType::Dynamic);

    xy::Physics::CollisionCircleShape ballShape(25.f);
    ballShape.setDensity(1.5f);
    ballShape.setRestitution(0.8f);
    ballBodyA->addCollisionShape(ballShape);

    auto ba = ballEntityA->addComponent<xy::Physics::RigidBody>(ballBodyA);

    m_scene.addEntity(ballEntityA, xy::Scene::Layer::BackMiddle);

    auto ballEntityB = xy::Entity::create(m_messageBus);
    ballEntityB->setWorldPosition({ 440.f, 500.f });
    auto ballBodyB = xy::Physics::RigidBody::create(m_messageBus, xy::Physics::BodyType::Dynamic);

    ballShape.setRadius(50.f);
    ballBodyB->addCollisionShape(ballShape);

    auto bb = ballEntityB->addComponent<xy::Physics::RigidBody>(ballBodyB);
    m_scene.addEntity(ballEntityB, xy::Scene::Layer::BackMiddle);

    bb->addAffector(xy::Physics::ConstantForceAffector({ 20.f, 0.f }, 40.f));

    /*xy::Physics::DistanceJoint dj(*ba, { 960.f, 540.f }, { 440.f, 500.f });
    bb->addJoint(dj);

    xy::Physics::FrictionJoint fj(*ba, { 960.f, 540.f });
    bb->addJoint(fj);

    xy::Physics::HingeJoint hj(*ba, { 520.f, 540.f });
    hj.setLimits(90.f, 270.f);
    hj.limitEnabled(true);
    bb->addJoint(hj);

    xy::Physics::SliderJoint sj(*ba, { 520.f, 540.f }, { 10.f, 0.f });
    sj.canCollide(true);
    sj.setLimits(300.f, -220.f);
    sj.limitEnabled(true);
    sj.setMotorSpeed(-400.f);
    sj.setMaxMotorForce(5000.f);
    sj.motorEnabled(true);
    bb->addJoint(sj);

    xy::Physics::WheelJoint wj(*ba, { 960.f, 540.f }, { 0.f, 10.f });
    wj.setSpringFrequency(0.f);
    bb->addJoint(wj);

    xy::Physics::RopeJoint rj(*ba, { 960.f, 540.f }, { 440.f, 500.f });
    rj.canCollide(true);
    bb->addJoint(rj);

    xy::Physics::WeldJoint wj(*ba, { 520.f, 540.f });
    bb->addJoint(wj);

    xy::Physics::MotorJoint mj(*ba);
    bb->addJoint(mj);*/


}