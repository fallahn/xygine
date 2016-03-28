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
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/PostBloom.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/shaders/NormalMapped.hpp>

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

    enum ParticleShaderId
    {
        NormalMapTextured = 1,
        NormalMapTexturedSpecular
    };

    sf::Uint64 controllerId = 0;

    bool debugDraw = false;

    sf::Shader* shader;
}

ParticleDemoState::ParticleDemoState(xy::StateStack& stateStack, Context context)
    : State         (stateStack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_physWorld     (m_messageBus)
{
    launchLoadingScreen();
    xy::Stats::clear();
    m_physWorld.setGravity({ 0.f, 900.f });
    m_physWorld.setPixelScale(100.f);

    m_scene.setView(context.defaultView);

    xy::PostProcess::Ptr pp = xy::PostProcess::create<xy::PostBloom>();
    //m_scene.addPostProcess(pp);
    pp = xy::PostProcess::create<xy::PostChromeAb>();
    //m_scene.addPostProcess(pp);
    m_scene.setClearColour({ 0u, 0u, 20u });

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    m_shaderResource.preload(ParticleShaderId::NormalMapTexturedSpecular, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED_SPECULAR);
    //m_shaderResource.preload(ParticleShaderId::NormalMapTextured, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED);
    shader = &m_shaderResource.get(ParticleShaderId::NormalMapTexturedSpecular);

    setupParticles();
    buildTerrain();

    context.renderWindow.setMouseCursorVisible(true);

    quitLoadingScreen();
}

bool ParticleDemoState::update(float dt)
{    
    m_scene.update(dt);

    //update lighting
    auto ents = m_scene.queryQuadTree(m_scene.getVisibleArea());
    auto i = 0;
    for (; i < ents.size() && i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        auto light = ents[i]->getEntity()->getComponent<xy::PointLight>();
        if (light)
        {
            const std::string idx = std::to_string(i);
            
            auto pos = light->getWorldPosition();
            shader->setParameter("u_pointLightPositions[" + std::to_string(i) + "]", pos);
            shader->setParameter("u_pointLights[" + idx + "].intensity", light->getIntensity());
            shader->setParameter("u_pointLights[" + idx + "].diffuseColour", light->getDiffuseColour());
            shader->setParameter("u_pointLights[" + idx + "].specularColour", light->getSpecularColour());
            shader->setParameter("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());
        }
    }
    //switch off inactive lights
    for (; i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        shader->setParameter("u_pointLights[" + std::to_string(i) + "].intensity", 0.f);
    }


    m_reportText.setString(xy::Stats::getString());

    return true;
}

void ParticleDemoState::draw()
{   
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    //rw.draw(m_physWorld);
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
    
    m_particleDef.loadFromFile("assets/particles/explosion.xyp", m_textureResource);
    particleController->addDefinition(ParticleType::Explosion, m_particleDef);

    auto pc = entity->addComponent(particleController);

    m_particleDef.loadFromFile("assets/particles/bubbles.xyp", m_textureResource);
    pc->addDefinition(ParticleType::Bubbles, m_particleDef);

    m_particleDef.loadFromFile("assets/particles/fire.xyp", m_textureResource);
    pc->addDefinition(ParticleType::Fire, m_particleDef);

    m_particleDef.loadFromFile("assets/particles/fairydust.xyp", m_textureResource);
    pc->addDefinition(ParticleType::FairyDust, m_particleDef);

    xy::Component::MessageHandler mh;
    mh.action = [](xy::Component* c, const xy::Message& msg) 
    {
        auto& msgData = msg.getData<xy::Message::EntityEvent>();
        auto controller = dynamic_cast<xy::ParticleController*>(c);
        controller->fire(ParticleType::Explosion, msgData.entity->getWorldPosition());
    };
    mh.id = xy::Message::EntityMessage;
    pc->addMessageHandler(mh);

    controllerId = entity->getUID();
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void ParticleDemoState::buildTerrain()
{
    auto ent = xy::Entity::create(m_messageBus);

    /*auto ad = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    ad->setTexture(m_textureResource.get("assets/images/cave/background.png"));
    ad->setNormalMap(m_textureResource.get("assets/images/cave/background_normal.png"));
    ad->setShader(m_shaderResource.get(ParticleShaderId::NormalMapTexturedSpecular));
    ad->setScale(1.6f, 1.6f);
    ent->addComponent(ad);*/

    auto cd = std::make_unique<CaveDemo::CaveDrawable>(m_messageBus);
    ent->move((sf::Vector2f(1920.f, 1080.f) - cd->getSize()) / 2.f);
    //ent->move(100.f, 100.f);
    auto cave = ent->addComponent(cd);
    cave->setTexture(m_textureResource.get("assets/images/cave/diffuse.png"));
    cave->setNormalMap(m_textureResource.get("assets/images/cave/normal.png"));
    cave->setMaskMap(m_textureResource.get("assets/images/cave/mask.png"));
    cave->setShader(&m_shaderResource.get(ParticleShaderId::NormalMapTexturedSpecular));
    
    //get edges to add to physworld
    const auto& edges = cave->getEdges();
    REPORT("edge count", std::to_string(edges.size()));
    auto rb = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    for (const auto& e : edges)
    {
        xy::Physics::CollisionEdgeShape es(e);
        rb->addCollisionShape(es);
    }
    ent->addComponent(rb);

    m_scene.addEntity(ent, xy::Scene::Layer::BackRear);
}

void ParticleDemoState::spawnThing(const sf::Vector2f& position)
{
    auto ps = m_particleDef.createSystem(m_messageBus);
    ps->setLifetimeVariance(m_particleDef.lifetime / 2.f);
    ps->start(m_particleDef.releaseCount, m_particleDef.delay, m_particleDef.duration);

    auto physBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionCircleShape cs(10.f);
    cs.setRestitution(0.99f);
    cs.setDensity(20.f);
    physBody->addCollisionShape(cs);

    auto td = xy::Component::create<TimedDestruction>(m_messageBus);

    auto dwbl = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    dwbl->setTexture(m_textureResource.get("assets/images/physics demo/ball.png"));
    //dwbl->setNormalMap(m_textureResource.get("assets/images/physics demo/ball_normal.png"));
    //dwbl->setShader(m_shaderResource.get(ParticleShaderId::NormalMapTexturedSpecular));
    auto size = dwbl->getFrameSize();
    dwbl->setOrigin({ size.x / 2.f, size.y / 2.f });
    dwbl->setColour({ 198u, 200u, 250u });

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, sf::FloatRect(-size.x / 2.f, -size.y / 2.f, size.x, size.y));

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 100.f, sf::Color(198u, 200u, 250u), sf::Color(128u, 128u, 255u));
    light->setDepth(300.f);
    light->setRange(750.f);
    //light->setIntensity(0.9f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setWorldPosition(position);
    entity->addComponent(ps);
    entity->addComponent(physBody);
    entity->addComponent(td);
    entity->addComponent(dwbl);
    entity->addComponent(qtc);
    entity->addComponent(light);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}