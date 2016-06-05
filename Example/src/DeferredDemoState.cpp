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
#include <RotationComponent.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Const.hpp>

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
        "uniform sampler2D u_maskMap;\n"
        "uniform mat4 u_inverseWorldViewMatrix;\n"

        "const vec4 tangent = vec4(1.0, 0.0, 0.0, 1.0); \n"
        "const vec4 normal = vec4(0.0, 0.0, 1.0, 1.0); \n"

        /*"mat3 inverse(mat3 m)\n"
        "{\n"
        "    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];\n"
        "    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];\n"
        "    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];\n"

        "    float b01 = a22 * a11 - a12 * a21;\n"
        "    float b11 = -a22 * a10 + a12 * a20;\n"
        "    float b21 = a21 * a10 - a11 * a20;\n"

        "    float det = a00 * b01 + a01 * b11 + a02 * b21;\n"

        "    return mat3(b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),\n"
        "        b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),\n"
        "        b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) / det;\n"
        "}\n"*/

        "void main()\n"
        "{\n"

        /*"    mat3 normalMatrix = transpose(mat3(u_inverseWorldViewMatrix));\n"*/
        "    vec3 n = normalize(vec3(u_inverseWorldViewMatrix * normal));\n"
        "    vec3 t = normalize(vec3(u_inverseWorldViewMatrix * tangent));\n"
        /*"    t = normalize(t - (dot(t, n) * n));\n"*/
        "    vec3 b = cross(n,t);\n" \
        /*"    mat3 tangentSpaceTransformMatrix = mat3(t, b, n);\n"*/

        "    vec3 bumpNormal = texture2D(u_normalMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;\n"
        /*"    bumpNormal = normalize(t * bumpNormal.x + b * bumpNormal.y + n * bumpNormal.z);\n"*/

        "    gl_FragData[0] = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n"
        "    gl_FragData[1] = vec4(0.5 * (bumpNormal + 1.0), 1.0);\n"
        "    gl_FragData[2] = texture2D(u_maskMap, gl_TexCoord[0].xy);\n"
        "}";
}

DeferredDemoState::DeferredDemoState(xy::StateStack& stateStack, Context context)
    : State(stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene(m_messageBus)
{
    launchLoadingScreen();

    //m_scene.setView(context.defaultView);
    xy::Stats::clear();
    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 930.f); 

    m_labelText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_labelText.setPosition(40.f, 20.f);
    m_labelText.setString("OUTPUT                                               DIFFUSE                    NORMAL\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n                                                     MASK (R = intens, G = amount, B = illum)");
    
    //create g-buffer with 3 textures
    m_renderTexture.create(960, 1080, 3u);

    m_deferredShader.loadFromMemory(fragShader, sf::Shader::Fragment);
    //m_deferredShader.setUniform("u_rotationMatrix", rotationMatrix(90.f));

    m_normalMapShader.loadFromMemory(xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED_SPECULAR_ILLUM);
    m_normalMapShader.setUniform("u_diffuseMap", m_renderTexture.getTexture(0));
    m_normalMapShader.setUniform("u_normalMap", m_renderTexture.getTexture(1));
    m_normalMapShader.setUniform("u_maskMap", m_renderTexture.getTexture(2));
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
    rw.draw(m_labelText);
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
    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(480.f, 540.f);
    
    m_scene.setActiveCamera(entity->addComponent(camera));
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
    
    //moving light
    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 500.f, 250.f);
    light->setDepth(110.f);
    light->setDiffuseColour({ 255u, 185u, 135u });
    light->setIntensity(1.1f);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(light);
    entity->setPosition(120.f, 100.f);

    auto rotatingEntity = xy::Entity::create(m_messageBus);
    auto rotator = xy::Component::create<RotationComponent>(m_messageBus);
    rotatingEntity->addComponent(rotator);
    rotatingEntity->addChild(entity);
    rotatingEntity->setPosition(480.f, 540.f);
    m_scene.addEntity(rotatingEntity, xy::Scene::Layer::BackRear);


    //so we have flat colours behind the scene
    m_textureResource.setFallbackColour({ 100u, 100u, 100u });
    auto background = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("fallback_grey"));
    background->setScale(50.f, 80.f);
    m_textureResource.setFallbackColour({ 127, 127, 255 });
    background->setNormalMap(m_textureResource.get("fallback_normal"));
    m_textureResource.setFallbackColour(sf::Color::Black);
    background->setMaskMap(m_textureResource.get("fallback_mask"));
    background->setShader(m_deferredShader);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(background);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);


    //add a sprite to draw
    auto doofer = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("assets/images/deferred/run_diffuse.png"));
    doofer->loadAnimationData("assets/images/deferred/run.xya");
    doofer->playAnimation(0);
    doofer->setShader(m_deferredShader);
    doofer->setNormalMap(m_textureResource.get("assets/images/deferred/run_normal.png"));
    doofer->setMaskMap(m_textureResource.get("assets/images/deferred/run_mask.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(doofer);
    entity->setPosition(140.f, 140.f);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    doofer = xy::Component::create<xy::AnimatedDrawable>(m_messageBus, m_textureResource.get("assets/images/deferred/run_diffuse.png"));
    doofer->loadAnimationData("assets/images/deferred/run.xya");
    doofer->playAnimation(0);
    doofer->setShader(m_deferredShader);
    doofer->setNormalMap(m_textureResource.get("assets/images/deferred/run_normal.png"));
    doofer->setMaskMap(m_textureResource.get("assets/images/deferred/run_mask.png"));

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(doofer);
    entity->setPosition(590.f, 590.f);
    entity->setOrigin(150.f, 150.f);
    //entity->setRotation(-90.f);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
}