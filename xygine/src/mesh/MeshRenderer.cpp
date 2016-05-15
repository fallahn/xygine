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
#include <xygine/mesh/StaticConsts.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Const.hpp>

#include <SFML/Graphics/RenderStates.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>

using namespace xy;

namespace
{
    const float fov = 40.f;
    const float nearPlane = 1.f;
    const float farPlane = 40.f;
}

MeshRenderer::MeshRenderer(const sf::Vector2u& size, const Scene& scene)
    : m_scene               (scene),
    m_matrixBlockBuffer     ("u_matrixBlock"),
    m_lightingBlockBuffer   ("u_lightingBlock")
{
    //set up a default material to assign to newly created models
    m_defaultShader.loadFromMemory(Shader3D::DefaultVertex, Shader3D::DefaultFragment);
    m_defaultMaterial = std::make_unique<Material>(m_defaultShader);
    
    //create the render buffer
    m_renderTexture.create(size.x, size.y, true);
    m_sprite.setTexture(m_renderTexture.getTexture());

    updateView();

    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16);
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16);
    m_matrixBlockBuffer.create(m_matrixBlock);

    m_defaultMaterial->addUniformBuffer(m_matrixBlockBuffer);
}

//public
std::unique_ptr<Model> MeshRenderer::createModel(MessageBus& mb, const Mesh& mesh)
{
    auto model = Component::create<Model>(mb, mesh, Lock());
    m_models.push_back(model.get());
    //set model uniform block for projection mat
    //TODO and lighting
    
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

    auto camPos = m_scene.getView().getCenter();
    glm::mat4 m_viewMatrix = glm::inverse(glm::translate(glm::mat4(), glm::vec3(camPos.x, camPos.y, m_cameraZ)));
    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16);
    m_matrixBlockBuffer.update(m_matrixBlock);
}

void MeshRenderer::handleMessage(const Message& msg)
{
    //TODO handle camera changed event
}

//private
void MeshRenderer::drawScene() const
{    
    //TODO update UBO with scene lighting / cam pos and projection mat
    

    m_renderTexture.setActive(true);
    m_renderTexture.clear(sf::Color::Transparent);

    glViewport(0, 0, 1920, 1080);
    //glClearColor(1.f, 1.f, 1.f, 0.5f);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (const auto& m : m_models)m->draw(m_viewMatrix);

    m_renderTexture.display();
    //m_renderTexture.setActive(false);
}

void MeshRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    drawScene();
    rt.draw(m_sprite, states);
}

void MeshRenderer::updateView()
{
    auto viewSize = m_scene.getView().getSize();
    m_projectionMatrix = glm::perspective(fov, viewSize.x / viewSize.y, nearPlane, farPlane);
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16);

    //calc how far away from the scene the camera would be
    //assuming the 2D world is drawn at 0 depth
    const float angle = std::tan(fov / 2.f * 0.0174532925f);
    m_cameraZ = (static_cast<float>(viewSize.y) / 2.f) / angle;

    XY_WARNING(std::abs(m_cameraZ) > farPlane, "Camera depth greater than far plane");

    //m_cameraZ *= -m_sceneScale; //scales units - would also need to scale X/Y
}