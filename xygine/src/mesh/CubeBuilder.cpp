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

CubeBuilder::CubeBuilder(float size)
    : m_size    (size)
{
    m_indices =
    {
        0u, 1u, 2u, 0u, 2u, 3u,
        4u, 5u, 6u, 4u, 6u, 7u,
        8u, 9u, 10u, 8u, 10u, 11u,
        12u, 13u, 14u, 12u, 14u, 15u,
        16u, 17u, 18u, 16u, 18u, 19u,
        20u, 21u, 22u, 20u, 22u, 23u
    };

    SubMeshLayout sml;
    sml.data = (void*)m_indices.data();
    sml.size = m_indices.size();
    sml.type = Mesh::PrimitiveType::Triangles;
    addSubMeshLayout(sml);
}

//public
void CubeBuilder::build()
{    
    if (m_vertexData.empty())
    {
        m_elements =
        {
            VertexLayout::Element(VertexLayout::Element::Type::Position, 3u),
            VertexLayout::Element(VertexLayout::Element::Type::Normal, 3u),
            VertexLayout::Element(VertexLayout::Element::Type::Tangent, 3u),
            VertexLayout::Element(VertexLayout::Element::Type::Bitangent, 3u),
            VertexLayout::Element(VertexLayout::Element::Type::UV0, 2u)
        };

        const float halfWidth = m_size / 2.f;
        m_vertexData =
        {
            //front
            -halfWidth, -halfWidth, halfWidth,
            0.f, 0.f, 1.f, //normal
            1.f, 0.f, 0.f, //tan
            0.f, 1.f, 0.f, //bitan
            0.f, 0.f,      //uv
            halfWidth, -halfWidth, halfWidth,
            0.f, 0.f, 1.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            1.f, 0.f,
            halfWidth, halfWidth, halfWidth,
            0.f, 0.f, 1.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            1.f, 1.f,
            -halfWidth, halfWidth, halfWidth,
            0.f, 0.f, 1.f,
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 1.f,
            //back
            -halfWidth, -halfWidth, -halfWidth,
            0.f, 0.f, -1.f,
            -1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            1.f, 0.f,
            -halfWidth, halfWidth, -halfWidth,
            0.f, 0.f, -1.f,
            -1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            1.f, 1.f,
            halfWidth, halfWidth, -halfWidth,
            0.f, 0.f, -1.f,
            -1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 1.f,
            halfWidth, -halfWidth, -halfWidth,
            0.f, 0.f, -1.f,
            -1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f,
            //top
            -halfWidth, halfWidth, -halfWidth,
            0.f, 1.f, 0.f,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f,
            -halfWidth, halfWidth, halfWidth,
            0.f, 1.f, 0.f,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 0.f,
            halfWidth, halfWidth, halfWidth,
            0.f, 1.f, 0.f,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            1.f, 0.f,
            halfWidth, halfWidth, -halfWidth,
            0.f, 1.f, 0.f,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            1.f, 1.f,
            //bottom
            -halfWidth, -halfWidth, -halfWidth,
            0.f, -1.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            1.f, 1.f,
            halfWidth, -halfWidth, -halfWidth,
            0.f, -1.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f,
            halfWidth, -halfWidth, halfWidth,
            0.f, -1.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 0.f,
            -halfWidth, -halfWidth, halfWidth,
            0.f, -1.f, 0.f,
            1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            1.f, 0.f,
            //right
            halfWidth, -halfWidth, -halfWidth,
            1.f, 0.f, 0.f,
            0.f, 0.f, -1.f,
            0.f, 1.f, 0.f,
            1.f, 0.f,
            halfWidth, halfWidth, -halfWidth,
            1.f, 0.f, 0.f,
            0.f, 0.f, -1.f,
            0.f, 1.f, 0.f,
            1.f, 1.f,
            halfWidth, halfWidth, halfWidth,
            1.f, 0.f, 0.f,
            0.f, 0.f, -1.f,
            0.f, 1.f, 0.f,
            0.f, 1.f,
            halfWidth, -halfWidth, halfWidth,
            1.f, 0.f, 0.f,
            0.f, 0.f, -1.f,
            0.f, 1.f, 0.f,
            0.f, 0.f,
            //left
            -halfWidth, -halfWidth, -halfWidth,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f, 0.f,
            0.f, 0.f,
            -halfWidth, -halfWidth, halfWidth,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f, 0.f,
            1.f, 0.f,
            -halfWidth, halfWidth, halfWidth,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f, 0.f,
            1.f, 1.f,
            -halfWidth, halfWidth, -halfWidth,
            -1.f, 0.f, 0.f,
            0.f, 0.f, 1.f,
            0.f, 1.f, 0.f,
            0.f, 1.f
        };

        m_boundingBox = { {-halfWidth, -halfWidth, -halfWidth}, {halfWidth, halfWidth,halfWidth} };
    }
}

VertexLayout CubeBuilder::getVertexLayout() const
{
    return VertexLayout(m_elements);
}