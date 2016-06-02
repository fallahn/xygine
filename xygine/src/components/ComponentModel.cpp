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
#include <glm/gtx/quaternion.hpp>

using namespace xy;

Model::Model(MessageBus& mb, const Mesh& mesh, const MeshRenderer::Lock&)
    : Component         (mb, this),
    m_scale             (1.f, 1.f, 1.f),
    m_needsUpdate       (false),
    m_mesh              (mesh),
    m_material          (nullptr),
    m_skeleton          (nullptr),
    m_currentAnimation  (0)
{
    m_subMaterials.resize(mesh.getSubMeshCount());
    std::fill(m_subMaterials.begin(), m_subMaterials.end(), nullptr);

    m_boundingBox = mesh.getBoundingBox();
}

void Model::entityUpdate(Entity& entity, float dt)
{
    const auto position = entity.getWorldPosition();
    m_worldMatrix = glm::translate(glm::mat4(), glm::vec3(position.x, position.y, 0.f));
    
    const float rotation = xy::Util::Const::degToRad * entity.getRotation();
    m_worldMatrix = glm::rotate(m_worldMatrix, rotation, glm::vec3(0.f, 0.f, 1.f));

    const auto scale = entity.getScale();
    m_worldMatrix = glm::scale(m_worldMatrix, glm::vec3(scale.x, scale.y, (scale.x + scale.y) / 2.f));

    if (m_needsUpdate) updateTransform();
    m_worldMatrix *= m_preTransform;

    m_worldBounds = entity.getWorldTransform().transformRect(m_boundingBox.asFloatRect());
}

void Model::setBaseMaterial(const Material& material, bool applyToAll)
{
    //TODO iterate over all passes in materials
    
    auto oldShader = -1;
    if(m_material) oldShader = m_material->getShader().getNativeHandle();

    m_material = &material;
    if (applyToAll)
    {
        for (auto i = 0u; i < m_mesh.getSubMeshCount(); ++i)
        {
            setSubMaterial(material, i);
        }
    }
    updateVertexAttribs(oldShader, m_material->getShader().getNativeHandle(), material);
}

void Model::setSubMaterial(const Material& material, std::size_t idx)
{
    //TODO make sure to iterate over all passes of old and new materials
    if (idx < m_mesh.getSubMeshCount())
    {
        auto oldShader = -1;
        if(m_subMaterials[idx]) oldShader = m_subMaterials[idx]->getShader().getNativeHandle();
        m_subMaterials[idx] = &material;
        updateVertexAttribs(oldShader, material.getShader().getNativeHandle(), material);
    }
}

void Model::rotate(Model::Axis axis, float rotation)
{
    glm::vec3 normal;
    switch(axis)
    {
        default: break;
    case Axis::X:
        normal.x = 1.f;
        break;
    case Axis::Y:
        normal.y = 1.f;
        break;
    case Axis::Z:
        normal.z = 1.f;
        break;
    }
    m_rotation = glm::rotate(m_rotation, xy::Util::Const::degToRad * rotation, normal);
    m_needsUpdate = true;
}

void Model::setPosition(const sf::Vector3f& position)
{
    m_translation = { position.x, position.y, position.z };
    m_needsUpdate = true;
}

void Model::setScale(const sf::Vector3f& scale)
{
    m_scale = { scale.x, scale.y, scale.y };
}

void Model::setSkeleton(const Skeleton& s)
{
    m_skeleton = &s;
    for (auto& a : m_animations)
    {
        a.setSkeleton(m_skeleton);
    }

    m_currentFrame = m_skeleton->getFrame(0);
}

void Model::addAnimation(const Skeleton::Animation& animation)
{
    m_animations.push_back(animation);
    if (m_skeleton) m_animations.back().setSkeleton(m_skeleton);
}

void Model::setAnimations(const std::vector<Skeleton::Animation>& animations)
{
    m_animations = animations;
    if (m_skeleton)
    {
        for (auto& a : m_animations)
        {
            a.setSkeleton(m_skeleton);
        }
    }
}

//private
void Model::updateTransform()
{
    m_preTransform = glm::mat4();
    m_preTransform = glm::translate(m_preTransform, m_translation);
    m_preTransform *= glm::toMat4(m_rotation);
    m_preTransform = glm::scale(m_preTransform, m_scale);

    m_boundingBox = m_mesh.getBoundingBox();
    m_boundingBox.transform(m_preTransform);

    m_needsUpdate = false;
}

std::size_t Model::draw(const glm::mat4& viewMatrix, const sf::FloatRect& visibleArea, RenderPass::ID pass) const
{
    if (!m_worldBounds.intersects(visibleArea)) return 0;
    
    glm::mat4 worldViewMat = viewMatrix * m_worldMatrix;

    if (m_mesh.getSubMeshCount() > 0)
    {
        auto count = m_mesh.getSubMeshCount();
        for (auto i = 0u; i < count; ++i)
        {
            if (!m_subMaterials[i]->setActivePass(pass)) continue;
            m_subMaterials[i]->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
            m_subMaterials[i]->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
            m_subMaterials[i]->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(worldViewMat)))));
            m_subMaterials[i]->bind();
            auto vao = m_vaoBindings.find(m_subMaterials[i]->getShader().getNativeHandle());
            vao->second.bind();
            const auto subMesh = m_mesh.getSubMesh(i);
            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh->getIndexBufferID()));
            glCheck(glDrawElements(static_cast<GLenum>(subMesh->getPrimitiveType()), subMesh->getIndexCount(), static_cast<GLenum>(subMesh->getIndexFormat()), 0));
            vao->second.unbind();
        }
        return count;
    }
    else
    {
        if(!m_material->setActivePass(pass)) return 0;
        m_material->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
        m_material->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
        m_material->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(worldViewMat)))));
        m_material->bind();
        auto vao = m_vaoBindings.find(m_material->getShader().getNativeHandle());
        vao->second.bind();
        glCheck(glDrawArrays(static_cast<GLenum>(m_mesh.getPrimitiveType()), 0, m_mesh.getVertexCount()));
        vao->second.unbind();
    }
    return 1;
}

void Model::updateVertexAttribs(ShaderID oldShader, ShaderID newShader, const Material& newMat)
{
    XY_ASSERT(newShader > -1, "New material cannot be null");
 
    //count instances of old shader used, and remove binding if there are none
    auto count = 0;
    if (m_material->getShader().getNativeHandle() == oldShader) count++;
    for (auto& m : m_subMaterials)
    {
        if (m->getShader().getNativeHandle() == oldShader)
        {
            count++;
        }
    }
    if (count == 0) m_vaoBindings.erase(oldShader);

    //if we don't yet have a VAO for this material
    //create one
    if (m_vaoBindings.count(newShader) == 0)
    {
        m_vaoBindings.insert(std::make_pair(newShader, VertexAttribBinding(m_mesh, newMat)));
    }
}