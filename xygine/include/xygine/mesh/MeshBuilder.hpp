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

#ifndef XY_MESH_BUILDER_HPP_
#define XY_MESH_BUILDER_HPP_

#include <xygine/mesh/Mesh.hpp>

namespace xy
{
    class BoundingBox;
    /*!
    \brief Mesh builder class.
    The MeshBuilder is an abstract base class from which should
    be inherited when creating a Mesh loader or generator, used
    with the MeshRecource class. The MeshBuilder provides a single
    interface which can be implemented to provide MeshResources
    with the data they require. Instances are used once only when
    they are added to a MeshResource so no special lifetime
    requirements exist, and can be discarded once they have been used.
    */
    class XY_EXPORT_API MeshBuilder
    {
    public:
        /*!
        \brief Describes the layout of a SubMesh
        belonging to the built mesh, and a pointer
        to its data.
        */
        struct SubMeshLayout final
        {
            Mesh::PrimitiveType type = Mesh::PrimitiveType::Triangles; //< Primitive type used by SubMesh
            Mesh::IndexFormat indexFormat = Mesh::IndexFormat::I8; //< Size of the Index values used by SubMesh
            std::size_t size = 0; //< Number of indices in the SubMesh
            void* data = nullptr; //< Pointer to the index array used by SubMesh
        };
        
        /*!
        \brief Default constructor
        */
        MeshBuilder() = default;
        virtual ~MeshBuilder() = default;

        /*!
        \brief Builds the mesh data.
        This should be overloaded by derived classes to
        assemble CPU side vertex data ready for consumption
        by the MeshResource class. This is normally where 
        mesh format specific parsing would occur, for example
        when loading external files from disk exported from a
        modelling program, or other external source.
        */
        virtual void build() = 0;

        /*!
        \brief Returns the vertex layout information for the mesh.
        This function needs to be implemented to provide the
        MeshResource vertex layout information for the loaded mesh.
        \see VertexLayout
        */
        virtual xy::VertexLayout getVertexLayout() const = 0;

        /*!
        \brief Returns a pointer to the vertex data.
        This should return a pointer to the beginning of an array of floats
        containing the vertex data as described by the VertexLayout returned
        by getVertexLayout()
        */
        virtual const float* getVertexData() const = 0;

        /*!
        \brief This must return the total number of vertices which make up
        the mesh.
        */
        virtual std::size_t getVertexCount() const = 0;

        /*!
        \brief Returns the three dimensional AABB for this mesh
        */
        virtual const BoundingBox& getBoundingBox() const = 0;

        /*!
        \brief Returns whether or not the mesh created should be hinted as dynamic
        */
        virtual bool dynamic() const { return false; }

        /*!
        \brief Returns the Primitive type to be used when drawing this mesh
        */
        virtual Mesh::PrimitiveType primitiveType() const { return Mesh::PrimitiveType::TriangleStrip; }

        /*!
        \brief Returns a vector of SubMeshLayout.
        If a mesh requires SubMeshes (index arrays) implement this and
        return a struct for each SubMesh required
        */
        virtual std::vector<SubMeshLayout> getSubMeshLayouts() const { return{}; }

    private:

    };
}

#endif //XY_MESH_BUILDER_HPP_