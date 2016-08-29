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

#include <xygine/mesh/SphereBuilder.hpp>
#include <xygine/Assert.hpp>
#include <xygine/util/Const.hpp>

#include <glm/vec3.hpp>
#include <glm/gtc/packing.hpp>

using namespace xy;

SphereBuilder::SphereBuilder(float radius, std::size_t resolution, bool backface)
    : m_radius          (radius),
    m_resolution        (resolution),
    m_generateBackface  (backface),
    m_vertexCount       (0)
{
    XY_ASSERT(resolution > 0 && radius > 0, "Sphere requires size");

    m_elements = 
    {
        VertexLayout::Element(VertexLayout::Element::Type::Position, 3u),
        VertexLayout::Element(VertexLayout::Element::Type::Normal, 3u),
        VertexLayout::Element(VertexLayout::Element::Type::Tangent, 3u),
        VertexLayout::Element(VertexLayout::Element::Type::Bitangent, 3u),
        VertexLayout::Element(VertexLayout::Element::Type::UV0, 2u)
    };
}

//public
void SphereBuilder::build()
{
    if (m_vertexData.empty())
    {
        //generate vertex layout
        std::size_t offset = 0; //offset into index array as we add faces
        std::function<void(const glm::quat&, const glm::vec2&)> buildFace = [this, &offset](const glm::quat& rotation, const glm::vec2& uvOffset)
        {
            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> uvs;
            const float spacing = 1.f / m_resolution;
            const glm::vec2 uvSpacing((1.f / 2.f) / static_cast<float>(m_resolution), (1.f / 3.f) / static_cast<float>(m_resolution));
            for (auto y = 0u; y <= m_resolution; ++y)
            {
                for (auto x = 0u; x <= m_resolution; ++x)
                {
                    positions.emplace_back((x * spacing) - 0.5f, (y * spacing) - 0.5f, 0.5f);
                    uvs.emplace_back(uvOffset.x + (uvSpacing.x * x), uvOffset.y + (uvSpacing.y * y));
                }
            }
          
            for (auto i = 0u; i < positions.size(); ++i)
            {
                auto vert = glm::normalize(positions[i]) * rotation;
                m_vertexData.emplace_back(vert.x * m_radius);
                m_vertexData.emplace_back(vert.y * m_radius);
                m_vertexData.emplace_back(vert.z * m_radius);
                //normals
                m_vertexData.emplace_back(vert.x);
                m_vertexData.emplace_back(vert.y);
                m_vertexData.emplace_back(vert.z);
                
                //because we're on a grid the tan points to
                // the next vertex position - unless we're
                //at the end of a line
                glm::vec3 tan;
                if (i % (m_resolution + 1) == (m_resolution))
                {
                    //end of the line
                    tan = glm::normalize(positions[i - 1] - positions[i]) * rotation;
                    tan = glm::reflect(tan, vert);
                }
                else
                {
                    tan = glm::normalize(positions[i + 1] - positions[i]) * rotation;
                }
                glm::vec3 bitan = glm::cross(tan, vert);

                m_vertexData.emplace_back(tan.x);
                m_vertexData.emplace_back(tan.y);
                m_vertexData.emplace_back(tan.z);

                m_vertexData.emplace_back(bitan.x);
                m_vertexData.emplace_back(bitan.y);
                m_vertexData.emplace_back(bitan.z);

                //UVs
                m_vertexData.emplace_back(uvs[i].x);
                m_vertexData.emplace_back(uvs[i].y);
            }

            //update indices
            for (auto y = 0u; y <= m_resolution; ++y)
            {
                auto start = y * m_resolution;
                for (auto x = 0u; x <= m_resolution; ++x)
                {
                    m_indices.push_back(static_cast<std::uint16_t>(offset + start + m_resolution + x));
                    m_indices.push_back(static_cast<std::uint16_t>(offset + start + x));
                }

                //add a degenerate at the end of the row bar last
                if (y < m_resolution - 1)
                {
                    m_indices.push_back(static_cast<std::uint16_t>(offset + ((y + 1) * m_resolution + (m_resolution - 1))));
                    m_indices.push_back(static_cast<std::uint16_t>(offset + ((y + 1) * m_resolution)));
                }
            }
            offset += positions.size();
        };

        //build each face rotating as we go
        //mapping to 2x3 texture atlas
        const float u = 1.f / 2.f;
        const float v = 1.f / 3.f;
        buildFace(glm::quat(), { 0.f, v * 2.f }); //Z+
        glm::quat rotation = glm::rotate(glm::quat(), xy::Util::Const::degToRad * 90.f, glm::vec3(0.f, 1.f, 0.f));
        buildFace(rotation, { u, 0.f }); //X-

        if (m_generateBackface)
        {
            rotation = glm::rotate(glm::quat(), xy::Util::Const::degToRad * 180.f, glm::vec3(0.f, 1.f, 0.f));
            buildFace(rotation, { u, v * 2.f }); //Z-
        }
        rotation = glm::rotate(glm::quat(), xy::Util::Const::degToRad * 270.f, glm::vec3(0.f, 1.f, 0.f));
        buildFace(rotation, {}); //X+

        rotation = glm::rotate(glm::quat(), xy::Util::Const::degToRad * 90.f, glm::vec3(1.f, 0.f, 0.f));
        buildFace(rotation, { u, v }); //Y-

        rotation = glm::rotate(glm::quat(), xy::Util::Const::degToRad * -90.f, glm::vec3(1.f, 0.f, 0.f));
        buildFace(rotation, { 0.f, v }); //Y+

        //update vertex count
        std::size_t vertSize = 0;
        for (const auto& e : m_elements) vertSize += e.size;
        m_vertexCount = m_vertexData.size() / vertSize;

        //create a submesh layout
        SubMeshLayout sml;
        sml.data = (void*)m_indices.data();
        sml.size = m_indices.size();
        sml.type = Mesh::PrimitiveType::TriangleStrip;
        sml.indexFormat = Mesh::IndexFormat::I16;
        addSubMeshLayout(sml);

        m_boundingBox = { {-m_radius, -m_radius, -m_radius},{m_radius, m_radius, m_radius} };
    }
}

VertexLayout SphereBuilder::getVertexLayout() const
{
    return VertexLayout(m_elements);
}

