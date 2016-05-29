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
    //set up a default material to assign to newly created models
    m_defaultShader.loadFromMemory(DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);
    m_defaultMaterial = std::make_unique<Material>(m_defaultShader);
    
    //create the render buffer
    m_gBuffer.create(size.x, size.y, 4u, true, true);

    updateView();
    std::memset(&m_matrixBlock, 0, sizeof(m_matrixBlock));
    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16 * sizeof(float));
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.create(m_matrixBlock);

    m_defaultMaterial->addUniformBuffer(m_matrixBlockBuffer);
    
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
}

//public
std::unique_ptr<Model> MeshRenderer::createModel(MessageBus& mb, const Mesh& mesh)
{
    auto model = Component::create<Model>(mb, mesh, Lock());
    m_models.push_back(model.get());
    
    //set default material
    model->setBaseMaterial(*m_defaultMaterial);

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

void MeshRenderer::drawScene() const
{    
    m_gBuffer.setActive(true);
    
    auto viewPort = m_gBuffer.getView().getViewport();
    glViewport(
        static_cast<GLuint>(viewPort.left * xy::DefaultSceneSize.x), 
        static_cast<GLuint>(viewPort.top * xy::DefaultSceneSize.y),
        static_cast<GLuint>(viewPort.width * xy::DefaultSceneSize.x),
        static_cast<GLuint>(viewPort.height * xy::DefaultSceneSize.y));

    glClearColor(1.f, 1.f, 1.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE); //TODO apply these in material passes
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    for (const auto& m : m_models)m->draw(m_viewMatrix);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    m_gBuffer.display();
    

    //TODO get lights
    //foreach active light render scene to light depthmap
}

void MeshRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    drawScene();

    //m_ssaoTexture.clear(sf::Color::Transparent);
    //m_ssaoTexture.draw(m_ssaoSprite/*, &m_ssaoShader*/);
    //m_ssaoTexture.display();

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

    m_projectionMatrix = glm::perspective(fov, viewSize.x / viewSize.y, nearPlane, m_cameraZ * 2.f);
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16);

    m_ssaoShader.setUniform("u_projectionMatrix", sf::Glsl::Mat4(glm::value_ptr(m_projectionMatrix)));
    m_defaultShader.setUniform("u_farPlane", m_cameraZ * 2.f);
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
    }

    //turn off others by setting intensity to 0
    for (; i < MAX_LIGHTS; ++i)
    {
        m_lightingBlock.u_pointLights[i].intensity = 0.f;
    }

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
            m_lightingShader.setUniform("u_aoMap", m_ssaoTexture.getTexture());
            m_lightingShader.setUniform("u_illuminationMap", m_lightBlurTexture.getTexture());
        }
    }
    else
    {
        xy::Logger::log("Failed to create output shader for deferred renderer", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    m_outputQuad = std::make_unique<RenderQuad>(sf::Vector2f(200.f, 100.f), m_lightingShader);
}