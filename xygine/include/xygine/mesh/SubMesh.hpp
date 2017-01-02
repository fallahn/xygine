/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_SUBMESH_HPP_
#define XY_SUBMESH_HPP_

#include <xygine/mesh/Mesh.hpp>

namespace xy
{
    /*!
    \brief SubMeshes represent an Index Array within a mesh.
    Usually SubMeshes are used when switching between multiple
    materials within a single model.
    */
    class XY_EXPORT_API SubMesh final
    {
        friend class Mesh;
    public:
        SubMesh(Mesh&, std::size_t, Mesh::PrimitiveType, Mesh::IndexFormat, std::size_t, bool, const Mesh::Lock&);
        ~SubMesh();
        SubMesh(const SubMesh&) = delete;
        SubMesh& operator = (const SubMesh&) = delete;

        /*!
        \brief Returns the SubMesh's index with its parent array
        */
        std::size_t getMeshIndex() const;
        /*!
        \brief Returns the current PrimitiveType used to draw this SubMesh
        */
        Mesh::PrimitiveType getPrimitiveType() const;
        /*!
        \brief Returns the number of indices stored in this SubMesh's buffer
        */
        std::size_t getIndexCount() const;
        /*!
        \brief Returns the format of the index data, used to correctly calculate
        the stride of the array
        */
        Mesh::IndexFormat getIndexFormat() const;
        /*!
        \brief Returns the OpenGL handle of the index buffer used by this SubMesh
        */
        IndexBufferID getIndexBufferID() const;
        /*!
        \brief Returns true if this SubMesh's buffer is hinted with GL_DYNAMIC_DRAW
        */
        bool dynamic() const;
        
        /*!
        \brief Updates the index buffer's data.
        \param data Pointer to data to be uploaded
        \param count Amount of data to upload, 0 to upload all
        \param start Index at which to start uploading the data
        */
        void setIndexData(const void* data, std::size_t count, std::size_t start);

    private:
        Mesh* m_mesh;
        std::size_t m_meshIndex;
        Mesh::PrimitiveType m_primitiveType;
        Mesh::IndexFormat m_indexFormat;
        std::size_t m_count;
        IndexBufferID m_iboID;
        bool m_dynamic;
    };
}

#endif //XY_SUBMESH_HPP_