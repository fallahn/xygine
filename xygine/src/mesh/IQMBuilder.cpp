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

#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fstream>
#include <cstring>

using namespace xy;

namespace
{
    const std::string IQM_MAGIC = "INTERQUAKEMODEL";
    const std::uint8_t IQM_VERSION = 2u;

    //data size of vertex properties
    const std::uint8_t positionSize = 3u;
    const std::uint8_t normalSize = 3u;
    const std::uint8_t uvSize = 2u;
    const std::uint8_t blendIndexSize = 4u;
    const std::uint8_t blendWeightSize = 4u;
    const std::uint8_t colourSize = 4u;

    char* readHeader(char* headerPtr, Iqm::Header& dest)
    {
        std::memcpy(&dest.magic, headerPtr, sizeof(dest.magic));
        headerPtr += sizeof(dest.magic);
        std::memcpy(&dest.version, headerPtr, sizeof(dest.version));
        headerPtr += sizeof(dest.version);
        std::memcpy(&dest.filesize, headerPtr, sizeof(dest.filesize));
        headerPtr += sizeof(dest.filesize);
        std::memcpy(&dest.flags, headerPtr, sizeof(dest.flags));
        headerPtr += sizeof(dest.flags);
        std::memcpy(&dest.textCount, headerPtr, sizeof(dest.textCount));
        headerPtr += sizeof(dest.textCount);
        std::memcpy(&dest.textOffset, headerPtr, sizeof(dest.textOffset));
        headerPtr += sizeof(dest.textOffset);
        std::memcpy(&dest.meshCount, headerPtr, sizeof(dest.meshCount));
        headerPtr += sizeof(dest.meshCount);
        std::memcpy(&dest.meshOffset, headerPtr, sizeof(dest.meshOffset));
        headerPtr += sizeof(dest.meshOffset);
        std::memcpy(&dest.varrayCount, headerPtr, sizeof(dest.varrayCount));
        headerPtr += sizeof(dest.varrayCount);
        std::memcpy(&dest.vertexCount, headerPtr, sizeof(dest.vertexCount));
        headerPtr += sizeof(dest.vertexCount);
        std::memcpy(&dest.varrayOffset, headerPtr, sizeof(dest.varrayOffset));
        headerPtr += sizeof(dest.varrayOffset);
        std::memcpy(&dest.triangleCount, headerPtr, sizeof(dest.triangleCount));
        headerPtr += sizeof(dest.triangleCount);
        std::memcpy(&dest.triangleOffset, headerPtr, sizeof(dest.triangleOffset));
        headerPtr += sizeof(dest.triangleOffset);
        std::memcpy(&dest.adjacencyOffset, headerPtr, sizeof(dest.adjacencyOffset));
        headerPtr += sizeof(dest.adjacencyOffset);
        std::memcpy(&dest.jointCount, headerPtr, sizeof(dest.jointCount));
        headerPtr += sizeof(dest.jointCount);
        std::memcpy(&dest.jointOffset, headerPtr, sizeof(dest.jointOffset));
        headerPtr += sizeof(dest.jointOffset);
        std::memcpy(&dest.poseCount, headerPtr, sizeof(dest.poseCount));
        headerPtr += sizeof(dest.poseCount);
        std::memcpy(&dest.poseOffset, headerPtr, sizeof(dest.poseOffset));
        headerPtr += sizeof(dest.poseOffset);
        std::memcpy(&dest.animCount, headerPtr, sizeof(dest.animCount));
        headerPtr += sizeof(dest.animCount);
        std::memcpy(&dest.animOffset, headerPtr, sizeof(dest.animOffset));
        headerPtr += sizeof(dest.animOffset);
        std::memcpy(&dest.frameCount, headerPtr, sizeof(dest.frameCount));
        headerPtr += sizeof(dest.frameCount);
        std::memcpy(&dest.frameChannelCount, headerPtr, sizeof(dest.frameChannelCount));
        headerPtr += sizeof(dest.frameChannelCount);
        std::memcpy(&dest.frameOffset, headerPtr, sizeof(dest.frameOffset));
        headerPtr += sizeof(dest.frameOffset);
        std::memcpy(&dest.boundsOffset, headerPtr, sizeof(dest.boundsOffset));
        headerPtr += sizeof(dest.boundsOffset);
        std::memcpy(&dest.commentCount, headerPtr, sizeof(dest.commentCount));
        headerPtr += sizeof(dest.commentCount);
        std::memcpy(&dest.commentOffset, headerPtr, sizeof(dest.commentOffset));
        headerPtr += sizeof(dest.commentOffset);
        std::memcpy(&dest.extensionCount, headerPtr, sizeof(dest.extensionCount));
        headerPtr += sizeof(dest.extensionCount);
        std::memcpy(&dest.extensionOffset, headerPtr, sizeof(dest.extensionOffset));
        headerPtr += sizeof(dest.extensionOffset);

        return headerPtr;
    }

    char* readVertexArray(char* vertArrayPtr, Iqm::VertexArray& dest)
    {
        std::memcpy(&dest.type, vertArrayPtr, sizeof(dest.type));
        vertArrayPtr += sizeof(dest.type);
        std::memcpy(&dest.flags, vertArrayPtr, sizeof(dest.flags));
        vertArrayPtr += sizeof(dest.flags);
        std::memcpy(&dest.format, vertArrayPtr, sizeof(dest.format));
        vertArrayPtr += sizeof(dest.format);
        std::memcpy(&dest.size, vertArrayPtr, sizeof(dest.size));
        vertArrayPtr += sizeof(dest.size);
        std::memcpy(&dest.offset, vertArrayPtr, sizeof(dest.offset));
        vertArrayPtr += sizeof(dest.offset);

        return vertArrayPtr;
    }

    char* readMesh(char* meshArrayPtr, Iqm::Mesh& dest)
    {
        std::memcpy(&dest.name, meshArrayPtr, sizeof(dest.name));
        meshArrayPtr += sizeof(dest.name);
        std::memcpy(&dest.material, meshArrayPtr, sizeof(dest.material));
        meshArrayPtr += sizeof(dest.material);
        std::memcpy(&dest.firstVertex, meshArrayPtr, sizeof(dest.firstVertex));
        meshArrayPtr += sizeof(dest.firstVertex);
        std::memcpy(&dest.vertexCount, meshArrayPtr, sizeof(dest.vertexCount));
        meshArrayPtr += sizeof(dest.vertexCount);
        std::memcpy(&dest.firstTriangle, meshArrayPtr, sizeof(dest.firstTriangle));
        meshArrayPtr += sizeof(dest.firstTriangle);
        std::memcpy(&dest.triangleCount, meshArrayPtr, sizeof(dest.triangleCount));
        meshArrayPtr += sizeof(dest.triangleCount);

        return meshArrayPtr;
    }

    char* readJoint(char* jointArrayPtr, Iqm::Joint& dest)
    {
        std::memcpy(&dest.name, jointArrayPtr, sizeof(dest.name));
        jointArrayPtr += sizeof(dest.name);
        std::memcpy(&dest.parent, jointArrayPtr, sizeof(dest.parent));
        jointArrayPtr += sizeof(dest.parent);
        std::memcpy(&dest.translate, jointArrayPtr, sizeof(dest.translate));
        jointArrayPtr += sizeof(dest.translate);
        std::memcpy(&dest.rotate, jointArrayPtr, sizeof(dest.rotate));
        jointArrayPtr += sizeof(dest.rotate);
        std::memcpy(&dest.scale, jointArrayPtr, sizeof(dest.scale));
        jointArrayPtr += sizeof(dest.scale);

        return jointArrayPtr;
    }

    char* readPose(char* poseArrayPtr, Iqm::Pose& dest)
    {
        std::memcpy(&dest.parent, poseArrayPtr, sizeof(dest.parent));
        poseArrayPtr += sizeof(dest.parent);
        std::memcpy(&dest.mask, poseArrayPtr, sizeof(dest.mask));
        poseArrayPtr += sizeof(dest.mask);
        std::memcpy(&dest.channelOffset, poseArrayPtr, sizeof(dest.channelOffset));
        poseArrayPtr += sizeof(dest.channelOffset);
        std::memcpy(&dest.channelScale, poseArrayPtr, sizeof(dest.channelScale));
        poseArrayPtr += sizeof(dest.channelScale);

        return poseArrayPtr;
    }

    char* readAnim(char* animArrayPtr, Iqm::Anim& dest)
    {
        std::memcpy(&dest.name, animArrayPtr, sizeof(dest.name));
        animArrayPtr += sizeof(dest.name);
        std::memcpy(&dest.firstFrame, animArrayPtr, sizeof(dest.firstFrame));
        animArrayPtr += sizeof(dest.firstFrame);
        std::memcpy(&dest.frameCount, animArrayPtr, sizeof(dest.frameCount));
        animArrayPtr += sizeof(dest.frameCount);
        std::memcpy(&dest.framerate, animArrayPtr, sizeof(dest.framerate));
        animArrayPtr += sizeof(dest.framerate);
        std::memcpy(&dest.flags, animArrayPtr, sizeof(dest.flags));
        animArrayPtr += sizeof(dest.flags);

        return animArrayPtr;
    }

    glm::mat4 createBoneMatrix(const glm::quat& rotation, const glm::vec3& translation, const glm::vec3& scale)
    {
        glm::mat3 rotMat = glm::inverse(glm::mat3_cast(glm::normalize(rotation)));
        rotMat[0] *= scale;
        rotMat[1] *= scale;
        rotMat[2] *= scale;

        glm::mat4 retVal(rotMat);
        retVal[0].w = translation.x;
        retVal[1].w = translation.y;
        retVal[2].w = translation.z;
        retVal[3].w = 1.0;

        return glm::transpose(retVal);
    }
}

IQMBuilder::IQMBuilder(const std::string& path)
    : m_filePath    (path),
    m_vertexCount   (0)
{
    //TODO we should only be allocating memory for elements
    //which actually exist in the model
    m_elements = 
    {
        {VertexLayout::Element::Type::Position, positionSize},
        {VertexLayout::Element::Type::Normal, normalSize},
        {VertexLayout::Element::Type::Tangent, normalSize},
        {VertexLayout::Element::Type::Bitangent, normalSize},
        {VertexLayout::Element::Type::UV0, uvSize},
        {VertexLayout::Element::Type::BlendIndices, blendIndexSize},
        {VertexLayout::Element::Type::BlendWeights, blendWeightSize},
        {VertexLayout::Element::Type::Colour, colourSize}
    };
}

//public
void IQMBuilder::build()
{   
    if (m_vertexData.empty())
    {
        XY_ASSERT(!m_filePath.empty(), "IQM Model path cannot be empty");

        std::fstream file;
        file.open(m_filePath, std::ios::in | std::ios::out | std::ios::binary);

        if (file.fail())
        {
            Logger::log("Failed to open " + m_filePath + " for loading.", Logger::Type::Error);
            return;
        }

        //get file size and check it's valid
        file.seekg(0, std::ios::end);
        std::uint32_t fileSize = static_cast<std::uint32_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        XY_ASSERT(fileSize, "IQM File size is zero");

        //load file into a vector
        std::vector<char> fileData(fileSize);
        file.read(static_cast<char*>(fileData.data()), fileSize);

        //read header data
        char* headerBytes = fileData.data();

        file.seekg(0);

        Iqm::Header header;
        readHeader(headerBytes, header);


        //validate header data
        if (std::string(header.magic) != IQM_MAGIC)
        {
            Logger::log("Not an IQM file, header returned " + std::string(header.magic) + " instead of " + IQM_MAGIC, Logger::Type::Error);
            return;
        }

        if (header.version != IQM_VERSION)
        {
            Logger::log("Wrong IQM version, found " + std::to_string(header.version) + " expected " + std::to_string(IQM_VERSION), Logger::Type::Error);
            return;
        }

        if (header.filesize > (16 << 20))
        {
            Logger::log("IQM file greater than 16mb, file not loaded", Logger::Type::Error);
            return;
        }

        //load in string data
        char* textIter = headerBytes + header.textOffset;
        std::string strings;
        strings.resize(header.textCount);
        std::memcpy(&strings[0], textIter, header.textCount);

        if (header.vertexCount > 0)
        {
            loadVertexData(header, headerBytes, strings);
        }
        else
        {
            Logger::log("No vertex data was loaded from " + m_filePath + ". Is this an animation file?", Logger::Type::Warning, Logger::Output::All);
        }
        loadAnimationData(header, headerBytes, strings);

        //check to see if model stores a bounding box
        if (header.boundsOffset > 0)
        {
            Iqm::Bounds bounds; //TODO we have unique bounds for each keyframe
            std::memcpy(&bounds, headerBytes + header.boundsOffset, sizeof(Iqm::Bounds));
            m_boundingBox = { { bounds.bbmin[0], bounds.bbmin[1], bounds.bbmin[2] },{ bounds.bbmax[0], bounds.bbmax[1], bounds.bbmax[2] } };
        }
        else
        {
            Logger::log("IQM model " + xy::FileSystem::getFileName(m_filePath) + " contains no bounds value, attempting to use calculated bounds...", Logger::Type::Warning);
            Logger::log("If scene culling appears to work incorrectly consider exporting model with a default animation", Logger::Type::Info);
        }      
    }
}

VertexLayout IQMBuilder::getVertexLayout() const
{
    return VertexLayout(m_elements);
}

void IQMBuilder::loadVertexData(const Iqm::Header& header, char* headerBytes, const std::string& strings)
{   
    //load vertex data (attributes are kept in separate arrays)
    //TODO this memory should only be allocated if the data exists
    char* vertArrayIter = headerBytes + header.varrayOffset;
    std::vector<float> positions(header.vertexCount * positionSize);
    std::vector<float> normals(header.vertexCount * normalSize);
    std::vector<float> tangents(header.vertexCount * (normalSize + 1)); //tangent data is 4 component - w is sign of cross product used for bitan
    std::vector<float> texCoords(header.vertexCount * uvSize);
    std::vector<std::uint8_t> blendIndices(header.vertexCount * blendIndexSize);
    std::vector<std::uint8_t> blendWeights(header.vertexCount * blendWeightSize);
    std::vector<std::uint8_t> colours(header.vertexCount * colourSize);

    for (auto i = 0u; i < header.varrayCount; ++i)
    {
        Iqm::VertexArray vertArray;
        vertArrayIter = readVertexArray(vertArrayIter, vertArray);

        switch (vertArray.type)
        {
        case Iqm::POSITION:
            std::memcpy(&positions[0], headerBytes + vertArray.offset, sizeof(float) * positions.size());
            break;
        case Iqm::NORMAL:
            std::memcpy(&normals[0], headerBytes + vertArray.offset, sizeof(float) * normals.size());
            break;
        case Iqm::TANGENT:
            std::memcpy(&tangents[0], headerBytes + vertArray.offset, sizeof(float) * tangents.size());
            break;
        case Iqm::TEXCOORD:
            std::memcpy(&texCoords[0], headerBytes + vertArray.offset, sizeof(float) * texCoords.size());
            break;
        case Iqm::BLENDINDICES:
            std::memcpy(&blendIndices[0], headerBytes + vertArray.offset, sizeof(uint8_t) * blendIndices.size());
            break;
        case Iqm::BLENDWEIGHTS:
            std::memcpy(&blendWeights[0], headerBytes + vertArray.offset, sizeof(uint8_t) * blendIndices.size());
            break;
        case Iqm::COLOUR:
            std::memcpy(colours.data(), headerBytes + vertArray.offset, sizeof(std::uint8_t) * colours.size());
            break;
        default: break;
        }
    }

    //load index arrays for sub meshes
    //do this first as we need to store triangle data if we create tangent data from UVs/faces
    std::vector<Iqm::Triangle> triangles;

    char* meshBytesIter = headerBytes + header.meshOffset;
    for (auto i = 0u; i < header.meshCount; ++i)
    {
        Iqm::Mesh mesh;
        meshBytesIter = readMesh(meshBytesIter, mesh);

        addMaterialName(&strings[mesh.material]);

        Iqm::Triangle triangle;
        char* triangleIter = headerBytes + header.triangleOffset + (mesh.firstTriangle * sizeof(triangle.vertex));
        std::vector<std::uint16_t> indices;

        for (auto j = 0u; j < mesh.triangleCount; ++j)
        {
            std::memcpy(triangle.vertex, triangleIter, sizeof(triangle.vertex));
            triangleIter += sizeof(triangle.vertex);
            //IQM has triangles wound CW so they need reversing
            indices.push_back(triangle.vertex[2]);
            indices.push_back(triangle.vertex[1]);
            indices.push_back(triangle.vertex[0]);

            //if (createTanNormals) //cache triangles as we'll need them
            /*{
            triangles.push_back(triangle);
            }*/
        }

        m_indexArrays.push_back(indices);

        SubMeshLayout sml;
        sml.data = (void*)m_indexArrays.back().data();
        sml.indexFormat = Mesh::IndexFormat::I16;
        sml.size = m_indexArrays[i].size();
        sml.type = Mesh::PrimitiveType::Triangles;
        addSubMeshLayout(sml);
    }


    //calc bitans
    std::vector<float> pureTangents; //tangents with 4th component removed
    std::vector<float> bitangents;
    //if (!createTanNormals) //use loaded data from model
    {
        for (auto i = 0u, j = 0u; i < normals.size(); i += normalSize, j += (normalSize + 1))
        {
            glm::vec3 normal(normals[i], normals[i + 1u], normals[i + 2u]);
            glm::vec3 tangent(tangents[j], tangents[j + 1u], tangents[j + 2u]);
            glm::vec3 bitan = glm::cross(normal, tangent) * -tangents[j + 3u];

            pureTangents.push_back(tangent.x);
            pureTangents.push_back(tangent.y);
            pureTangents.push_back(tangent.z);

            bitangents.push_back(bitan.x);
            bitangents.push_back(bitan.y);
            bitangents.push_back(bitan.z);
        }
    }
    //else
    //{
    //    //do manual calc from UVs (remember we haven't swapped coords yet!)
    //    pureTangents.resize(normals.size());
    //    std::memset(&pureTangents[0], 0, pureTangents.size());
    //    bitangents.resize(normals.size());
    //    std::memset(&bitangents[0], 0, bitangents.size());

    //    normals.clear();
    //    normals.resize(pureTangents.size());
    //    std::memset(&normals[0], 0, normals.size());

    //    for (const auto& t : triangles)
    //    {
    //        //calc face normal from vertex positions
    //        std::vector<glm::vec3> facePositions =
    //        {
    //            vec3FromArray(t.vertex[0], positions),
    //            vec3FromArray(t.vertex[1], positions),
    //            vec3FromArray(t.vertex[2], positions)
    //        };

    //        glm::vec3 deltaPos1 = facePositions[1] - facePositions[0];
    //        glm::vec3 deltaPos2 = facePositions[2] - facePositions[0];
    //        glm::vec3 faceNormal = -glm::cross(deltaPos1, deltaPos2);

    //        addVecToArray(t.vertex[0], normals, faceNormal);
    //        addVecToArray(t.vertex[1], normals, faceNormal);
    //        addVecToArray(t.vertex[2], normals, faceNormal);

    //        //and tan / bitan from UV
    //        std::vector<glm::vec2> faceUVs =
    //        {
    //            vec2FromArray(t.vertex[0], texCoords),
    //            vec2FromArray(t.vertex[1], texCoords),
    //            vec2FromArray(t.vertex[2], texCoords)
    //        };

    //        glm::vec2 deltaUV1 = faceUVs[1] - faceUVs[0];
    //        glm::vec2 deltaUV2 = faceUVs[2] - faceUVs[0];

    //        float sign = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    //        glm::vec3 faceTan = (deltaPos1 * deltaUV2.y - deltaPos2 *deltaUV1.y) * sign;
    //        glm::vec3 faceBitan = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * sign;

    //        swapZY(faceTan);
    //        swapZY(faceBitan);

    //        addVecToArray(t.vertex[0], pureTangents, faceTan);
    //        addVecToArray(t.vertex[1], pureTangents, faceTan);
    //        addVecToArray(t.vertex[2], pureTangents, faceTan);

    //        addVecToArray(t.vertex[0], bitangents, faceBitan);
    //        addVecToArray(t.vertex[1], bitangents, faceBitan);
    //        addVecToArray(t.vertex[2], bitangents, faceBitan);
    //    }

    /*normaliseVec3Array(normals);
    normaliseVec3Array(pureTangents);
    normaliseVec3Array(bitangents);
    }*/

    //interleave data
    std::uint32_t posIndex = 0u;
    std::uint32_t normalIndex = 0u;
    std::uint32_t tanIndex = 0u;
    std::uint32_t bitanIndex = 0u;
    std::uint32_t uvIndex = 0u;
    std::uint32_t blendIndex = 0u;
    std::uint32_t blendWeightIndex = 0u;
    std::uint32_t colourIndex = 0u;

    glm::vec3 min, max;
    for (auto i = 0u; i < header.vertexCount; ++i)
    {
        for (auto j = 0u; j < positionSize; ++j)
        {
            float value = positions[posIndex++];
            //guestimate a bounding box
            switch (j)
            {
            default: break;
            case 0:
                if (value < min.x) min.x = value;
                else if (value > max.x) max.x = value;
                break;
            case 1:
                if (value < min.y) min.y = value;
                else if (value > max.y) max.y = value;
                break;
            case 2:
                if (value < min.z) min.z = value;
                else if (value > max.z) max.z = value;
                break;
            }
            m_vertexData.push_back(value);
        }

        for (auto j = 0u; j < normalSize; ++j)
        {
            m_vertexData.push_back(normals[normalIndex++]);
        }

        for (auto j = 0u; j < normalSize; ++j)
        {
            m_vertexData.push_back(pureTangents[tanIndex++]);
        }

        for (auto j = 0u; j < normalSize; ++j)
        {
            m_vertexData.push_back(bitangents[bitanIndex++]);
        }

        for (auto j = 0u; j < uvSize; ++j)
        {
            m_vertexData.push_back(texCoords[uvIndex++]);
        }

        for (auto j = 0u; j < blendIndexSize; ++j)
        {
            m_vertexData.push_back(static_cast<float>(blendIndices[blendIndex++]));
        }

        for (auto j = 0u; j < blendWeightSize; ++j)
        {
            //blend weights need to be normalised
            m_vertexData.push_back(static_cast<float>(blendWeights[blendWeightIndex++]) / 255.f);
        }

        for (auto j = 0u; j < colourSize; ++j)
        {
            m_vertexData.push_back(static_cast<float>(colours[colourIndex++]) / 255.f);
        }
    }
    m_boundingBox = { min, max };
    m_vertexCount = header.vertexCount;
}

void IQMBuilder::loadAnimationData(const Iqm::Header& header, char* headerBytes, const std::string& strings)
{
    //load joints into bind pose
    char* jointIter = headerBytes + header.jointOffset;
    std::vector<glm::mat4> bindPose(header.jointCount);
    std::vector<glm::mat4> inverseBindPose(header.jointCount);
    std::vector<std::int32_t> jointIndices(header.jointCount);

    for (auto i = 0u; i < header.jointCount; ++i)
    {
        Iqm::Joint joint;
        jointIter = readJoint(jointIter, joint);

        glm::quat rotation;
        rotation.w = joint.rotate[3];
        rotation.x = joint.rotate[0];
        rotation.y = joint.rotate[1];
        rotation.z = joint.rotate[2];

        glm::vec3 translation(joint.translate[0], joint.translate[1], joint.translate[2]);
        glm::vec3 scale(joint.scale[0], joint.scale[1], joint.scale[2]);

        bindPose[i] = createBoneMatrix(rotation, translation, scale);
        inverseBindPose[i] = glm::inverse(bindPose[i]);

        if (joint.parent >= 0)
        {
            //multiply by parent's transform - IQM's matrix multiplication is reversed
            bindPose[i] = bindPose[joint.parent] * bindPose[i];
            inverseBindPose[i] *= inverseBindPose[joint.parent];

        }
        jointIndices[i] = joint.parent; //we need to track these for blending frames during animation
    }

    //load keyframes - a 'pose' is a single posed joint, and a set of poses makes up one frame equivalent to a posed skeleton
    std::vector<std::vector<glm::mat4>> keyframes(header.frameCount);
    std::uint16_t* frameIter = (std::uint16_t*)(headerBytes + header.frameOffset);
    if (bindPose.size() > 0)
    {
        for (auto frameIndex = 0u; frameIndex < header.frameCount; ++frameIndex)
        {
            char* poseIter = headerBytes + header.poseOffset;
            for (auto poseIndex = 0u; poseIndex < header.poseCount; ++poseIndex)
            {
                Iqm::Pose pose;
                poseIter = readPose(poseIter, pose);

                keyframes[frameIndex].resize(header.poseCount); 

                glm::quat rotation;
                glm::vec3 translation, scale;

                translation.x = pose.channelOffset[0];
                if (pose.mask & 0x01) translation.x += *frameIter++ * pose.channelScale[0];
                translation.y = pose.channelOffset[1];
                if (pose.mask & 0x02) translation.y += *frameIter++ * pose.channelScale[1];
                translation.z = pose.channelOffset[2];
                if (pose.mask & 0x04) translation.z += *frameIter++ * pose.channelScale[2];

                rotation.x = pose.channelOffset[3];
                if (pose.mask & 0x08) rotation.x += *frameIter++ * pose.channelScale[3];
                rotation.y = pose.channelOffset[4];
                if (pose.mask & 0x10) rotation.y += *frameIter++ * pose.channelScale[4];
                rotation.z = pose.channelOffset[5];
                if (pose.mask & 0x20) rotation.z += *frameIter++ * pose.channelScale[5];
                rotation.w = pose.channelOffset[6];
                if (pose.mask & 0x40) rotation.w += *frameIter++ * pose.channelScale[6];

                scale.x = pose.channelOffset[7];
                if (pose.mask & 0x80) scale.x += *frameIter++ * pose.channelScale[7];
                scale.y = pose.channelOffset[8];
                if (pose.mask & 0x100) scale.y += *frameIter++ * pose.channelScale[8];
                scale.z = pose.channelOffset[9];
                if (pose.mask & 0x200) scale.z += *frameIter++ * pose.channelScale[9];

                glm::mat4 mat = createBoneMatrix(rotation, translation, scale);
                if (pose.parent >= 0)
                {
                    keyframes[frameIndex][poseIndex] = bindPose[pose.parent] * mat * inverseBindPose[poseIndex];
                }
                else
                {
                    keyframes[frameIndex][poseIndex] = mat * inverseBindPose[poseIndex];
                }
            }
        }
    }
    keyframes.emplace_back(bindPose.size()); //use an empty frame in case we haven't loaded any animations

    char*  animIter = headerBytes + header.animOffset;
    for (auto animIndex = 0u; animIndex < header.animCount; ++animIndex)
    {
        Iqm::Anim anim;
        animIter = readAnim(animIter, anim);
        addAnimation({ anim.frameCount, anim.firstFrame, anim.framerate,{ &strings[anim.name] }, (anim.flags & Iqm::IQM_LOOP) != 0 });
    }
    //TODO search path for external animation files to load
    setSkeleton(jointIndices, keyframes);
}