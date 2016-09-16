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

#ifndef XY_IQM_BUILDER_HPP_
#define XY_IQM_BUILDER_HPP_

#include <xygine/mesh/ModelBuilder.hpp>
#include <xygine/mesh/BoundingBox.hpp>

#include <string>
#include <vector>

namespace xy
{
    namespace Iqm
    {
        struct Header;
    }
    
    /*!
    \brief ModelBuilder implementation for loading IQM format models.
    <a href="http://sauerbraten.org/iqm/">Format Details</a>
    */
    class XY_EXPORT_API IQMBuilder final : public xy::ModelBuilder
    {
    public:
        /*!
        \brief Constructor
        \param Path to IQM file to load
        \param bool true to re-calculate tangent normals while loading the model
        */
        explicit IQMBuilder(const std::string&, bool = false);
        ~IQMBuilder() = default;

        void build() override;
        VertexLayout getVertexLayout() const override;
        const float* getVertexData() const override { return m_vertexData.data(); }
        std::size_t getVertexCount() const { return m_vertexCount; }
        const BoundingBox& getBoundingBox() const override { return m_boundingBox; }

    private:
        std::vector<float> m_vertexData;
        std::vector<std::vector<std::uint16_t>> m_indexArrays;
        std::vector<VertexLayout::Element> m_elements;
        BoundingBox m_boundingBox;
        std::string m_filePath;
        std::size_t m_vertexCount;
        bool m_buildTangents;

        void loadVertexData(const Iqm::Header&, char*, const std::string&);
        void loadAnimationData(const Iqm::Header&, char*, const std::string&);
    };

    /*!
    \brief IQM format specific structs
    */
    namespace Iqm
    {
        struct Header
        {
            char magic[16];
            unsigned int version;
            unsigned int filesize;
            unsigned int flags;
            unsigned int textCount, textOffset;
            unsigned int meshCount, meshOffset;
            unsigned int varrayCount, vertexCount, varrayOffset;
            unsigned int triangleCount, triangleOffset, adjacencyOffset;
            unsigned int jointCount, jointOffset;
            unsigned int poseCount, poseOffset;
            unsigned int animCount, animOffset;
            unsigned int frameCount, frameChannelCount, frameOffset, boundsOffset;
            unsigned int commentCount, commentOffset;
            unsigned int extensionCount, extensionOffset;
        };

        struct Mesh
        {
            unsigned int name;
            unsigned int material;
            unsigned int firstVertex, vertexCount;
            unsigned int firstTriangle, triangleCount;
        };

        enum
        {
            POSITION = 0,
            TEXCOORD = 1,
            NORMAL = 2,
            TANGENT = 3,
            BLENDINDICES = 4,
            BLENDWEIGHTS = 5,
            COLOUR = 6,
            CUSTOM = 0x10
        };

        enum
        {
            BYTE = 0,
            UBYTE = 1,
            SHORT = 2,
            USHORT = 3,
            INT = 4,
            UINT = 5,
            HALF = 6,
            FLOAT = 7,
            DOUBLE = 8
        };

        struct Triangle
        {
            unsigned int vertex[3];
        };

        struct Adjacency
        {
            unsigned int triangle[3];
        };

        struct Jointv1
        {
            unsigned int name;
            int parent;
            float translate[3], rotate[3], scale[3];
        };

        struct Joint
        {
            unsigned int name;
            int parent;
            float translate[3], rotate[4], scale[3];
        };

        struct Posev1
        {
            int parent;
            unsigned int mask;
            float channelOffset[9];
            float channelScale[9];
        };

        struct Pose
        {
            int parent;
            unsigned int mask;
            float channelOffset[10];
            float channelScale[10];
        };

        struct Anim
        {
            unsigned int name;
            unsigned int firstFrame, frameCount;
            float framerate;
            unsigned int flags;
        };

        enum
        {
            IQM_LOOP = 1 << 0
        };

        struct VertexArray
        {
            unsigned int type;
            unsigned int flags;
            unsigned int format;
            unsigned int size;
            unsigned int offset;
        };

        struct Bounds
        {
            float bbmin[3], bbmax[3];
            float xyradius, radius;
        };

        struct Extension
        {
            unsigned int name;
            unsigned int dataCount, dataOffset;
            unsigned int extensionOffset; // pointer to next extension
        };
    } //namespace Iqm
}

#endif //XY_IQM_BUILDER_HPP_