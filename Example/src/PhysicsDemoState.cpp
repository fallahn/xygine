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
#include <xygine/components/AudioSource.hpp>

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
#include <xygine/physics/AffectorAreaForce.hpp>
#include <xygine/physics/AffectorPointForce.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <TimedDestruction.hpp>

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
    launchLoadingScreen();
    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    createBodies();

    quitLoadingScreen();
}

bool PhysicsDemoState::update(float dt)
{
    const auto& rw = getContext().renderWindow;
    auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));
    
    m_scene.update(dt);

    m_reportText.setString(xy::StatsReporter::reporter.getString());

    static sf::Clock clock;
    if (clock.getElapsedTime().asSeconds() > 6.f)
    {
        clock.restart();
        randomBall();
    }

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

    if (msg.id == xy::Message::PhysicsMessage)
    {
        //play collision sounds
        auto& msgData = msg.getData<xy::Message::PhysicsEvent>();
        if (msgData.event == xy::Message::PhysicsEvent::BeginContact)
        {
            auto contact = msgData.contact;

            xy::Command cmd;
            cmd.entityID = contact->getCollisionShapeA()->getRigidBody()->getParentUID();
            cmd.action = [](xy::Entity& entity, float)
            {
                auto sound = entity.getComponent<xy::AudioSource>();
                if (sound) sound->play();
            };
            m_scene.sendCommand(cmd);
            cmd.entityID = contact->getCollisionShapeB()->getRigidBody()->getParentUID();
            m_scene.sendCommand(cmd);
        }
    }
}

//private
void PhysicsDemoState::createBodies()
{
    auto groundEntity = xy::Entity::create(m_messageBus);
    groundEntity->setWorldPosition({ 0.f, 0.f });
    auto groundBody = xy::Physics::RigidBody::create(m_messageBus, xy::Physics::BodyType::Static);

    xy::Physics::CollisionRectangleShape boxShape({ 1920.f, 80.f });
    groundBody->addCollisionShape(boxShape);
    boxShape.setRect({ 20.f, 920.f }, { 0.f, 80.f });
    groundBody->addCollisionShape(boxShape);

    boxShape.setRect({ 20.f, 920.f }, { 1900.f, 80.f });
    groundBody->addCollisionShape(boxShape);

    boxShape.setRect({ 1920.f, 80.f }, { 0.f, 1000.f });
    groundBody->addCollisionShape(boxShape);

    std::vector<sf::Vector2f> points = 
    {
        {800.f, 850.f},
        {880.f, 830.f},
        {940.f, 820.f},
        {1100.f, 805.f}
    };
    xy::Physics::CollisionEdgeShape edgeShape(points, xy::Physics::CollisionEdgeShape::Option::Loop);
    groundBody->addCollisionShape(edgeShape);

    points =
    {
        { 400.f, 650.f },
        { 480.f, 630.f },
        { 540.f, 620.f },
        { 700.f, 605.f }
    };
    edgeShape.setPoints(points);
    groundBody->addCollisionShape(edgeShape);

    points =
    {
        { 1700.f, 950.f },
        { 1780.f, 930.f },
        { 1820.f, 920.f },
        { 1780.f, 815.f },
        { 1700.f, 900.f }
    };
    xy::Physics::CollisionPolygonShape polyShape(points);
    polyShape.addAffector(xy::Physics::AreaForceAffector({ -190.f, -4000.f }, 80.f));
    polyShape.setIsSensor(true);
    groundBody->addCollisionShape(polyShape);

    points =
    {
        { 300.f, 950.f },
        { 380.f, 970.f },
        { 420.f, 980.f },
        { 380.f, 815.f },
        { 300.f, 900.f }
    };
    polyShape.setPoints(points);
    polyShape.setRestitution(1.f);
    polyShape.clearAffectors();
    xy::Physics::PointForceAffector pfa(450.f);
    polyShape.addAffector(pfa);
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

    auto soundSource = xy::AudioSource::create(m_messageBus, m_soundResource);
    soundSource->setSound("assets/sound/04.wav");
    soundSource->setAttenuation(20.f);
    soundSource->setMinimumDistance(400.f);
    ballEntityA->addComponent<xy::AudioSource>(soundSource);

    m_scene.addEntity(ballEntityA, xy::Scene::Layer::BackMiddle);

    randomBall();
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

void PhysicsDemoState::randomBall()
{
    sf::Vector2i randPos(xy::Util::Random::value(20, 1850), xy::Util::Random::value(125, 350));
    
    auto ballEntityB = xy::Entity::create(m_messageBus);
    ballEntityB->setWorldPosition(sf::Vector2f(randPos));
    auto ballBodyB = xy::Physics::RigidBody::create(m_messageBus, xy::Physics::BodyType::Dynamic);

    xy::Physics::CollisionCircleShape ballShape(static_cast<float>(xy::Util::Random::value(25, 50)));
    ballShape.setDensity(1.5f);
    ballShape.setRestitution(xy::Util::Random::value(0.2f, 0.9f));
    ballBodyB->addCollisionShape(ballShape);

    ballEntityB->addComponent<xy::Physics::RigidBody>(ballBodyB);

    auto soundSource = xy::AudioSource::create(m_messageBus, m_soundResource);
    soundSource = xy::AudioSource::create(m_messageBus, m_soundResource);
    soundSource->setSound("assets/sound/04.wav");
    soundSource->setAttenuation(20.f);
    soundSource->setPitch(xy::Util::Random::value(0.4f, 5.f));
    ballEntityB->addComponent<xy::AudioSource>(soundSource);

    auto td = std::make_unique<TimedDestruction>(m_messageBus);
    ballEntityB->addComponent<TimedDestruction>(td);

    m_scene.addEntity(ballEntityB, xy::Scene::Layer::BackMiddle);
}