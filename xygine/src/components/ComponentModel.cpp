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

#include <xygine/components/Model.hpp>
#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Const.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace xy;

Model::Model(MessageBus& mb, const Mesh& mesh, const MeshRenderer::Lock&)
    : Component     (mb, this),
    m_mesh          (mesh),
    m_depth         (0.f),
    m_material      (nullptr)
{
    m_subMaterials.resize(mesh.getSubMeshCount());
    std::fill(m_subMaterials.begin(), m_subMaterials.end(), nullptr);
}

void Model::entityUpdate(Entity& entity, float dt)
{
    const auto position = entity.getWorldPosition();
    m_worldMatrix = glm::translate(glm::mat4(), glm::vec3(position.x, position.y, m_depth));
    
    const float rotation = xy::Util::Const::degToRad * entity.getRotation();
    m_worldMatrix = glm::rotate(m_worldMatrix, rotation, glm::vec3(0.f, 0.f, 1.f));

    const auto scale = entity.getScale();
    m_worldMatrix = glm::scale(m_worldMatrix, glm::vec3(scale.x, scale.y, (scale.x + scale.y) / 2.f));

    m_worldMatrix *= m_preTransform;
}

void Model::setBaseMaterial(const Material& material, bool applyToAll)
{
    auto oldMat = m_material;
    m_material = &material;
    if (applyToAll)
    {
        for (auto i = 0u; i < m_mesh.getSubMeshCount(); ++i)
        {
            setSubMaterial(material, i);
        }
    }
    updateVertexAttribs(oldMat, m_material);
}

void Model::setSubMaterial(const Material& material, std::size_t idx)
{
    if (idx < m_mesh.getSubMeshCount())
    {
        auto oldMat = m_subMaterials[idx];
        m_subMaterials[idx] = &material;
        updateVertexAttribs(oldMat, &material);
    }
}

void Model::preTransform(Model::Axis axis, float rotation)
{
    switch(axis)
    {
        default: break;
    case Axis::X:
        m_rotation.x = xy::Util::Const::degToRad * rotation;
        break;
    case Axis::Y:
        m_rotation.y = xy::Util::Const::degToRad * rotation;
        break;
    case Axis::Z:
        m_rotation.z = xy::Util::Const::degToRad * rotation;
        break;
    }

    m_preTransform = glm::translate(glm::mat4(), { 50, 160.f, 0.f });

    m_preTransform = glm::rotate(m_preTransform, m_rotation.y, glm::vec3(1.f, 0.f, 0.f));
    m_preTransform = glm::rotate(m_preTransform, m_rotation.z, glm::vec3(0.f, 1.f, 0.f));
    m_preTransform = glm::rotate(m_preTransform, m_rotation.x, glm::vec3(0.f, 0.f, 1.f));

    m_preTransform = glm::scale(m_preTransform, { 50.f, 50.f, 50.f });
}

//private
void Model::draw(const glm::mat4& viewMatrix) const
{
    glm::mat4 worldViewMat = viewMatrix * m_worldMatrix;

    if (m_mesh.getSubMeshCount() > 0)
    {
        auto count = m_mesh.getSubMeshCount();
        for (auto i = 0u; i < count; ++i)
        {
            m_subMaterials[i]->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
            m_subMaterials[i]->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
            m_subMaterials[i]->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(worldViewMat)))));
            m_subMaterials[i]->bind();
            auto vao = m_vaoBindings.find(m_subMaterials[i]);
            vao->second.bind();
            const auto subMesh = m_mesh.getSubMesh(i);
            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh->getIndexBufferID()));
            glCheck(glDrawElements(static_cast<GLenum>(subMesh->getPrimitiveType()), subMesh->getIndexCount(), static_cast<GLenum>(subMesh->getIndexFormat()), 0));
            vao->second.unbind();
        }
    }
    else
    {
        m_material->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
        m_material->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
        m_material->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(worldViewMat)))));
        m_material->bind();
        auto vao = m_vaoBindings.find(m_material);
        vao->second.bind();
        glCheck(glDrawArrays(static_cast<GLenum>(m_mesh.getPrimitiveType()), 0, m_mesh.getVertexCount()));
        vao->second.unbind();
    }
}

void Model::updateVertexAttribs(const Material* oldMat, const Material* newMat)
{
    XY_ASSERT(newMat, "New material cannot be null");
    if (oldMat)
    {
        //if not found in submeshes or used as main mat
        if (oldMat != m_material &&
            std::find(m_subMaterials.begin(), m_subMaterials.end(), oldMat) == m_subMaterials.end())
        {
            //remove from VAOs
            m_vaoBindings.erase(oldMat);
        }
    }

    //if we don't yet have a VAO for this material
    //create one
    if (m_vaoBindings.count(newMat) == 0)
    {
        m_vaoBindings.insert(std::make_pair(newMat, VertexAttribBinding(m_mesh, *newMat)));
    }
}