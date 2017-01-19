/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_MODEL_BUILDER_HPP_
#define XY_MODEL_BUILDER_HPP_

#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/Skeleton.hpp>

#include <memory>

namespace xy
{
    class BoundingBox;
    /*!
    \brief ModelBuilder class.
    This abstract base class provides an interface for the mesh renderer to
    load abitrary model types. Inherit this class when creating a loader for
    a specific file format, or a mesh generator (such as the CubeBuilder or 
    QuadBuilder).
    */
    class XY_EXPORT_API ModelBuilder
    {
    public:
        /*!
        \brief Describes the layout of a SubMesh
        belonging to the built mesh, and a pointer
        to its data. When parsing model data these
        should be stored using addSubMeshLayout() for 
        each submesh index in the model (if any)
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
        ModelBuilder() = default;
        virtual ~ModelBuilder() = default;

        /*!
        \brief Builds the mesh data.
        This should be overloaded by derived classes to
        assemble CPU side vertex data ready for consumption
        by the MeshRenderer class. This is normally where 
        mesh format specific parsing would occur, for example
        when loading external files from disk exported from a
        modelling program, or other external source. Metadata
        such as material names or animation info can also be
        loaded here and stored with addMaterialName()
        */
        virtual void build() = 0;

        /*!
        \brief Returns the vertex layout information for the mesh.
        This function needs to be implemented to provide the
        MeshRenderer vertex layout information for the loaded mesh.
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
        */
        const std::vector<SubMeshLayout>& getSubMeshLayouts() const { return m_subMeshes; }

        /*!
        \brief Returns a list of material names (if they are stored in the model format)
        in the order to which they applied to submeshes (or base mesh if there is only one)
        */
        const std::vector<std::string>& getMaterialNames() const { return m_materialNames; }

        /*!
        \brief Returns a unique_ptr to a Skeleton if the model has one, else nullptr
        */
        std::unique_ptr<Skeleton> getSkeleton() { return std::move(m_skeleton); }

        /*!
        \brief Returns a vector of Animation structs if the model contains animations
        else returns an empty vector
        This is only useful to query if the getSkeleton() function doesn't return nullptr
        */
        const std::vector<Skeleton::Animation>& getAnimations() const { return m_animations; }

    protected:
        /*!
        \brief Used to store descriptions of sub-meshes to be returned to the MeshRenderer
        */
        void addSubMeshLayout(const SubMeshLayout& l)
        {
            m_subMeshes.push_back(l);
        }

        /*!
        \brief Used to store the names of materials
        in the order to which they are applied to sub-meshes
        */
        void addMaterialName(const std::string& n)
        {
            m_materialNames.push_back(n);
        }

        /*!
        \brief Allows adding a loaded animation to the vector of Animation structs
        */
        void addAnimation(const Skeleton::Animation& a) { m_animations.push_back(a); }

        /*!
        \brief Sets the skeletal data
        */
        void setSkeleton(const std::vector<std::int32_t>& jointIndices, const std::vector<std::vector<glm::mat4>>& keyFrames)
        {
            if (!jointIndices.empty() && !keyFrames.empty())
            {
                m_skeleton = std::make_unique<Skeleton>(jointIndices, keyFrames);
            }
        }

    private:

        std::vector<SubMeshLayout> m_subMeshes;
        std::vector<std::string> m_materialNames;

        std::unique_ptr<Skeleton> m_skeleton;
        std::vector<Skeleton::Animation> m_animations;
    };
}

#endif //XY_MESH_BUILDER_HPP_