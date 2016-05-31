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

#ifndef XY_MESH_RESOURCE_HPP_
#define XY_MESH_RESOURCE_HPP_

#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/ModelBuilder.hpp>

#include <map>
#include <memory>

namespace xy
{
    /*!
    \brief Manages the lifetimes of 3D meshes used by model components.
    Multiple model components may reference the same mesh, which can be
    retrieved from the MeshResource via its ID. A mesh must first be 
    added to the resource via the add() function which takes a reference
    to a ModelBuilder instance, used to construct the mesh. The MeshRenderer
    has its own set of resource managers, including a MeshResource, so
    the most common usage would be to allow the MeshRenderer handle resource
    management rather than instancing this class yourself.
    */
    class XY_EXPORT_API MeshResource final
    {
    public:
        using ID = std::uint32_t;

        MeshResource();
        ~MeshResource() = default;

        /*!
        \brief Adds a mesh to the resource and maps it to the given ID.
        \param ID a unique ID for the mesh to be added
        \param MeshBuilder custom mesh builder used to create the mesh mapped to this ID
        \returns Reference to the newly added mesh
        */
        Mesh& add(ID, ModelBuilder&);
        /*!
        \brief Returns a reference to the mesh with the given ID is it is found
        else returns a reference to the default mesh.
        */
        Mesh& get(ID);

    private:
        std::map<ID, std::unique_ptr<Mesh>> m_meshes;
        VertexLayout m_defaultLayout;
        Mesh m_defaultMesh;
    };
}

#endif //XY_MESH_RESOURCE_HPP_