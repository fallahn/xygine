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

#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/mesh/shaders/DeferredLighting.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/mesh/shaders/SSAO.hpp>
#include <xygine/mesh/shaders/LightBlur.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Console.hpp>

#include <SFML/Graphics/RenderStates.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>

using namespace xy;

namespace
{
    const float fov = 30.f * xy::Util::Const::degToRad;
    const float nearPlane = 0.1f;
    const unsigned MAX_LIGHTS = 8;
}

MeshRenderer::MeshRenderer(const sf::Vector2u& size, const Scene& scene)
    : m_scene               (scene),
    m_matrixBlockBuffer     ("u_matrixBlock"),
    m_lightingBlockBuffer   ("u_lightBlock"),
    m_doLightBlur           (true),
    m_lightingBlockID       (-1)
{
    //create the render buffer
    m_gBuffer.create(size.x, size.y, 4u, true);

    //use floating point textures for position and normals
    std::function<void(int)> useFloatingpoint = [size, this](int id)
    {
        glCheck(glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(id).getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, 0));
        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    };
    useFloatingpoint(MaterialChannel::Normal);
    useFloatingpoint(MaterialChannel::Position);

    updateView();
    std::memset(&m_matrixBlock, 0, sizeof(m_matrixBlock));
    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16 * sizeof(float));
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.create(m_matrixBlock);

    m_materialResource.get(MaterialResource::Static).addUniformBuffer(m_matrixBlockBuffer);

    //set lighting uniform buffer
    std::memset(&m_lightingBlock, 0, sizeof(m_lightingBlock));
    m_lightingBlockBuffer.create(m_lightingBlock);

    //set up the buffer for ssao
    //initSSAO();

    //performs a light blur pass
    initSelfIllum();

    //prepare the lighting shader for the output
    initOutput();

    //we need this for the output kludge in draw()
    sf::Image img;
    img.create(2, 2, sf::Color::Transparent);
    m_dummyTetxure.loadFromImage(img);
    m_dummySprite.setTexture(m_dummyTetxure);

    //inits console commands which affect renderer
    setupConCommands();

    //testing depth texture
    m_depthTexture.create(1024u, 1024u, MAX_LIGHTS);
}

MeshRenderer::~MeshRenderer()
{
    Console::unregisterCommands(this);
}

//public
void MeshRenderer::loadModel(std::int32_t id, ModelBuilder& mb)
{
    m_meshResource.add(id, mb);

    //TODO check for any material properties and map their IDs
    //to material resource

    //check for skeleton property and cache along with any animations if they are found
    auto skeleton = mb.getSkeleton();
    if (skeleton && m_animationResource.find(id) == m_animationResource.end())
    {
        const auto& anims = mb.getAnimations();
        m_animationResource.insert(std::make_pair(id, AnimationData(skeleton, anims)));
    }
}

std::unique_ptr<Model> MeshRenderer::createModel(std::int32_t id, MessageBus&mb)
{
    auto& mesh = m_meshResource.get(id);
    auto model = createModel(mesh, mb);

    //TODO check material ID for mesh ID and add materials if they exist

    //check for skeleton, and animations
    auto result = m_animationResource.find(id);
    if (result != m_animationResource.end())
    {
        model->setSkeleton(*result->second.skeleton.get());
        model->setAnimations(result->second.animations);
        //TODO set material to default skinned
    }

    return std::move(model);
}

std::unique_ptr<Model> MeshRenderer::createModel(const Mesh& mesh, MessageBus& mb)
{
    auto model = Component::create<Model>(mb, mesh, Lock());
    m_models.push_back(model.get());

    //set default material
    model->setBaseMaterial(m_materialResource.get(MaterialResource::Static));

    return std::move(model);
}

void MeshRenderer::update()
{
    m_models.erase(std::remove_if(m_models.begin(), m_models.end(),
        [](const Model* m)
    {
        return m->destroyed();
    }), m_models.end());


    auto view = m_scene.getView();
    auto camPos = view.getCenter();
    auto rotation = view.getRotation() * xy::Util::Const::degToRad;
    glm::vec3 camWorldPosition(camPos.x, camPos.y, m_cameraZ);

    //update UBO with scene lighting / cam pos
    updateLights(camWorldPosition);

    m_viewMatrix = glm::translate(glm::mat4(), camWorldPosition);
    //rotate
    m_viewMatrix = glm::rotate(m_viewMatrix, rotation, glm::vec3(0.f, 0.f, 1.f));
    m_viewMatrix = glm::inverse(m_viewMatrix);

    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.update(m_matrixBlock);
}

void MeshRenderer::handleMessage(const Message& msg)
{
    //handle camera changed event
    if (msg.id == xy::Message::SceneMessage)
    {
        const auto& msgData = msg.getData<xy::Message::SceneEvent>();
        switch (msgData.action)
        {
        default:
        case xy::Message::SceneEvent::CameraChanged:
            updateView();
            break;
        }
    }
}

void MeshRenderer::enableGlowPass(bool enable)
{
    m_doLightBlur = enable;
    if (enable)
    {
        m_lightingShader.setUniform("u_illuminationMap", m_lightBlurTexture.getTexture());
    }
    else
    {
        m_lightingShader.setUniform("u_illuminationMap", m_lightFallback);
    }
}

//private
void MeshRenderer::createNoiseTexture()
{
    std::array<sf::Glsl::Vec3, 16u> data;
    for (auto& v : data)
    {
        v =
        {
            xy::Util::Random::value(-1.f, 1.f),
            xy::Util::Random::value(-1.f, 1.f),
            0.f
        };
    }
    m_ssaoNoiseTexture.create(4, 4);
    m_ssaoNoiseTexture.setRepeated(true);
    glCheck(glBindTexture(GL_TEXTURE_2D, m_ssaoNoiseTexture.getNativeHandle()));
    glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, data.data()));
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

void MeshRenderer::drawDepth() const
{
    auto visibleArea = m_scene.getVisibleArea();
    std::size_t drawCount = 0;
    //get light count
    auto lightCount = m_scene.getVisibleLights(visibleArea).size();
    //TODO get visible area from light POV to cull geometry


    //foreach active light render scene to light depthmap
    
    auto viewPort = m_depthTexture.getView().getViewport();
    auto size = m_depthTexture.getSize();

    //glCheck(glClearDepth(0.f));
    for (auto i = 0u; i < lightCount; ++i)
    {
        //set projection to current light
        std::memcpy(m_matrixBlock.u_lightWorldViewProjectionMatrix, m_lightingBlock.u_pointLights[i].wvpMatrix, sizeof(float) * 16);

        //set active texture layer
        m_depthTexture.setActive(true);
        m_depthTexture.setLayerActive(i);

        glViewport(
            static_cast<GLuint>(viewPort.left * size.x),
            static_cast<GLuint>(viewPort.top * size.y),
            static_cast<GLuint>(viewPort.width * size.x),
            static_cast<GLuint>(viewPort.height * size.y));

        glCheck(glClearColor(1.f, 1.f, 1.f, 0.f));
        glCheck(glEnable(GL_DEPTH_TEST));
        glCheck(glEnable(GL_CULL_FACE));
        glCheck(glClear(GL_DEPTH_BUFFER_BIT));
        for (const auto& m : m_models)
        {
            drawCount += m->draw(m_viewMatrix, visibleArea, RenderPass::ShadowMap);
        }
        glCheck(glDisable(GL_CULL_FACE));
        glCheck(glDisable(GL_DEPTH_TEST));
        m_depthTexture.display(); 
    }
     
    REPORT("Shadow draw count", std::to_string(drawCount));
}

void MeshRenderer::drawScene() const
{
    auto visibleArea = m_scene.getVisibleArea();
    std::size_t drawCount = 0;

    m_gBuffer.setActive(true);
    auto viewPort = m_gBuffer.getView().getViewport();
    auto size = m_gBuffer.getSize();
    glViewport(
    static_cast<GLuint>(viewPort.left * size.x),
    static_cast<GLuint>(viewPort.top * size.y),
    static_cast<GLuint>(viewPort.width * size.x),
    static_cast<GLuint>(viewPort.height * size.y));

    glCheck(glClearColor(1.f, 1.f, 1.f, 0.f));
        
    glCheck(glEnable(GL_CULL_FACE));
    glCheck(glEnable(GL_DEPTH_TEST));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    drawCount = 0;
    for (const auto& m : m_models)
    {
        drawCount += m->draw(m_viewMatrix, visibleArea, RenderPass::Default);
    }
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_CULL_FACE));
    m_gBuffer.display();
    
    REPORT("Draw Count", std::to_string(drawCount));
}

void MeshRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    drawDepth();
    drawScene();

    /*m_ssaoTexture.clear(sf::Color::Transparent);
    m_ssaoTexture.draw(m_ssaoSprite, &m_ssaoShader);
    m_ssaoTexture.display();*/

    if (m_doLightBlur)
    {
        m_lightDownsampleTexture.clear();
        m_lightDownsampleTexture.draw(m_downSampleSprite, &m_lightDownsampleShader);
        m_lightDownsampleTexture.display();

        m_lightBlurTexture.clear();
        m_lightBlurTexture.draw(m_lightBlurSprite, &m_lightBlurShader);
        m_lightBlurTexture.display();
    }

    //this is a kludge, as it seems the only way to activate
    //a render target is to draw something to it, which we need to
    //do in order to bind the lighting UBO to the correct context
    rt.draw(m_dummySprite);

    m_lightingBlockBuffer.bind(m_lightingShader.getNativeHandle(), m_lightingBlockID);
    rt.draw(*m_outputQuad);
    rt.resetGLStates();
}

void MeshRenderer::updateView()
{
    auto viewSize = m_scene.getView().getSize();

    //calc how far away from the scene the camera would be
    //assuming the 2D world is drawn at 0 depth
    const float angle = std::tan(fov / 2.f);
    m_cameraZ = ((viewSize.y / 2.f) / angle);

    m_projectionMatrix = glm::perspective(fov, viewSize.x / viewSize.y, m_cameraZ * 0.8f, m_cameraZ * 1.2f);
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16);

    //m_ssaoShader.setUniform("u_projectionMatrix", sf::Glsl::Mat4(glm::value_ptr(m_projectionMatrix)));
    //m_defaultShader.setUniform("u_farPlane", m_cameraZ * 2.f);
}

void MeshRenderer::updateLights(const glm::vec3& camWorldPosition)
{
    m_lightingBlock.u_cameraWorldPosition[0] = camWorldPosition.x;
    m_lightingBlock.u_cameraWorldPosition[1] = camWorldPosition.y;
    m_lightingBlock.u_cameraWorldPosition[2] = camWorldPosition.z;

    auto ambient = m_scene.getAmbientColour();
    m_lightingBlock.u_ambientColour[0] = static_cast<float>(ambient.r) / 255.f;
    m_lightingBlock.u_ambientColour[1] = static_cast<float>(ambient.g) / 255.f;
    m_lightingBlock.u_ambientColour[2] = static_cast<float>(ambient.b) / 255.f;
    m_lightingBlock.u_ambientColour[3] = static_cast<float>(ambient.a) / 255.f;

    //update active lights
    const auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    auto i = 0u;
    for (; i < MAX_LIGHTS && i < lights.size(); ++i)
    {
        auto& colour = lights[i]->getDiffuseColour();
        m_lightingBlock.u_pointLights[i].diffuseColour[0] = static_cast<float>(colour.r) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[1] = static_cast<float>(colour.g) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[2] = static_cast<float>(colour.b) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[3] = static_cast<float>(colour.a) / 255.f;

        auto& spec = lights[i]->getSpecularColour();
        m_lightingBlock.u_pointLights[i].specularColour[0] = static_cast<float>(spec.r) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[1] = static_cast<float>(spec.g) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[2] = static_cast<float>(spec.b) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[3] = static_cast<float>(spec.a) / 255.f;

        m_lightingBlock.u_pointLights[i].intensity = lights[i]->getIntensity();
        m_lightingBlock.u_pointLights[i].inverseRange = lights[i]->getInverseRange();

        auto& position = lights[i]->getWorldPosition();
        m_lightingBlock.u_pointLights[i].position[0] = position.x;
        m_lightingBlock.u_pointLights[i].position[1] = position.y;
        m_lightingBlock.u_pointLights[i].position[2] = position.z;

        //use this to render a depth map with the light pointing to the centre
        //of the viewable area, with a max depth of the light range
        //TODO cache as much of this as possible
        auto lightPerspectiveMatrix = glm::perspective(90.f, xy::DefaultSceneSize.x / xy::DefaultSceneSize.y, 0.f, /*1.f / lights[i]->getInverseRange()*/m_cameraZ);
        auto lightWorldMatrix = glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(camWorldPosition.x, camWorldPosition.y, 0.f), glm::vec3(0.f, 1.f,0.f));
        std::memcpy(m_lightingBlock.u_pointLights[i].wvpMatrix, glm::value_ptr(lightPerspectiveMatrix * m_viewMatrix * lightWorldMatrix), 16 * sizeof(float));
    }

    //turn off others by setting intensity to 0
    for (; i < MAX_LIGHTS; ++i)
    {
        m_lightingBlock.u_pointLights[i].intensity = 0.f;
    }

    m_lightBlurShader.setUniform("u_lightCount", int(std::min(std::size_t(MAX_LIGHTS), lights.size())));
    m_lightingBlockBuffer.update(m_lightingBlock);
}

void MeshRenderer::initSSAO()
{
    for (auto i = 0u; i < m_ssaoKernel.size(); ++i)
    {
        const float scale = static_cast<float>(i) / m_ssaoKernel.size();
        m_ssaoKernel[i] =
        {
            xy::Util::Random::value(-1.f, 1.f),
            xy::Util::Random::value(-1.f, 1.f),
            xy::Util::Random::value(0.f, 1.f)
        };
        m_ssaoKernel[i] *= (0.1f + 0.9f * scale * scale);
    }
    createNoiseTexture();
    m_ssaoShader.loadFromMemory(xy::Shader::Mesh::QuadVertex, xy::Shader::Mesh::SSAOFragment2);
    m_ssaoShader.setUniformArray("u_kernel", m_ssaoKernel.data(), m_ssaoKernel.size());
    m_ssaoShader.setUniform("u_projectionMatrix", sf::Glsl::Mat4(glm::value_ptr(m_projectionMatrix)));
    m_ssaoShader.setUniform("u_noiseMap", m_ssaoNoiseTexture);
    m_ssaoShader.setUniform("u_positionMap", m_gBuffer.getTexture(MaterialChannel::Position));
    m_ssaoShader.setUniform("u_normalMap", m_gBuffer.getTexture(MaterialChannel::Normal));
    m_ssaoTexture.create(960, 540);
    m_ssaoTexture.setSmooth(true);
    m_ssaoSprite.setTexture(m_gBuffer.getTexture(MaterialChannel::Position));
}

void MeshRenderer::initSelfIllum()
{
    m_lightDownsampleTexture.create(480, 270);
    m_lightDownsampleTexture.setSmooth(true);
    m_downSampleSprite.setTexture(m_gBuffer.getTexture(MaterialChannel::Diffuse));
    m_lightDownsampleShader.loadFromMemory(Shader::Mesh::QuadVertex, Shader::Mesh::LightDownsampleFrag);
    m_lightDownsampleShader.setUniform("u_diffuseMap", m_gBuffer.getTexture(MaterialChannel::Diffuse));
    m_lightDownsampleShader.setUniform("u_maskMap", m_gBuffer.getTexture(MaterialChannel::Mask));

    m_lightBlurTexture.create(480, 270);
    m_lightBlurTexture.setSmooth(true);
    m_lightBlurSprite.setTexture(m_lightDownsampleTexture.getTexture());
    m_lightBlurShader.loadFromMemory(Shader::Mesh::QuadVertex, Shader::Mesh::LightBlurFrag);
    m_lightBlurShader.setUniform("u_diffuseMap", m_lightDownsampleTexture.getTexture());

    //use this when disabling the blur output
    sf::Image img;
    img.create(2, 2, sf::Color::Black);
    m_lightFallback.loadFromImage(img);
}

void MeshRenderer::initOutput()
{
    if (m_lightingShader.loadFromMemory(xy::Shader::Mesh::LightingVert, xy::Shader::Mesh::LightingFrag))
    {
        glCheck(m_lightingBlockID = glGetUniformBlockIndex(m_lightingShader.getNativeHandle(), m_lightingBlockBuffer.getName().c_str()));
        if (m_lightingBlockID == GL_INVALID_INDEX)
        {
            xy::Logger::log("Failed to find uniform ID for lighting block in deferred output", xy::Logger::Type::Error, xy::Logger::Output::All);
        }
        else
        {
            m_lightingShader.setUniform("u_diffuseMap", m_gBuffer.getTexture(MaterialChannel::Diffuse));
            m_lightingShader.setUniform("u_normalMap", m_gBuffer.getTexture(MaterialChannel::Normal));
            m_lightingShader.setUniform("u_maskMap", m_gBuffer.getTexture(MaterialChannel::Mask));
            m_lightingShader.setUniform("u_positionMap", m_gBuffer.getTexture(MaterialChannel::Position));
            //m_lightingShader.setUniform("u_aoMap", m_ssaoTexture.getTexture());
            m_lightingShader.setUniform("u_illuminationMap", m_lightBlurTexture.getTexture());
        }
    }
    else
    {
        xy::Logger::log("Failed to create output shader for deferred renderer", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    if (m_debugShader.loadFromMemory(xy::Shader::Mesh::LightingVert, xy::Shader::Mesh::DebugFrag))
    {
        m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Diffuse));
    }
    else
    {
        xy::Logger::log("Failed to create output shader for mesh debug", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    //---temp---//
    m_depthShader.loadFromMemory(xy::Shader::Mesh::LightingVert, xy::Shader::Mesh::DepthFrag);
    //----------//
    m_outputQuad = std::make_unique<RenderQuad>(sf::Vector2f(200.f, 100.f), m_lightingShader);
    m_outputQuad->addRenderPass(RenderPass::Debug, m_debugShader);
    //---temp---//
    m_outputQuad->addRenderPass(RenderPass::ShadowMap, m_depthShader);
}

void MeshRenderer::setupConCommands()
{
    //allows disabling the self-illum glow pass
    Console::addCommand("r_glowEnable",
        [this](const std::string& params)
    {
        if (params.find_first_of('0') == 0 ||
            params.find_first_of("false") == 0)
        {
            enableGlowPass(false);
        }

        else if (params.find_first_of('1') == 0 ||
            params.find_first_of("true") == 0)
        {
            enableGlowPass(true);
        }

        else
        {
            Console::print("r_glowEnable: valid parameters are 0, 1, false or true");
        }
    }, this);

    //allows switching the output to one of the gbuffer textures
    Console::addCommand("r_gBuffer",
        [this](const std::string& params)
    {
        if (params.find_first_of("0") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Default);
            Console::print("Switched output to default");
        }
        else if (params.find_first_of("1") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Diffuse));
            Console::print("Switched output to diffuse");
        }
        else if (params.find_first_of("2") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Normal));
            Console::print("Switched output to world normals");
        }
        else if (params.find_first_of("3") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Mask));
            Console::print("Switched output to mask map");
        }
        else if (params.find_first_of("4") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Position));
            Console::print("Switched output to world positions");
        }
        else
        {
            Console::print("r_gBuffer: 0 Default, 1 Diffuse, 2 Normals, 3 Mask, 4 Position");
        }
    }, this);

    Console::addCommand("r_showDepthMap", 
        [this](const std::string& params)
    {
        //TODO check layer parameter
        //TODO disable output again

        m_outputQuad->setActivePass(RenderPass::ShadowMap);
        sf::Shader::bind(&m_depthShader);
        auto loc = glGetUniformLocation(m_depthShader.getNativeHandle(), "u_texture");
        glCheck(glActiveTexture(GL_TEXTURE0)); //we know this shader only has one texture sampler
        glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthTexture.getNativeHandle()));
        glCheck(glUniform1i(loc, 0));

        Console::print("Switched output to depth map");
    }, this);
}