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

#include <PlatformDemoState.hpp>
#include <PlatformPlayerController.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>

#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/components/Camera.hpp>

#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>

#include <xygine/components/Model.hpp>
#include <RotationComponent.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/mesh/shaders/GeomVis.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/mesh/IQMBuilder.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
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

    enum PlatformShaderId
    {
        VertexLit
    };

    Plat::PlayerController * playerController = nullptr;
    sf::Uint8 playerInput = 0;
}

PlatformDemoState::PlatformDemoState(xy::StateStack& stateStack, Context context)
    : State         (stateStack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_physWorld     (m_messageBus),
    m_meshRenderer  ({ 1920, 1080 }, m_scene)
{
    launchLoadingScreen();
    xy::Stats::clear();
    m_physWorld.setGravity({ 0.f, 1180.f });
    m_physWorld.setPixelScale(120.f);

    m_scene.setView(context.defaultView);

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    buildTerrain();
    buildPhysics();

    //TODO add some crates or so
    addPlayer();
    createMesh();

    context.renderWindow.setMouseCursorVisible(true);

    quitLoadingScreen();
}

bool PlatformDemoState::update(float dt)
{    
    playerController->applyInput(playerInput);

    m_scene.update(dt);
    m_meshRenderer.update();

    ////update lighting
    //auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    //auto i = 0;
    //for (; i < lights.size() && i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    //{
    //    auto light = lights[i];
    //    if (light)
    //    {
    //        const std::string idx = std::to_string(i);
    //        
    //        auto pos = light->getWorldPosition();
    //        shader->setUniform("u_pointLightPositions[" + std::to_string(i) + "]", pos);
    //        shader->setUniform("u_pointLights[" + idx + "].intensity", light->getIntensity());
    //        shader->setUniform("u_pointLights[" + idx + "].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
    //        shader->setUniform("u_pointLights[" + idx + "].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
    //        shader->setUniform("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());
    //    }
    //}
    ////switch off inactive lights
    //for (; i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    //{
    //    shader->setUniform("u_pointLights[" + std::to_string(i) + "].intensity", 0.f);
    //}

    m_reportText.setString(xy::Stats::getString());

    return true;
}

void PlatformDemoState::draw()
{   
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);    
    
    rw.setView(m_scene.getView());
    rw.draw(m_physWorld);
    
    rw.setView(getContext().defaultView);   
    rw.draw(m_meshRenderer);
    rw.draw(m_reportText);
}

bool PlatformDemoState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        const auto& rw = getContext().renderWindow;
        auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));
        
    }
        break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case upKey:
            playerInput |= Plat::Jump;
            break;
        case downKey:
            
            break;
        case leftKey:
            playerInput |= Plat::Left;
            break;
        case rightKey:
            playerInput |= Plat::Right;
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
            playerInput &= ~Plat::Jump;
            break;
        case downKey:
            
            break;
        case leftKey:
            playerInput &= ~Plat::Left;
            break;
        case rightKey:
            playerInput &= ~Plat::Right;
            break;
        case fireKey:

            break;
        case sf::Keyboard::Q:

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

void PlatformDemoState::handleMessage(const xy::Message& msg)
{ 
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);
}

//private
void PlatformDemoState::createMesh()
{
    xy::CubeBuilder cb(32.f);
    m_meshResource.add(MeshID::Cube, cb);

    xy::IQMBuilder ib("assets/models/mrfixit.iqm");
    m_meshResource.add(MeshID::Fixit, ib);

    auto model = m_meshRenderer.createModel(m_messageBus, m_meshResource.get(MeshID::Fixit));

    m_shaderResource.preload(PlatformShaderId::VertexLit, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    auto& demoMaterial = m_materialResource.add(MatId::Demo, m_shaderResource.get(PlatformShaderId::VertexLit));
    demoMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    demoMaterial.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/diffuse_test.png") });
    demoMaterial.addProperty({ "u_normalMap", m_textureResource.get("assets/images/normal_test.png") });
    demoMaterial.addProperty({ "u_maskMap", m_textureResource.get("assets/images/mask_test.png") });

    auto& fixitMaterialBody = m_materialResource.add(MatId::MrFixitBody, m_shaderResource.get(PlatformShaderId::VertexLit));
    fixitMaterialBody.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    fixitMaterialBody.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/fixit/fixitBody.png") });
    fixitMaterialBody.addProperty({ "u_normalMap", m_textureResource.get("assets/images/fixit/fixitBody_normal.png") });
    fixitMaterialBody.addProperty({ "u_maskMap", m_textureResource.get("assets/images/fixit/fixitBody_mask.png") });

    auto& fixitMaterialHead = m_materialResource.add(MatId::MrFixitHead, m_shaderResource.get(PlatformShaderId::VertexLit));
    fixitMaterialHead.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    fixitMaterialHead.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/fixit/fixitHead.png") });
    fixitMaterialHead.addProperty({ "u_normalMap", m_textureResource.get("assets/images/fixit/fixitHead_normal.png") });
    fixitMaterialHead.addProperty({ "u_maskMap", m_textureResource.get("assets/images/fixit/fixitHead_mask.png") });

    model->setSubMaterial(fixitMaterialBody, 0);
    model->setSubMaterial(fixitMaterialHead, 1);

    auto ent = xy::Entity::create(m_messageBus);
    ent->addComponent(model);
    ent->setScale(50.f, 50.f);
    ent->setPosition(960.f, 370.f);
    m_scene.addEntity(ent, xy::Scene::Layer::FrontFront);


    //tests cube mesh and default material
    model = m_meshRenderer.createModel(m_messageBus, m_meshResource.get(MeshID::Cube));
    //model->setSubMaterial(material, 0);
    ent = xy::Entity::create(m_messageBus);
    auto rotator = xy::Component::create<RotationComponent>(m_messageBus);
    ent->addComponent(rotator);
    ent->addComponent(model);
    ent->setScale(12.f, 12.f);
    ent->setPosition(1520.f, 540.f);
    m_scene.addEntity(ent, xy::Scene::Layer::FrontFront);

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 1200.f, 220.f/*, sf::Color::Blue*/);
    light->setDepth(600.f);
    //light->setIntensity(1.5f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->addComponent(light);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void PlatformDemoState::buildTerrain()
{
    auto drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/left_edge.png"));

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(256.f, 1080.f - 128.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1024.f, 1080.f - 128.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1792.f, 1080.f - 128.f);
    entity->addComponent(drawable);
    
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/right_edge.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(2560.f, 0.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_01.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(400.f, 700.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_03.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(2000.f, 550.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_02.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1670.f, 450.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_04.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1210.f, 600.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

}

void PlatformDemoState::buildPhysics()
{
    auto groundBody = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Static);
    
    xy::Physics::CollisionRectangleShape rectShape({ 128.f, 1080.f });
    groundBody->addCollisionShape(rectShape);
    rectShape.setRect({ 128.f, 1080.f - 128.f, 2560.f, 128.f });
    groundBody->addCollisionShape(rectShape);
    rectShape.setRect({ 2688.f, 0.f, 128.f, 1080.f });
    groundBody->addCollisionShape(rectShape);
    rectShape.setRect({ 128.f, -20.f, 2560.f, 20.f });
    groundBody->addCollisionShape(rectShape);

    //add platforms
    std::vector<sf::Vector2f> points = 
    {
        {420.f, 800.f},
        {400.f, 765.f},
        {784.f, 765.f},
        {768.f, 800.f}
    };
    xy::Physics::CollisionEdgeShape ce(points);
    groundBody->addCollisionShape(ce);
    
    points =
    {
        { 2020.f, 639.f },
        { 2000.f, 604.f },
        { 2384.f, 604.f },
        { 2364.f, 639.f }
    };
    ce.setPoints(points);
    groundBody->addCollisionShape(ce);

    points =
    {
        { 1690.f, 533.f },
        { 1670.f, 497.f },
        { 2054.f, 497.f },
        { 2034.f, 533.f }
    };
    ce.setPoints(points);
    groundBody->addCollisionShape(ce);

    points =
    {
        { 1230.f, 700.f },
        { 1210.f, 665.f },
        { 1584.f, 665.f },
        { 1564.f, 700.f }
    };
    ce.setPoints(points);
    groundBody->addCollisionShape(ce);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(groundBody);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
}

void PlatformDemoState::addPlayer()
{
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionRectangleShape cs({ 100.f, 160.f });
    cs.setFriction(0.6f);
    cs.setDensity(0.5f);
    
    body->fixedRotation(true);
    body->addCollisionShape(cs);

    auto controller = xy::Component::create<Plat::PlayerController>(m_messageBus);

    auto camera = xy::Component::create<xy::Camera>(m_messageBus, getContext().defaultView);
    camera->lockTransform(xy::Camera::TransformLock::AxisY);
    camera->lockBounds({ 0.f,0.f, 2816.f, 1080.f });

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(960.f, 540.f);
    entity->addComponent(body);
    playerController = entity->addComponent(controller);
    m_scene.setActiveCamera(entity->addComponent(camera));

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}