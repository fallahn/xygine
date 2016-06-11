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
#include <PlatformBackground.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/Console.hpp>
#include <xygine/util/Random.hpp>

#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/components/Camera.hpp>

#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>

#include <xygine/components/Model.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/mesh/IQMBuilder.hpp>

#include <xygine/shaders/NormalMapped.hpp>

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
        SpecularSmooth2D,
        SpecularBumped3D,
        SpecularSmooth3D,
        TexturedSmooth3D,
        TexturedSkinned
    };

    Plat::PlayerController * playerController = nullptr;
    sf::Uint8 playerInput = 0;
    bool showDebug = false;
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
    m_scene.setAmbientColour({ 91, 46, 13 });

    cacheMeshes();
    buildTerrain();
    buildPhysics();

    addItems();
    addPlayer();

    REPORT("Q", "Show Debug");
    xy::App::setMouseCursorVisible(true);

    quitLoadingScreen();

    xy::Console::addCommand("do_flaps", [](const std::string&) {xy::Console::print("bus flaps and dicketry"); }, this);
}

PlatformDemoState::~PlatformDemoState()
{
    xy::Console::unregisterCommands(this);
}

bool PlatformDemoState::update(float dt)
{    
    playerController->applyInput(playerInput);

    m_scene.update(dt);
    m_meshRenderer.update();

    //update lighting
    auto& shader = m_shaderResource.get(PlatformShaderId::SpecularSmooth2D);
    //shader.setUniform("u_ambientColour", sf::Glsl::Vec4(m_scene.getAmbientColour()));
    auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    auto i = 0u;
    for (; i < lights.size() && i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        auto light = lights[i];
        if (light)
        {
            const std::string idx = std::to_string(i);
            
            auto pos = light->getWorldPosition();
            shader.setUniform("u_pointLightPositions[" + std::to_string(i) + "]", pos);
            shader.setUniform("u_pointLights[" + idx + "].intensity", light->getIntensity());
            shader.setUniform("u_pointLights[" + idx + "].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
            shader.setUniform("u_pointLights[" + idx + "].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
            shader.setUniform("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());
        }
    }
    //switch off inactive lights
    for (; i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        shader.setUniform("u_pointLights[" + std::to_string(i) + "].intensity", 0.f);
    }
    
    return true;
}

void PlatformDemoState::draw()
{   
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);    
        
    rw.setView(getContext().defaultView);   
    rw.draw(m_meshRenderer);

    if (showDebug)
    {
        rw.setView(m_scene.getView());
        rw.draw(m_physWorld);
    }
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
            showDebug = !showDebug;
            m_scene.drawDebug(showDebug);
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
void PlatformDemoState::cacheMeshes()
{
    xy::CubeBuilder cb(100.f);
    m_meshRenderer.loadModel(MeshID::Cube, cb);

    xy::IQMBuilder ib("assets/models/mrfixit.iqm");
    m_meshRenderer.loadModel(MeshID::Fixit, ib);

    xy::IQMBuilder ib2("assets/models/platform_01.iqm");
    m_meshRenderer.loadModel(MeshID::Platform, ib2);

    xy::IQMBuilder ib3("assets/models/batcat.iqm");
    m_meshRenderer.loadModel(MeshID::Batcat, ib3);

    m_shaderResource.preload(PlatformShaderId::SpecularBumped3D, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    m_shaderResource.preload(PlatformShaderId::TexturedSkinned, DEFERRED_TEXTURED_BUMPED_SKINNED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    m_shaderResource.preload(PlatformShaderId::TexturedSmooth3D, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);
    m_shaderResource.preload(PlatformShaderId::SpecularSmooth3D, DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);

    auto& demoMaterial = m_materialResource.add(MatId::Demo, m_shaderResource.get(PlatformShaderId::SpecularBumped3D));
    demoMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    demoMaterial.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/platform/cube_diffuse.png") });
    demoMaterial.addProperty({ "u_normalMap", m_textureResource.get("assets/images/platform/cube_normal.png") });
    demoMaterial.addProperty({ "u_maskMap", m_textureResource.get("assets/images/platform/cube_mask.png") });
  
    auto& fixitMaterialBody = m_materialResource.add(MatId::MrFixitBody, m_shaderResource.get(PlatformShaderId::TexturedSkinned));
    fixitMaterialBody.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    fixitMaterialBody.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/fixit/fixitBody.png") });
    fixitMaterialBody.addProperty({ "u_normalMap", m_textureResource.get("assets/images/fixit/fixitBody_normal.png") });
    fixitMaterialBody.addProperty({ "u_maskMap", m_textureResource.get("assets/images/fixit/fixitBody_mask.png") });

    auto& fixitMaterialHead = m_materialResource.add(MatId::MrFixitHead, m_shaderResource.get(PlatformShaderId::TexturedSkinned));
    fixitMaterialHead.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    fixitMaterialHead.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/fixit/fixitHead.png") });
    fixitMaterialHead.addProperty({ "u_normalMap", m_textureResource.get("assets/images/fixit/fixitHead_normal.png") });
    fixitMaterialHead.addProperty({ "u_maskMap", m_textureResource.get("assets/images/fixit/fixitHead_mask.png") });

    auto& platformMaterial01 = m_materialResource.add(MatId::Platform01, m_shaderResource.get(PlatformShaderId::TexturedSmooth3D));
    platformMaterial01.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    platformMaterial01.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/platform/plat_01.png") });

    auto& platformMaterial04 = m_materialResource.add(MatId::Platform04, m_shaderResource.get(PlatformShaderId::TexturedSmooth3D));
    platformMaterial04.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    platformMaterial04.addProperty({ "u_diffuseMap", m_textureResource.get("assets/images/platform/plat_04.png") });

    auto& catMat = m_materialResource.add(MatId::BatcatMat, m_shaderResource.get(PlatformShaderId::TexturedSkinned));
    catMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    auto& tex = m_textureResource.get("assets/images/platform/batcat_diffuse.png");
    tex.setRepeated(true);
    catMat.addProperty({ "u_diffuseMap", tex });
    auto& tex2 = m_textureResource.get("assets/images/platform/batcat_normal.png");
    tex2.setRepeated(true);
    catMat.addProperty({ "u_normalMap", tex2 });
    auto& tex3 = m_textureResource.get("assets/images/platform/batcat_mask.png");
    tex3.setRepeated(true);
    catMat.addProperty({ "u_maskMap", tex3 });

    auto& lightMaterial = m_materialResource.add(MatId::LightSource, m_shaderResource.get(PlatformShaderId::SpecularSmooth3D));
    lightMaterial.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    lightMaterial.addProperty({ "u_colour", sf::Color(255, 255, 100) });
    lightMaterial.addProperty({ "u_maskColour", sf::Color::Blue });

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 800.f, 500.f, sf::Color(255, 255, 100));
    light->setDepth(400.f);

    auto model = m_meshRenderer.createModel(MeshID::Cube, m_messageBus);
    model->setPosition({ 0.f, 0.f, light->getWorldPosition().z });
    model->setSubMaterial(lightMaterial, 0);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    entity->setScale(0.25f, 0.25f);
    entity->addComponent(light);
    entity->addComponent(model);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);


    //---------------
    light = xy::Component::create<xy::PointLight>(m_messageBus, 600.f, 500.f, sf::Color(255, 255, 100));
    light->setDepth(100.f);

    model = m_meshRenderer.createModel(MeshID::Cube, m_messageBus);
    model->setPosition({ 0.f, 0.f, light->getWorldPosition().z });
    model->setSubMaterial(lightMaterial, 0);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(2000.f, 200.f);
    entity->setScale(0.25f, 0.25f);
    entity->addComponent(light);
    entity->addComponent(model);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void PlatformDemoState::buildTerrain()
{
    m_shaderResource.preload(PlatformShaderId::SpecularSmooth2D, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED);
    m_textureResource.setFallbackColour({ 127, 127, 255 });
    const auto& normalTexture = m_textureResource.get("normalFallback");
    
    auto background = xy::Component::create<Plat::Background>(m_messageBus, m_textureResource);
    background->setAmbientColour(m_scene.getAmbientColour());
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(background);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    auto drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/left_edge.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(256.f, 1080.f - 128.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1024.f, 1080.f - 128.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/ground_section.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1792.f, 1080.f - 128.f);
    entity->addComponent(drawable);
    
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/right_edge.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(2560.f, 0.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    auto model = m_meshRenderer.createModel(MeshID::Platform, m_messageBus);
    model->rotate(xy::Model::Axis::X, 90.f);
    model->rotate(xy::Model::Axis::Z, 180.f);
    model->setPosition({ 384.f, 158.f, 0.f });
    model->setBaseMaterial(m_materialResource.get(MatId::Platform01));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(400.f, 700.f);
    entity->addComponent(model);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_03.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(2000.f, 550.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------
    drawable = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    drawable->setTexture(m_textureResource.get("assets/images/platform/plat_02.png"));
    drawable->setNormalMap(normalTexture);
    drawable->setShader(m_shaderResource.get(PlatformShaderId::SpecularSmooth2D));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1670.f, 450.f);
    entity->addComponent(drawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    //-------------------------

    model = m_meshRenderer.createModel(MeshID::Platform, m_messageBus);
    model->rotate(xy::Model::Axis::X, 90.f);
    model->rotate(xy::Model::Axis::Z, 180.f);
    model->setPosition({ 384.f, 158.f, 0.f });
    model->setBaseMaterial(m_materialResource.get(MatId::Platform04));

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1210.f, 600.f);
    entity->addComponent(model);

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

void PlatformDemoState::addItems()
{
    std::function<void(const sf::Vector2f&)> createBox = 
        [this](const sf::Vector2f& position)
    {
        auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
        xy::Physics::CollisionRectangleShape cs({ 100.f, 100.f }, { -50.f, -50.f });
        cs.setDensity(11.f);
        cs.setFriction(0.7f);
        body->addCollisionShape(cs);

        auto model = m_meshRenderer.createModel(MeshID::Cube, m_messageBus);
        model->setBaseMaterial(m_materialResource.get(MatId::Demo));

        auto ent = xy::Entity::create(m_messageBus);
        ent->setPosition(position);
        ent->addComponent(body);
        ent->addComponent(model);

        m_scene.addEntity(ent, xy::Scene::BackFront);
    };

    for (auto i = 0u; i < 8u; ++i)
    {
        createBox({ xy::Util::Random::value(20.f, 2500.f), 20.f });
    }
}

void PlatformDemoState::addPlayer()
{
    auto body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    xy::Physics::CollisionRectangleShape cs({ 120.f, 240.f });
    cs.setFriction(0.6f);
    cs.setDensity(0.9f);

    body->fixedRotation(true);
    body->addCollisionShape(cs);

    auto controller = xy::Component::create<Plat::PlayerController>(m_messageBus);

    auto camera = xy::Component::create<xy::Camera>(m_messageBus, getContext().defaultView);
    camera->lockTransform(xy::Camera::TransformLock::AxisY);
    camera->lockBounds({ 0.f,0.f, 2816.f, 1080.f });

    auto model = m_meshRenderer.createModel(MeshID::Batcat, m_messageBus);
    model->setBaseMaterial(m_materialResource.get(MatId::BatcatMat));
    model->rotate(xy::Model::Axis::X, 90.f);
    model->rotate(xy::Model::Axis::Z, -90.f);
    model->setPosition({ 60.f, 240.f, 0.f });

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(960.f, 540.f);
    entity->addComponent(body);
    entity->addComponent(model);
    playerController = entity->addComponent(controller);
    m_scene.setActiveCamera(entity->addComponent(camera));
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);



    body = xy::Component::create<xy::Physics::RigidBody>(m_messageBus, xy::Physics::BodyType::Dynamic);
    body->fixedRotation(true);
    cs.setRect({ 200.f, 300.f });
    body->addCollisionShape(cs);
    cs.setRect({ 80.f, 80.f }, { 60.f, -80.f });
    body->addCollisionShape(cs);

    model = m_meshRenderer.createModel(MeshID::Fixit, m_messageBus);
    model->setSubMaterial(m_materialResource.get(MatId::MrFixitBody), 0);
    model->setSubMaterial(m_materialResource.get(MatId::MrFixitHead), 1);
    model->rotate(xy::Model::Axis::X, 90.f);
    model->rotate(xy::Model::Axis::Z, 90.f);
    model->setScale({ 50.f, 50.f, 50.f });
    model->setPosition({ 100.f, 300.f, 0.f });

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1320.f, 40.f);
    entity->addComponent(model);
    entity->addComponent(body);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}