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
#include <xygine/detail/GLCheck.hpp>
#include <xygine/Entity.hpp>

#include <glm/gtc/matrix_transform.hpp>

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

//private
void Model::draw(const glm::mat4& viewMatrix) const
{
    /*
    if submesh count > 0
        foreach submesh
            submeshMat[i].bind();
            vaos[submeshMat[i]].bind()
            draw elements
    */
    if (m_mesh.getSubMeshCount() > 0)
    {

    }
    else
    {
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