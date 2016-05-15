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

#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/mesh/VertexLayout.hpp>

#include <xygine/detail/GLCheck.hpp>
#include <xygine/detail/GLExtensions.hpp>

#include <xygine/Assert.hpp>

using namespace xy;

namespace
{
    
}

Mesh::Mesh(const VertexLayout& vertLayout, std::size_t count, bool dynamic)
    : m_vertexLayout    (vertLayout),
    m_vertexCount       (count),
    m_vboID             (0),
    m_primitiveType     (PrimitiveType::Triangles),
    m_dynamic           (dynamic)
{
    glCheck(glGenBuffers(1, &m_vboID));
}

Mesh::~Mesh()
{
    if (m_vboID)
    {
        glCheck(glDeleteBuffers(1, &m_vboID));
    }
}

//public
const VertexLayout& Mesh::getVertexLayout() const
{
    return m_vertexLayout;
}

std::size_t Mesh::getVertexCount() const
{
    return m_vertexCount;
}

std::size_t Mesh::getVertexSize() const
{
    return m_vertexLayout.getVertexSize();
}

VertexBufferID Mesh::getBufferID() const
{
    return m_vboID;
}

bool Mesh::dynamic() const
{
    return m_dynamic;
}

Mesh::PrimitiveType Mesh::getPrimitiveType() const
{
    return m_primitiveType;
}

void Mesh::setPrimitiveType(Mesh::PrimitiveType type)
{
    m_primitiveType = type;
}

void Mesh::setVertexData(const float* data, std::size_t count, std::size_t offset)
{
    XY_ASSERT(data, "VBO data is null");

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vboID));
    if (count == 0 && offset == 0)
    {
        //update whole buffer
        glCheck(glBufferData(GL_ARRAY_BUFFER, m_vertexLayout.getVertexSize() * m_vertexCount, data, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    }
    else
    {
        //update sub buffer
        if (count == 0)
        {
            count = m_vertexCount - offset;
        }
        glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset * m_vertexLayout.getVertexSize(), count * m_vertexLayout.getVertexSize(), data));
    }
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

SubMesh& Mesh::addSubMesh(Mesh::PrimitiveType type, Mesh::IndexFormat format, std::size_t count, bool dynamic)
{
    m_subMeshes.emplace_back(std::make_unique<SubMesh>(*this, m_subMeshes.size(), type, format, count, dynamic, Lock()));
    return *m_subMeshes.back();
}

std::size_t Mesh::getSubMeshCount() const
{
    return m_subMeshes.size();
}

SubMesh* Mesh::getSubMesh(std::size_t idx) const
{
    return (idx < m_subMeshes.size()) ? m_subMeshes[idx].get() : nullptr;
}

//private