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

#ifndef XY_MESH_HPP_
#define XY_MESH_HPP_

#include <xygine/mesh/VertexLayout.hpp>
#include <xygine/mesh/BoundingBox.hpp>
#include <xygine/detail/GLExtensions.hpp>
#include <xygine/detail/Aliases.hpp>

#include <vector>
#include <memory>

namespace xy
{
    class SubMesh;
    /*!
    \brief A mesh represents the collection of vertices either
    as a single OpenGL VBO or a collection of submeshes used by
    a model component to draw a model in a scene.
    */
    class XY_EXPORT_API Mesh final
    {
        friend class SubMesh;
    public:
        /*!
        \brief Describes the number of bytes used in the
        VBO index to calculate proper stride
        */
        enum class IndexFormat
        {
            I8  = GL_UNSIGNED_BYTE,
            I16 = GL_UNSIGNED_SHORT,
            I32 = GL_UNSIGNED_INT
        };

        /*!
        \brief Describes the primitive type used when drawing this mesh
        */
        enum class PrimitiveType
        {
            Points        = GL_POINTS,
            Lines         = GL_LINES,
            LineStrip     = GL_LINE_STRIP,
            Triangles     = GL_TRIANGLES,
            TriangleStrip = GL_TRIANGLE_STRIP
        };
        /*!
        \brief Constructor
        \param VertexLayout Description of the vertex layout used by this mesh
        \param std::size_t Number of vertices in this mesh
        \param dynamic Set true if this mesh will be updated dynamically else false
        */
        Mesh(const VertexLayout&, std::size_t, bool dynamic = false);
        ~Mesh();
        Mesh(const Mesh&) = delete;
        const Mesh& operator = (const Mesh&) = delete;
        /*Mesh(Mesh&&) noexcept = default;*/
        Mesh& operator = (Mesh&&) = default;

        /*!
        \brief Returns a reference to the vertex layoutused by this mesh
        */
        const VertexLayout& getVertexLayout() const;
        /*!
        \brief Returns the number of vertices in this mesh
        */
        std::size_t getVertexCount() const;
        /*!
        \brief Returns the size of a single vertex in bytes.
        When using this to calculate stride, for example, divide
        this value by sizeof(float)
        */
        std::size_t getVertexSize() const;
        /*!
        \brief Returns the OpenGL handle for this mesh's VBO
        */
        VertexBufferID getBufferID() const;
        /*!
        \brief Returns true if this VBO is hinted with GL_DYNAMIC_DRAW, else false
        */
        bool dynamic() const;
        /*!
        \brief Returns the mesh's current PrimitiveType
        */
        PrimitiveType getPrimitiveType() const;
        /*!
        \brief Sets the PrimitiveType to use when this mesh is drawn
        */
        void setPrimitiveType(PrimitiveType);
        /*!
        \brief uploads the given vertex data to the VBO.
        \param data Pointer to floating point array of data to upload.
        \param count Number of vertices to upload.
        \param offset Index in the VBO at which to start uploading data.
        This allows either completely replacing the VBO vertex data, or
        a smaller part of it using the offset parameter and count value
        to state whereabouts in the array the givene data should be copied to
        */
        void setVertexData(const float* data, std::size_t count = 0, std::size_t offset = 0);
        /*!
        \brief Adds a SubMesh to this mesh.
        SubMeshes are index arrays used to draw only part of the mesh, for example when needing
        to switch materials within a single model. This function creates a SubMesh which is owned
        by the mesh and returns a reference to it should it need to be modified.
        \param PrimitiveType The primitive type used to draw the SubMesh
        \param IndexFormat The size in bytes of the indices used by the SubMesh
        \param std::size_t Number of indices in the SubMesh
        \param dynamic Set to true if the vertices in this SubMesh will be updated dynamically
        */
        SubMesh& addSubMesh(PrimitiveType, IndexFormat, std::size_t, bool dynamic = false);
        /*!
        \brief Returns the number of SubMeshes used by this mesh
        */
        std::size_t getSubMeshCount() const;
        /*!
        \brief Returns a pointer to the SubMesh at the given index if it is found, else nullptr
        */
        SubMesh* getSubMesh(std::size_t) const;
        /*!
        \brief returns the 3D AABB for this mesh in local coordinates
        */
        const BoundingBox& getBoundingBox() const { return m_boundingBox; }
        /*!
        \brief Set the BoundgBox used as the 3D AABB for this mesh, in local coordinates
        */
        void setBoundingBox(const BoundingBox& bb) { m_boundingBox = bb; }
    private:

        VertexLayout m_vertexLayout;
        std::size_t m_vertexCount;
        VertexBufferID m_vboID;
        PrimitiveType m_primitiveType;
        std::vector<std::unique_ptr<SubMesh>> m_subMeshes;
        bool m_dynamic;

        BoundingBox m_boundingBox;

        struct Lock final {};
    };
}

#endif //XY_MESH_HPP_