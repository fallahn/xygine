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

#include <xygine/mesh/CubeBuilder.hpp>

using namespace xy;

CubeBuilder::CubeBuilder(float size, bool normals, bool uvs)
    : m_size    (size),
    m_useNormals(normals),
    m_useUVs    (uvs)
{
    m_frontIndices = { 0, 1, 2, 3 };
    m_backIndices = { 4, 5, 6, 7 };
    m_leftIndices = { 8, 9, 10, 11 };
    m_rightIndices = { 12, 13, 14, 15 };
    m_topIndices = { 16, 17, 18, 19 };
    m_bottomIndices = { 20, 21, 22, 23 };
}

//public
void CubeBuilder::build()
{    
    if (m_vertexData.empty())
    {
        const float side = m_size / 2.f;
        std::vector<float> positions =
        {
            //front
            -side, -side, side,
            -side, side, side,
            side, -side, side,
            side,side, side,
            //back
            side, -side, -side,
            side, side, -side,
            -side, -side, -side,
            -side, side, -side,
            //left
            -side, -side, -side,
            -side, side, -side,
            -side, -side, side,
            -side, side, side,
            //right
            side, -side, side,
            side, side, side,
            side, -side, -side,
            side, side, -side,
            //top
            -side, side, side,
            -side, side, -side,
            side, side, side,
            side, side, -side,
            //bottom
            side, -side, side,
            side, -side, -side,
            -side, -side, side,
            -side, -side, -side
        };
        std::vector<float> normals;
        std::vector<float> UVs;
        
        //set up vert layout
        m_elements.emplace_back(VertexLayout::Element::Type::Position, 3);
        if (m_useNormals)
        {
            m_elements.emplace_back(VertexLayout::Element::Type::Normal, 3);
            normals = 
            {
                //front
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                0.f, 0.f, 1.f,
                //back
                0.f, 0.f, -1.f,
                0.f, 0.f, -1.f,
                0.f, 0.f, -1.f,
                0.f, 0.f, -1.f,
                //left
                -1.f, 0.f, 0.f,
                -1.f, 0.f, 0.f,
                -1.f, 0.f, 0.f,
                -1.f, 0.f, 0.f,
                //right
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                1.f, 0.f, 0.f,
                //top
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                0.f, 1.f, 0.f,
                //bottom
                0.f, -1.f, 0.f,
                0.f, -1.f, 0.f,
                0.f, -1.f, 0.f,
                0.f, -1.f, 0.f
            };
        }

        if (m_useUVs)
        {
            m_elements.emplace_back(VertexLayout::Element::Type::UV0, 2);
            UVs = 
            {
                //front
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f,
                //back
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f,
                //left
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f,
                //right
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f,
                //top
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f,
                //bottom
                0.f, 1.f,
                0.f, 0.f,
                1.f, 1.f,
                1.f, 0.f
            };
        }

        //create vert data
        std::size_t i, j, k;
        i = j = k = 0;
        for (auto l = 0u; l < getVertexCount(); ++l)
        {
            m_vertexData.push_back(positions[i++]);
            m_vertexData.push_back(positions[i++]);
            m_vertexData.push_back(positions[i++]);

            if (m_useNormals)
            {
                m_vertexData.push_back(normals[j++]);
                m_vertexData.push_back(normals[j++]);
                m_vertexData.push_back(normals[j++]);
            }

            if (m_useUVs)
            {
                m_vertexData.push_back(UVs[k++]);
                m_vertexData.push_back(UVs[k++]);
            }
        }
    }
}

VertexLayout CubeBuilder::getVertexLayout() const
{
    return VertexLayout(m_elements);
}

std::vector<MeshBuilder::SubMeshLayout> CubeBuilder::getSubMeshLayouts() const
{
    std::vector<MeshBuilder::SubMeshLayout> retval(6);
    retval[0].data = (void*)m_frontIndices.data();
    retval[0].size = 4;
    retval[0].type = Mesh::PrimitiveType::TriangleStrip;

    retval[1].data = (void*)m_backIndices.data();
    retval[1].size = 4;
    retval[1].type = Mesh::PrimitiveType::TriangleStrip;

    retval[2].data = (void*)m_leftIndices.data();
    retval[2].size = 4;
    retval[2].type = Mesh::PrimitiveType::TriangleStrip;

    retval[3].data = (void*)m_rightIndices.data();
    retval[3].size = 4;
    retval[3].type = Mesh::PrimitiveType::TriangleStrip;

    retval[4].data = (void*)m_topIndices.data();
    retval[4].size = 4;
    retval[4].type = Mesh::PrimitiveType::TriangleStrip;

    retval[5].data = (void*)m_bottomIndices.data();
    retval[5].size = 4;
    retval[5].type = Mesh::PrimitiveType::TriangleStrip;

    return std::move(retval);
}