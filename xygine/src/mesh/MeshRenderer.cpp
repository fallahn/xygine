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
#include <xygine/mesh/shaders/ForwardLighting.hpp>
#include <xygine/mesh/shaders/SSAO.hpp>
#include <xygine/mesh/shaders/LightBlur.hpp>
#include <xygine/mesh/shaders/SSWater.hpp>
#include <xygine/shaders/PostGaussianBlur.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Console.hpp>
#include <xygine/App.hpp>

#include <SFML/Graphics/RenderStates.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <xygine/imgui/imgui.h>

#include <cstring>

using namespace xy;

namespace
{
    const unsigned MAX_LIGHTS = 8;
    const unsigned DepthTextureUnit = 10u; //only used in a single shader which we know won't have this many textures
    const unsigned shadowmapSize = 1024u;

    //used to offset the enum values of shader IDs
    const std::uint32_t shadowShaderOffset = 5000u;
    const std::uint32_t materialShaderOffset = 10000u;


    void createWaves(float freqX, float freqY, sf::Image& dst)
    {       
        static constexpr sf::Uint32 size = 256;
        std::array<sf::Uint8, size * size * 4> data;
        std::memset(data.data(), 0, data.size());

        for (auto y = 0u; y < size; ++y)
        {
            for (auto x = 0u; x < size; ++x)
            {
                float value = std::sin(((static_cast<float>(x) / size) * xy::Util::Const::TAU) * freqX) * 128.f + 127.f;
                value += std::sin(((static_cast<float>(y) / size) * xy::Util::Const::TAU) * freqY) * 128.f + 127.f;
                value = std::max(0.f, std::min(255.f, value / 2.f));
                
                auto idx = (y * size + x) * 4;
                data[idx] = static_cast<sf::Uint8>(value);
            }
        }

        dst.create(size, size, data.data());
    }
}

MeshRenderer::MeshRenderer(const sf::Vector2u& size, const Scene& scene)
    : m_scene               (scene),
    m_matrixBlockBuffer     ("u_matrixBlock"),
    m_lightingBlockBuffer   ("u_lightBlock"),
    m_doLightBlur           (true),
    m_lightingBlockID       (-1),
    m_nearRatio             (0.8f),
    m_farRatio              (1.2f),
    m_fov                   (30.f)
{
    auto width = size.x;
    auto height = size.y;
    if (width > height)
    {
        height = (width / 16) * 9;
    }
    else
    {
        width = (height / 9) * 16;
    }
    
    resizeGBuffer(width, height);

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

    if (!m_waterShader.loadFromMemory(Shader::Mesh::WaterFragment, sf::Shader::Fragment))
    {
        XY_ASSERT(false, "Failed loading water shader");
    }

    sf::Image img;
    createWaves(2.f, 4.f, img);
    m_surfaceTexture.loadFromImage(img);
    m_surfaceTexture.setRepeated(true);
    m_waterShader.setUniform("u_positionMap", m_gBuffer.getTexture(MaterialChannel::Position));
    m_waterShader.setUniform("u_surfaceMap", m_surfaceTexture);

    //we need this for the output kludge in draw()
    img.create(2, 2, sf::Color::Transparent);
    m_dummyTexture.loadFromImage(img);
    m_dummySprite.setTexture(m_dummyTexture);

    //inits console commands which affect renderer
    setupConCommands();

#ifdef _DEBUG_
    addDebugMenus();
#else
    //create con command to enable debug menu
    static bool enabled = false;
    Console::addCommand("mesh_debug_menu",
        [this](const std::string& params)
    {
        if (params == "true" && !enabled)
        {
            enabled = true;
            addDebugMenus();
        }
        else if (params == "false")
        {
            enabled = false;
            App::removeUserWindows(this);
        }
        else
        {
            Console::print(params + ": invalid value. Options are true or false");
        }
    });
#endif
}

MeshRenderer::~MeshRenderer()
{
    Console::unregisterCommands(this);
//#ifdef _DEBUG_
    App::removeUserWindows(this);
//#endif
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
    model->m_meshRenderer = this;

    return std::move(model);
}

std::unique_ptr<MeshDrawable> MeshRenderer::createDrawable(MessageBus& mb)
{
    auto md = xy::Component::create<MeshDrawable>(mb, *this, Lock());
    return std::move(md);
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
    m_waterShader.setUniform("u_cameraWorldPosition", sf::Glsl::Vec3(camPos.x, camPos.y, m_cameraZ));
    m_waterShader.setUniform("u_time", m_shaderClock.getElapsedTime().asSeconds() * 10.f);

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
    else if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            std::int32_t resolution = static_cast<std::int32_t>(msgData.value);
            auto width = resolution >> 16;
            auto height = resolution & 0xffff;

            if (width > height)
            {
                height = (width / 16) * 9;
            }
            else
            {
                width = (height / 9) * 16;
            }

            resizeGBuffer(static_cast<sf::Uint32>(width), static_cast<sf::Uint32>(height));
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

void MeshRenderer::setNearFarRatios(float n, float f)
{
    XY_ASSERT(n > 0 && n < 1.f, "Near ratio must be between 0 and 1");
    XY_ASSERT(f > 1 && f < 100.f, "Far ratio must be greater than 1");

    m_nearRatio = n;
    m_farRatio = f;
    updateView();
}

void MeshRenderer::setFOV(float fov)
{
    XY_ASSERT(fov >= 10 && fov <= 180, "FOV out of range");
    m_fov = fov;
    updateView();
    for (auto m : m_models)
    {
        //if (!m->destroyed())
        {
            m->update2DScale();
        }
    }
}

void MeshRenderer::enableTransparency(Material& mat, Material::Description description) const
{
    if (!m_shaderResource.exists(description))
    {
        switch (description)
        {
        default: break;
        case Material::Coloured:
            m_shaderResource.preload(description, 
                ALPHABLEND_COLOURED_VERTEX, ALPHABLEND_COLOURED_FRAGMENT);
            break;
        case Material::ColouredBumped:
            m_shaderResource.preload(description,
                ALPHABLEND_COLOURED_BUMPED_VERTEX, ALPHABLEND_COLOURED_BUMPED_FRAGMENT);
            break;
        case Material::ColouredSkinned:
            m_shaderResource.preload(description, 
                "#version 150\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define SKINNED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::ColouredSkinnedBumped:
            m_shaderResource.preload(description, 
                "#version 150\n#define SKINNED\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define SKINNED\n#define BUMP\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::Textured:
            m_shaderResource.preload(description, 
                "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::TexturedBumped:
            m_shaderResource.preload(description, 
                "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::TexturedSkinned:
            m_shaderResource.preload(description, 
                "#version 150\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::TexturedSkinnedBumped:
            m_shaderResource.preload(description, 
                "#version 150\n#define TEXTURED\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::VertexColoured:
            m_shaderResource.preload(description, 
                "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::VertexColouredBumped:
            m_shaderResource.preload(description, 
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::VertexColouredSkinned:
            m_shaderResource.preload(description, 
                "#version 150\n#define VERTEX_COLOUR\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define SKINNED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        case Material::VertexColouredSkinnedBumped:
            m_shaderResource.preload(description, 
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::ForwardFragment);
            break;
        }
    }
    
    auto& shader = m_shaderResource.get(description);
    shader.setUniform("u_depthMaps", static_cast<int>(DepthTextureUnit));
    mat.addRenderPass(RenderPass::AlphaBlend, shader);
    mat.addUniformBuffer(getLightingUniforms());
}

xy::Material& MeshRenderer::addMaterial(std::uint32_t id, Material::Description description, bool castShadows, bool useAlphaBlending)
{   
    auto shaderID = description + materialShaderOffset;
    if (!m_shaderResource.exists(shaderID))
    {
        //create the shader
        switch (description)
        {
        default: break;
        case Material::Coloured:
            m_shaderResource.preload(shaderID,
                DEFERRED_COLOURED_VERTEX, DEFERRED_COLOURED_FRAGMENT);
            break;
        case Material::ColouredBumped:
            m_shaderResource.preload(shaderID,
                DEFERRED_COLOURED_BUMPED_VERTEX, DEFERRED_COLOURED_BUMPED_FRAGMENT);
            break;
        case Material::ColouredSkinned:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define SKINNED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::ColouredSkinnedBumped:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define SKINNED\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define SKINNED\n#define BUMP\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::Textured:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::TexturedBumped:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::TexturedSkinned:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::TexturedSkinnedBumped:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define TEXTURED\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define TEXTURED\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::VertexColoured:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::VertexColouredBumped:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::VertexColouredSkinned:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define VERTEX_COLOUR\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define SKINNED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        case Material::VertexColouredSkinnedBumped:
            m_shaderResource.preload(shaderID,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex,
                "#version 150\n#define VERTEX_COLOUR\n#define BUMP\n#define SKINNED\n" + xy::Shader::Mesh::DeferredFragment);
            break;
        }
    }

    auto& material = m_materialResource.add(id, m_shaderResource.get(shaderID));
    material.addUniformBuffer(getMatrixUniforms());

    if (castShadows)
    {
        auto shadowID = shadowShaderOffset;
        switch (description)
        {
        default: break;
        case Material::ColouredSkinned:
        case Material::ColouredSkinnedBumped:
        case Material::TexturedSkinned:
        case Material::TexturedSkinnedBumped:
        case Material::VertexColouredSkinned:
        case Material::VertexColouredSkinnedBumped:
            shadowID++;          
            break;
        }

        if (!m_shaderResource.exists(shadowID))
        {
            shadowID == shadowShaderOffset ? m_shaderResource.preload(shadowID, SHADOW_VERTEX, SHADOW_FRAGMENT)
                : m_shaderResource.preload(shadowID, SHADOW_VERTEX_SKINNED, SHADOW_FRAGMENT);
        }
        material.addRenderPass(RenderPass::ID::ShadowMap, m_shaderResource.get(shadowID));
    }

    if (useAlphaBlending)
    {
        enableTransparency(material, description);
    }

    return material;
}

xy::Material& MeshRenderer::getMaterial(std::uint32_t id) const
{
    return m_materialResource.get(id);
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

void MeshRenderer::drawDepth(const sf::FloatRect& visibleArea) const
{
    //auto visibleArea = m_scene.getVisibleArea();
    std::size_t drawCount = 0;
    //get light count
    auto lightCount = m_scene.getVisibleLights(visibleArea).size();
    //TODO get visible area from light POV to cull geometry?
    REPORT("Active Light Count", std::to_string(lightCount));

    //foreach active light render scene to light depthmap   
    auto viewPort = m_depthTexture.getView().getViewport();
    auto size = m_depthTexture.getSize();

    m_depthTexture.setActive(true);
    glCheck(glViewport(
        static_cast<GLuint>(viewPort.left * size.x),
        static_cast<GLuint>(viewPort.top * size.y),
        static_cast<GLuint>(viewPort.width * size.x),
        static_cast<GLuint>(viewPort.height * size.y)));

    for (auto i = 0u; i < lightCount; ++i)
    {
        if (m_lightingBlock.u_pointLights[i].castShadow
            && m_lightingBlock.u_pointLights[i].intensity > 0.01f)
        {
            //set projection to current light
            std::memcpy(m_matrixBlock.u_lightViewProjectionMatrix, m_lightingBlock.u_pointLights[i].vpMatrix, sizeof(float) * 16);
            m_matrixBlockBuffer.update(m_matrixBlock);

            //set active texture layer
            m_depthTexture.setLayerActive(i);

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
    }
    

    //draw the skylight if it's enabled
    if (m_lightingBlock.u_skyLight.intensity > 0)
    {
        //set projection to current light
        std::memcpy(m_matrixBlock.u_lightViewProjectionMatrix, m_lightingBlock.u_skyLight.vpMatrix, sizeof(float) * 16);
        m_matrixBlockBuffer.update(m_matrixBlock);

        //set active texture layer
        m_depthTexture.setLayerActive(m_depthTexture.getLayerCount() - 1);

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

void MeshRenderer::drawScene(const sf::FloatRect& visibleArea) const
{
    m_gBuffer.setActive(true);
    auto viewPort = m_gBuffer.getView().getViewport();
    auto size = m_gBuffer.getSize();
    glCheck(glViewport(
    static_cast<GLuint>(viewPort.left * size.x),
    static_cast<GLuint>(viewPort.top * size.y),
    static_cast<GLuint>(viewPort.width * size.x),
    static_cast<GLuint>(viewPort.height * size.y)));

    glCheck(glClearColor(1.f, 1.f, 1.f, 0.f));
        
    glCheck(glEnable(GL_CULL_FACE));
    glCheck(glEnable(GL_DEPTH_TEST));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    std::size_t drawCount = 0;
    for (const auto& m : m_models)
    {
        drawCount += m->draw(m_viewMatrix, visibleArea, RenderPass::Default);
    }
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_CULL_FACE));
    m_gBuffer.display();
    
    REPORT("Draw Count", std::to_string(drawCount));
}

void MeshRenderer::drawAlphaBlended(const sf::FloatRect& visibleArea) const
{
    //ugh, SFMLs flipped Y coord...
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_flippedProjectionMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.update(m_matrixBlock);

    //glCheck(glEnable(GL_CULL_FACE));
    //glCheck(glDepthMask(GL_FALSE)); //to enable this we need to ensure depth sorting of faces
    for (const auto& m : m_models)
    {
        m->draw(m_viewMatrix, visibleArea, RenderPass::AlphaBlend);
    }
    //glCheck(glDepthMask(GL_TRUE));
    //glCheck(glDisable(GL_CULL_FACE));

    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.update(m_matrixBlock);
}

void MeshRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto oldView = rt.getView();
    rt.setView(m_view);
    
    auto visibleArea = m_scene.getVisibleArea();
    drawDepth(visibleArea);
    drawScene(visibleArea);

    /*m_ssaoTexture.clear(sf::Color::Transparent);
    m_ssaoTexture.draw(m_ssaoSprite, &m_ssaoShader);
    m_ssaoTexture.display();*/

    if (m_doLightBlur)
    {
        m_lightDownsampleTexture.clear();
        m_lightDownsampleTexture.draw(m_downSampleSprite, &m_lightDownsampleShader);
        m_lightDownsampleTexture.display();

        m_lightBlurShader.setUniform("u_sourceTexture", m_lightDownsampleTexture.getTexture());
        m_lightBlurShader.setUniform("u_offset", sf::Vector2f(0.f, 1.f / 270.f));
        m_lightBlurTexture.clear();
        m_lightBlurTexture.draw(m_lightBlurSprite, &m_lightBlurShader);
        m_lightBlurTexture.display();

        m_lightBlurShader.setUniform("u_sourceTexture", m_lightBlurTexture.getTexture());
        m_lightBlurShader.setUniform("u_offset", sf::Vector2f(1.f / 480.f, 0.f));
        m_lightDownsampleTexture.clear();
        m_lightDownsampleTexture.draw(m_lightBlurSprite, &m_lightBlurShader);
        m_lightDownsampleTexture.display();
    }

    //this is a kludge, as it seems the only way to activate
    //a render target is to draw something to it, which we need to
    //do in order to bind the lighting UBO to the correct context
    rt.draw(m_dummySprite);

    m_lightingBlockBuffer.bind(m_lightingShader.getNativeHandle(), m_lightingBlockID);
    //we can't directly access the shader's texture count so we have to make this
    //assumption to get the correct texture unit. BEAR THIS IN MIND
    glCheck(glActiveTexture(GL_TEXTURE0 + DepthTextureUnit));
    glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthTexture.getNativeHandle()));

    glCheck(glEnable(GL_DEPTH_TEST)); //MUST be enabled before writing quad
    rt.draw(*m_outputQuad);
    drawAlphaBlended(visibleArea); //forward rendering pass
    glCheck(glDisable(GL_DEPTH_TEST));

    rt.resetGLStates();
    rt.setView(oldView);
}

void MeshRenderer::updateView()
{
    auto viewSize = m_scene.getView().getSize();

    //calc how far away from the scene the camera would be
    //assuming the 2D world is drawn at 0 depth
    const float fov = m_fov * xy::Util::Const::degToRad;
    const float angle = std::tan(fov / 2.f);
    m_cameraZ = ((viewSize.y / 2.f) / angle);

    m_projectionMatrix = glm::perspective(fov, viewSize.x / viewSize.y, m_cameraZ * m_nearRatio, m_cameraZ * m_farRatio);
    m_flippedProjectionMatrix = glm::scale(m_projectionMatrix, { 1.f, -1.f, 1.f });
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16 * sizeof(float));

    m_lightProjectionMatrix = glm::perspective(180.f, xy::DefaultSceneSize.x / xy::DefaultSceneSize.y, 10.f, m_cameraZ);

    REPORT("FOV", std::to_string(m_fov));

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
        const auto& colour = lights[i]->getDiffuseColour();
        m_lightingBlock.u_pointLights[i].diffuseColour[0] = static_cast<float>(colour.r) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[1] = static_cast<float>(colour.g) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[2] = static_cast<float>(colour.b) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[3] = static_cast<float>(colour.a) / 255.f;

        const auto& spec = lights[i]->getSpecularColour();
        m_lightingBlock.u_pointLights[i].specularColour[0] = static_cast<float>(spec.r) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[1] = static_cast<float>(spec.g) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[2] = static_cast<float>(spec.b) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[3] = static_cast<float>(spec.a) / 255.f;

        m_lightingBlock.u_pointLights[i].intensity = lights[i]->getIntensity();
        m_lightingBlock.u_pointLights[i].inverseRange = lights[i]->getInverseRange();
        m_lightingBlock.u_pointLights[i].range = lights[i]->getRange();

        const auto& position = lights[i]->getWorldPosition();
        m_lightingBlock.u_pointLights[i].position[0] = position.x;
        m_lightingBlock.u_pointLights[i].position[1] = position.y;
        m_lightingBlock.u_pointLights[i].position[2] = position.z;

        //rather than use a shadow cube map we tak advantage of the fixed camera
        //and and use a single map per light with a 180 degree FOV
        //auto lightPerspectiveMatrix = glm::perspective(180.f, xy::DefaultSceneSize.x / xy::DefaultSceneSize.y, position.z / 3.f, m_cameraZ);
        auto lightViewMatrix = glm::lookAt(glm::vec3(position.x, position.y, position.z), glm::vec3(position.x, position.y, position.z - (lights[i]->getRange())), glm::vec3(0.f, 1.f,0.f));
        std::memcpy(m_lightingBlock.u_pointLights[i].vpMatrix, glm::value_ptr(m_lightProjectionMatrix * lightViewMatrix), 16 * sizeof(float));

        m_lightingBlock.u_pointLights[i].castShadow = lights[i]->castShadows();
    }

    //turn off others by setting intensity to 0
    for (; i < MAX_LIGHTS; ++i)
    {
        m_lightingBlock.u_pointLights[i].intensity = 0.f;
    }


    //set the scene skylight
    const auto& skylight = m_scene.getSkyLight();
    glm::vec3 target(camWorldPosition.x, camWorldPosition.y, 0.f);
    const auto& direction = skylight.getDirection();
    auto skyPerspective = glm::ortho(-960.f, 960.f, -540.f, 540.f, -DefaultSceneSize.x / 2.f, DefaultSceneSize.x);
    auto skyView = glm::lookAt(target - glm::vec3(direction.x, direction.y, direction.z), target, glm::vec3(0.f, 1.f,0.f));
    //auto skyPerspective = glm::perspective(90.f, xy::DefaultSceneSize.x / xy::DefaultSceneSize.y, 300.f, DefaultSceneSize.y);
    //auto skyView = glm::lookAt(target - (glm::normalize(glm::vec3(direction.x, direction.y, direction.z)) * (DefaultSceneSize.y / 2.f)), target, glm::vec3(0.f, 1.f, 0.f));
    std::memcpy(m_lightingBlock.u_skyLight.vpMatrix, glm::value_ptr(skyPerspective * skyView), sizeof(float) * 16);

    m_lightingBlock.u_skyLight.direction[0] = direction.x;
    m_lightingBlock.u_skyLight.direction[1] = direction.y;
    m_lightingBlock.u_skyLight.direction[2] = direction.z;
    m_lightingBlock.u_skyLight.intensity = skylight.getIntensity();
    m_waterShader.setUniform("u_lightDirection", direction);

    const auto& diffuse = skylight.getDiffuseColour();
    m_lightingBlock.u_skyLight.diffuseColour[0] = static_cast<float>(diffuse.r) / 255.f;
    m_lightingBlock.u_skyLight.diffuseColour[1] = static_cast<float>(diffuse.g) / 255.f;
    m_lightingBlock.u_skyLight.diffuseColour[2] = static_cast<float>(diffuse.b) / 255.f;
    m_lightingBlock.u_skyLight.diffuseColour[3] = static_cast<float>(diffuse.a) / 255.f;
    m_waterShader.setUniform("u_lightDiffuse", sf::Glsl::Vec4(
        m_lightingBlock.u_skyLight.diffuseColour[0] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.diffuseColour[1] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.diffuseColour[2] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.diffuseColour[3] * m_lightingBlock.u_skyLight.intensity
    )); 

    const auto& specular = skylight.getSpecularColour();
    m_lightingBlock.u_skyLight.specularColour[0] = static_cast<float>(specular.r) / 255.f;
    m_lightingBlock.u_skyLight.specularColour[1] = static_cast<float>(specular.g) / 255.f;
    m_lightingBlock.u_skyLight.specularColour[2] = static_cast<float>(specular.b) / 255.f;
    m_lightingBlock.u_skyLight.specularColour[3] = static_cast<float>(specular.a) / 255.f;
    m_waterShader.setUniform("u_lightSpecular", sf::Glsl::Vec4(
        m_lightingBlock.u_skyLight.specularColour[0] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.specularColour[1] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.specularColour[2] * m_lightingBlock.u_skyLight.intensity,
        m_lightingBlock.u_skyLight.specularColour[3] * m_lightingBlock.u_skyLight.intensity
    ));

    m_lightingShader.setUniform("u_lightCount", int(std::min(std::size_t(MAX_LIGHTS), lights.size())));
    m_lightingBlockBuffer.update(m_lightingBlock);
}

void MeshRenderer::resizeGBuffer(sf::Uint32 x, sf::Uint32 y)
{
    //create the render buffer
    m_gBuffer.create(x, y, 4u, true, true);

    //use floating point textures for position and normals
    std::function<void(int)> useFloatingpoint = [x, y, this](int id)
    {
        glCheck(glBindTexture(GL_TEXTURE_2D, m_gBuffer.getTexture(id).getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, x, y, 0, GL_RGBA, GL_FLOAT, 0));
        glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    };
    useFloatingpoint(MaterialChannel::Normal);
    useFloatingpoint(MaterialChannel::Position);
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
    m_lightBlurShader.loadFromMemory(Shader::PostGaussianBlur::fragment, sf::Shader::Fragment);
    m_lightBlurShader.setUniform("u_sourceTexture", m_lightDownsampleTexture.getTexture());
    

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
            m_lightingShader.setUniform("u_depthBuffer", m_gBuffer.getDepthTexture());
            //m_lightingShader.setUniform("u_aoMap", m_ssaoTexture.getTexture());
            m_lightingShader.setUniform("u_illuminationMap", m_lightDownsampleTexture.getTexture());
            m_lightingShader.setUniform("u_reflectMap", m_scene.getReflection());

            //we can't directly access the shader's texture count so we have to make this
            //assumption based on the above to get the correct texture unit. BEAR THIS IN MIND
            sf::Shader::bind(&m_lightingShader);
            auto loc = glGetUniformLocation(m_lightingShader.getNativeHandle(), "u_depthMaps");
            glCheck(glUniform1i(loc, DepthTextureUnit)); //depth maps for lights
        }
    }
    else
    {
        xy::Logger::log("Failed to create output shader for deferred renderer", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    //allow debugging each channel of MRT
    if (m_debugShader.loadFromMemory(xy::Shader::Mesh::LightingVert, xy::Shader::Mesh::DebugFrag))
    {
        m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Diffuse));
    }
    else
    {
        xy::Logger::log("Failed to create output shader for mesh debug", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    //debugging depth output
    if (m_depthShader.loadFromMemory(xy::Shader::Mesh::LightingVert, xy::Shader::Mesh::DepthFrag))
    {
        sf::Shader::bind(&m_depthShader);
        auto loc = glGetUniformLocation(m_depthShader.getNativeHandle(), "u_texture");
        glCheck(glUniform1i(loc, 0));
    }
    else
    {
        xy::Logger::log("Failed to create output shader for depth debug", xy::Logger::Type::Error, xy::Logger::Output::All);
    }

    m_outputQuad = std::make_unique<RenderQuad>(sf::Vector2f(200.f, 100.f), m_lightingShader);
    m_outputQuad->addRenderPass(RenderPass::Debug, m_debugShader);
    m_outputQuad->addRenderPass(RenderPass::ShadowMap, m_depthShader);


    //depth texture for shadow maps
    m_depthTexture.create(shadowmapSize, shadowmapSize, MAX_LIGHTS + 1); //extra for skylight
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

    //displays the texture used by self illumination
    Console::addCommand("r_showGlowMap", 
        [this](const std::string& params)
    {
        if (params.find_first_of('0') == 0 ||
            params.find_first_of("false") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Default);
            Console::print("Switched output to default");
        }

        else if (params.find_first_of('1') == 0 ||
            params.find_first_of("true") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_lightDownsampleTexture.getTexture());
            Console::print("Switched output to glow map");
        }

        else
        {
            Console::print("r_showGlowMap: valid parameters are 0, 1, false or true");
        }
    });

    Console::addCommand("r_showReflectMap",
        [this](const std::string& params)
    {
        if (params.find_first_of('0') == 0 ||
            params.find_first_of("false") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Default);
            Console::print("Switched output to default");
        }

        else if (params.find_first_of('1') == 0 ||
            params.find_first_of("true") == 0)
        {
            m_outputQuad->setActivePass(RenderPass::Debug);
            m_debugShader.setUniform("u_texture", m_scene.getReflection());
            Console::print("Switched output to reflection map");
        }

        else
        {
            Console::print("r_showReflectMap: valid parameters are 0, 1, false or true");
        }
    });

    //shows the selected depth map
    Console::addCommand("r_showDepthMap", 
        [this](const std::string& params)
    {
        if (!params.empty())
        {
            try
            {
                float index = std::stof(params);
                if (index < m_depthTexture.getLayerCount())
                {
                    m_depthShader.setUniform("u_texIndex", index);
                }
                else
                {
                    Console::print("Only " + std::to_string(m_depthTexture.getLayerCount()) + " depth maps exist");
                    return;
                }
            }
            catch (...)
            {
                Console::print(params + ": invalid depth map index");
                return;
            }
            m_outputQuad->setActivePass(RenderPass::ShadowMap);

            glCheck(glActiveTexture(GL_TEXTURE0)); //we know this shader only has one texture sampler
            glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthTexture.getNativeHandle()));

            Console::print("Switched output to depth map");
        }
        else
        {
            Console::print("Usage: r_showDepthMap <index>");
            
            //switch to default display
            m_outputQuad->setActivePass(RenderPass::Default);
            Console::print("Switched output to default");
        }
    }, this);

    //updates the FOV
    Console::addCommand("r_fov", 
        [this](const std::string& params)
    {
        try
        {
            float fov = std::stof(params.c_str());
            fov = std::min(180.f, std::max(10.f, fov));
            setFOV(fov);
            Console::print("Field of View set to " + std::to_string(fov) + " degrees");
        }
        catch (...)
        {
            Console::print(params + " invalid value. Range is 10 to 180 degrees");
        }
    });
}

void MeshRenderer::addDebugMenus()
{
    std::function<void()> debugWindow = [this]()
    {
        nim::SetNextWindowSize({ 240.f, 368.f });
        nim::Begin("Switch Output");
        static int selected = 0;
        static int lastSelected = selected;
        nim::RadioButton("Default", &selected, 0);
        nim::RadioButton("Diffuse", &selected, 1);
        nim::RadioButton("World Normals", &selected, 2);
        nim::RadioButton("Mask", &selected, 3);
        nim::RadioButton("Position", &selected, 4);
        nim::RadioButton("Illumination", &selected, 5);
        nim::RadioButton("Reflection Map", &selected, 6);
        nim::RadioButton("Depth Map", &selected, 7);
        static int mapIndex = 0;
        static int lastMapIndex = mapIndex;
        nim::SliderInt("Depth", &mapIndex, 0, 8);
        static auto lastValue = m_fov;
        nim::SliderFloat("FOV", &m_fov, 10.f, 180.f);
        if (lastValue != m_fov) updateView();
        lastValue = m_nearRatio;
        nim::SliderFloat("NP Ratio", &m_nearRatio, 0.1f, 0.9f);
        if (lastValue != m_nearRatio) updateView();
        lastValue = m_farRatio;
        nim::SliderFloat("FP Ratio", &m_farRatio, 1.1f, 99.9f);
        if (lastValue != m_farRatio) updateView();
        static float lightInfluence = 1.f;
        float influence = lightInfluence;
        nim::SliderFloat("Light Mix", &lightInfluence, 0.f, 1.f);
        if (influence != lightInfluence) m_waterShader.setUniform("u_lightInfluence", lightInfluence);

        static bool overrideLevel = false;
        nim::Checkbox("Set Water Level", &overrideLevel);
        if (overrideLevel)
        {
            static float waterLevel = 0.1f;
            nim::SliderFloat("Level", &waterLevel, 0.1f, 800.f);
            m_waterShader.setUniform("u_waterLevel", waterLevel);
        }
        nim::End();

        if (selected != lastSelected)
        {
            switch (selected)
            {
            default: break;
            case 0:
                m_outputQuad->setActivePass(RenderPass::Default);
                break;
            case 1:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Diffuse));
                break;
            case 2:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Normal));
                break;
            case 3:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Mask));
                break;
            case 4:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_gBuffer.getTexture(MaterialChannel::Position));
                break;
            case 5:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_lightDownsampleTexture.getTexture());
                break;
            case 6:
                m_outputQuad->setActivePass(RenderPass::Debug);
                m_debugShader.setUniform("u_texture", m_scene.getReflection());
                break;
            case 7:
                m_outputQuad->setActivePass(RenderPass::ShadowMap);
                m_depthShader.setUniform("u_texIndex", static_cast<float>(mapIndex));
                glCheck(glActiveTexture(GL_TEXTURE0)); //we know this shader only has one texture sampler
                glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, m_depthTexture.getNativeHandle()));
                break;
            }
        }
        lastSelected = selected;

        if (mapIndex != lastMapIndex)
        {
            m_depthShader.setUniform("u_texIndex", static_cast<float>(mapIndex));
        }
        lastMapIndex = mapIndex;       
    };
    App::addUserWindow(debugWindow, this);
}