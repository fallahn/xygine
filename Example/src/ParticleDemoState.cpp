/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <ParticleDemoState.hpp>
#include <CaveDemoDrawable.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>

#include <xygine/components/ParticleController.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/PostBloom.hpp>
#include <xygine/PostChromeAb.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

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

    enum ParticleType
    {
        Bubbles = 0,
        Explosion,
        FairyDust,
        Fire,
        Count
    };

    sf::Uint64 controllerId = 0;

    bool debugDraw = false;
}

ParticleDemoState::ParticleDemoState(xy::StateStack& stateStack, Context context)
    : State         (stateStack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_physWorld     (m_messageBus)
{
    launchLoadingScreen();

    m_scene.setView(context.defaultView);

    xy::PostProcess::Ptr pp = xy::PostProcess::create<xy::PostChromeAb>();
    //m_scene.addPostProcess(pp);
    pp = xy::PostProcess::create<xy::PostBloom>();
    m_scene.addPostProcess(pp);
    m_scene.setClearColour({ 0u, 0u, 20u });

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    setupParticles();
    buildTerrain();

    context.renderWindow.setMouseCursorVisible(true);

    quitLoadingScreen();
}

bool ParticleDemoState::update(float dt)
{    
    m_scene.update(dt);

    m_reportText.setString(xy::StatsReporter::reporter.getString());

    return true;
}

void ParticleDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    rw.draw(m_physWorld);
    rw.draw(m_reportText);
}

bool ParticleDemoState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        const auto& rw = getContext().renderWindow;
        auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

        spawnThing(mousePos);
    }
        break;
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
        case sf::Keyboard::Q:
            debugDraw = !debugDraw;
            m_scene.drawDebug(debugDraw);
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

void ParticleDemoState::handleMessage(const xy::Message& msg)
{ 
    m_scene.handleMessage(msg);
}

//private
void ParticleDemoState::setupParticles()
{
    auto particleController = xy::Component::create<xy::ParticleController>(m_messageBus);
    auto entity = xy::Entity::create(m_messageBus);

    auto pc = entity->addComponent(particleController);
    
    m_particleDef.loadFromFile("assets/particles/bubbles.xyp", m_textureResource);
    pc->addDefinition(ParticleType::Bubbles, m_particleDef);

    m_particleDef.loadFromFile("assets/particles/explosion.xyp", m_textureResource);
    pc->addDefinition(ParticleType::Explosion, m_particleDef);

    m_particleDef.loadFromFile("assets/particles/fairydust.xyp", m_textureResource);
    pc->addDefinition(ParticleType::FairyDust, m_particleDef);

    m_particleDef.loadFromFile("assets/particles/fire.xyp", m_textureResource);
    pc->addDefinition(ParticleType::Fire, m_particleDef);

    controllerId = entity->getUID();
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void ParticleDemoState::buildTerrain()
{
    auto ent = xy::Entity::create(m_messageBus);
    auto cd = std::make_unique<CaveDemo::CaveDrawable>(m_messageBus);
    ent->move((sf::Vector2f(1920.f, 1080.f) - cd->getSize()) / 2.f);
    //ent->move(100.f, 100.f);
    auto cave = ent->addComponent(cd);
    m_scene.addEntity(ent, xy::Scene::Layer::BackRear);

    //get edges to add to physworld
    const auto& edges = cave->getEdges();
    int buns = 0;
}

void ParticleDemoState::spawnThing(const sf::Vector2f& position)
{
    auto ps = m_particleDef.createSystem(m_messageBus);
    ps->start(m_particleDef.releaseCount, m_particleDef.delay, m_particleDef.duration);

    auto physBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionCircleShape cs(10.f);
    cs.setRestitution(0.99f);
    cs.setDensity(20.f);
    physBody->addCollisionShape(cs);

    auto td = xy::Component::create<TimedDestruction>(m_messageBus);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setWorldPosition(position);
    entity->addComponent(ps);
    entity->addComponent(physBody);
    entity->addComponent(td);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}