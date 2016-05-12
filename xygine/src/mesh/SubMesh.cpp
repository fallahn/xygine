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

#include <xygine/mesh/SubMesh.hpp>

#include <xygine/detail/GLCheck.hpp>
#include <xygine/detail/GLExtensions.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

SubMesh::SubMesh(Mesh& mesh, std::size_t meshIndex, Mesh::PrimitiveType primitiveType, Mesh::IndexFormat indexFormat, std::size_t count, bool dynamic, const Mesh::Lock&)
    : m_mesh        (&mesh),
    m_meshIndex     (meshIndex),
    m_primitiveType (primitiveType),
    m_indexFormat   (indexFormat),
    m_count         (count),
    m_iboID         (0),
    m_dynamic       (dynamic)
{
    glCheck(glGenBuffers(1, &m_iboID));
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboID));

    //calc the stride
    std::size_t indexSize = 0;
    switch (m_indexFormat)
    {
    default:
    case Mesh::IndexFormat::I8:
        indexSize = 1;
        break;
    case Mesh::IndexFormat::I16:
        indexSize = 2;
        break;
    case Mesh::IndexFormat::I32:
        indexSize = 4;
        break;
    }

    //fill buffer with empty data
    glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * count, nullptr, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
}

SubMesh::~SubMesh()
{
    if (m_iboID != 0)
    {
        glCheck(glDeleteBuffers(1, &m_iboID));
    }
}

//public
std::size_t SubMesh::getMeshIndex() const
{
    return m_meshIndex;
}

Mesh::PrimitiveType SubMesh::getPrimitiveType() const
{
    return m_primitiveType;
}

std::size_t SubMesh::getIndexCount() const
{
    return m_count;
}

Mesh::IndexFormat SubMesh::getIndexFormat() const
{
    return m_indexFormat;
}

IndexBufferID SubMesh::getIndexBufferID() const
{
    return m_iboID;
}

bool SubMesh::dynamic() const 
{
    return m_dynamic;
}

void SubMesh::setIndexData(const void* data, std::size_t count, std::size_t start)
{
    glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iboID));

    //calc stride from index format
    std::size_t indexSize = 0;
    switch (m_indexFormat)
    {
    default:
    case Mesh::IndexFormat::I8:
        indexSize = 1;
        break;
    case Mesh::IndexFormat::I16:
        indexSize = 2;
        break;
    case Mesh::IndexFormat::I32:
        indexSize = 4;
        break;
    }

    if (count == 0 && start == 0)
    {
        //update entire buffer
        glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * m_count, data, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
    }
    else
    {
        //update sub-buffer
        if (count == 0)
        {
            count = m_count - start;
        }
        glCheck(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, start * indexSize, count * indexSize, data));
    }
}