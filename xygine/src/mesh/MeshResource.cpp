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

#include <xygine/mesh/MeshResource.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/Log.hpp>
#include <xygine/Assert.hpp>

#include <array>

using namespace xy;

MeshResource::MeshResource()
    : m_defaultLayout   ({ xy::VertexLayout::Element(xy::VertexLayout::Element::Type::Position, 3) }),
    m_defaultMesh       (m_defaultLayout, 4)
{
    //TODO add colour data
    std::array<float, 12> verts =
    {
        -10.5f, -10.5f, 0.f,
        -10.5f, 10.5f, 0.f,
        10.5f, -10.5f, 0.f,
        10.5f, 10.5f, 0.f
    };
    m_defaultMesh.setVertexData(verts.data());
    m_defaultMesh.setPrimitiveType(xy::Mesh::PrimitiveType::TriangleStrip);
    m_defaultMesh.setBoundingBox({ {-10.5, -10.5f, -1.f}, {10.5f, 10.5f, 1.f} });
}

//public
Mesh& MeshResource::add(MeshResource::ID id, ModelBuilder& mb)
{
    XY_ASSERT(m_meshes.find(id) == m_meshes.end(), "Mesh ID is already used by this resource");
    
    mb.build();

    Mesh m(mb.getVertexLayout(), mb.getVertexCount(), mb.dynamic());

    m_meshes.insert(std::make_pair(id, std::make_unique<Mesh>(mb.getVertexLayout(), mb.getVertexCount(), mb.dynamic())));

    auto& mesh = *m_meshes.find(id)->second;
    mesh.setVertexData(mb.getVertexData());
    mesh.setPrimitiveType(mb.primitiveType());

    const auto& subMeshLayouts = mb.getSubMeshLayouts();
    if (!subMeshLayouts.empty())
    {
        for (const auto& sml : subMeshLayouts)
        {
            auto& subMesh = mesh.addSubMesh(sml.type, sml.indexFormat, sml.size, mb.dynamic());
            subMesh.setIndexData(sml.data, 0, 0);
        }
    }
    mesh.setBoundingBox(mb.getBoundingBox());
    return mesh;
}

Mesh& MeshResource::get(MeshResource::ID id)
{
    auto result = m_meshes.find(id);
    if (result != m_meshes.end())
    {
        return *result->second;
    }
    LOG("Could not find Mesh " + std::to_string(id) + " returning default mesh", Logger::Type::Error);
    return m_defaultMesh;
}