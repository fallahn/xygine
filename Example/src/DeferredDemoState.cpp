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

#include <CommandIds.hpp>
#include <DeferredDemoState.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/util/Position.hpp>

#include <xygine/PostBloom.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/shaders/NormalMapped.hpp>

#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/PointLight.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
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

    const std::string fragShader =
        "#version 120\n"
        "uniform sampler2D u_diffuseMap;\n"
        "uniform sampler2D u_normalMap;\n"
        "uniform mat4 u_inverseWorldViewMatrix;\n"

        "void main()\n"
        "{\n"
        "    gl_FragData[0] = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n"
        "    gl_FragData[1] = texture2D(u_normalMap, gl_TexCoord[0].xy);\n"
        "    gl_FragData[2] = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}";
}

DeferredDemoState::DeferredDemoState(xy::StateStack& stateStack, Context context)
    : State(stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene(m_messageBus)
{
    launchLoadingScreen();

    //m_scene.setView(context.defaultView);

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 630.f); 
    
    //create g-buffer with 3 textures
    m_renderTexture.create(960, 1080, 3);

    m_deferredShader.loadFromMemory(fragShader, sf::Shader::Fragment);
    m_normalMapShader.loadFromMemory(xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED);
    m_normalMapShader.setUniform("u_diffuseMap", m_renderTexture.getTexture(0));
    m_normalMapShader.setUniform("u_normalMap", m_renderTexture.getTexture(1));
    //m_normalMapShader.setUniform("u_directionalLight.intensity", 0.6f);
    //m_normalMapShader.setUniform("u_directionalLightDirection", sf::Glsl::Vec3(0.f, 0.f, -1.f));

    //diffuse target
    m_sprites[0].setTexture(m_renderTexture.getTexture(0));
    m_sprites[0].setScale(0.5f, -0.5f);
    m_sprites[0].setPosition(960.f, 540.f);
    //normal map
    m_sprites[1].setTexture(m_renderTexture.getTexture(1));
    m_sprites[1].setScale(0.5f, -0.5f);
    m_sprites[1].setPosition(1440.f, 540.f);
    //mask map
    m_sprites[2].setTexture(m_renderTexture.getTexture(2));
    m_sprites[2].setScale(0.5f, -0.5f);
    m_sprites[2].setPosition(960.f, 1080.f);

    //rendered output from combined g-buffer
    m_sprites[3].setTexture(m_renderTexture.getTexture(0));
    m_sprites[3].setScale(1.f, -1.f);
    m_sprites[3].setPosition(0.f, 1080.f);
    m_normalMapShader.setUniform("u_inverseWorldViewMatrix", sf::Glsl::Mat4(m_sprites[3].getTransform().getInverse()));

    buildScene();

    quitLoadingScreen();
}

bool DeferredDemoState::update(float dt)
{    
    m_scene.update(dt);

    //update the normal map shader with light properties
    auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    auto i = 0u;
    for (; i < lights.size() && i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        auto light = lights[i];
        //if (light)
        {
            const std::string idx = std::to_string(i);
            auto pos = light->getWorldPosition();

            m_normalMapShader.setUniform("u_pointLightPositions[" + idx + "]", pos);
            m_normalMapShader.setUniform("u_pointLights[" + idx + "].intensity", light->getIntensity());
            m_normalMapShader.setUniform("u_pointLights[" + idx + "].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
            //m_normalMapShader.setUniform("u_pointLights[" + idx + "].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
            m_normalMapShader.setUniform("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());
        }
    }

    //switch off inactive lights
    for (; i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        const auto idx = std::to_string(i);
        m_normalMapShader.setUniform("u_pointLights[" + idx + "].intensity", 0.f);
    }


    m_reportText.setString(xy::Stats::getString());
    return true;
}

void DeferredDemoState::draw()
{
    m_renderTexture.clear(sf::Color::Transparent);
    m_renderTexture.draw(m_scene);
    m_renderTexture.display();
    
    auto& rw = getContext().renderWindow;
    //rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    
    for (auto i = 0u; i < 3u; ++i)
    {
        rw.draw(m_sprites[i]);
    }
    rw.draw(m_sprites[3], &m_normalMapShader);

    rw.draw(m_reportText);
}

bool DeferredDemoState::handleEvent(const sf::Event& evt)
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

void DeferredDemoState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void DeferredDemoState::buildScene()
{
    //use a special camera to render to the texture if it's not the default scene size
    auto camera = xy::Component::create<xy::Camera>(m_messageBus, sf::View({}, { 960.f, 1080.f }));
    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 500.f, 250.f);
    light->setDepth(110.f);
    light->setDiffuseColour({ 255u, 185u, 135u });
    light->setIntensity(1.1f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(480.f, 540.f);
    entity->addComponent(light);
    m_scene.setActiveCamera(entity->addComponent(camera));
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
    
    //so we have flat colours behind the scene
    m_textureResource.setFallbackColour({ 100u, 100u, 100u });
    auto background = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("fallback_grey"));
    background->setScale(50.f, 80.f);
    m_textureResource.setFallbackColour({ 127, 127, 255 });
    background->setNormalMap(m_textureResource.get("fallback_normal"));
    background->setShader(m_deferredShader);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(background);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);


    //add a sprite to draw
    auto doofer = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("assets/images/deferred/doofer_dead.png"));
    doofer->loadAnimationData("assets/images/deferred/doofer_dead.xya");
    doofer->playAnimation(0);
    doofer->setScale(4.f, 4.f);
    doofer->setShader(m_deferredShader);
    doofer->setNormalMap(m_textureResource.get("assets/images/deferred/doofer_dead_normal.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(doofer);
    entity->setPosition(200.f, 200.f);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);



    doofer = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("assets/images/deferred/doofer_dead.png"));
    doofer->loadAnimationData("assets/images/deferred/doofer_dead.xya");
    doofer->playAnimation(0);
    doofer->setScale(4.f, 4.f);
    doofer->setShader(m_deferredShader);
    doofer->setNormalMap(m_textureResource.get("assets/images/deferred/doofer_dead_normal.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(doofer);
    entity->setPosition(600.f, 600.f);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}