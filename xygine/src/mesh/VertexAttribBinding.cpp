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

#include <xygine/mesh/VertexAttribBinding.hpp>
#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/StaticConsts.hpp>

#include <xygine/detail/GLCheck.hpp>
#include <xygine/detail/GLExtensions.hpp>

#include <xygine/Log.hpp>

using namespace xy;

namespace
{
    GLint MaxVertexAttribs = 0;
}

VertexAttribBinding::VertexAttribBinding(const Mesh& mesh, const Material& material)
    : m_id  (0)
{
    if (MaxVertexAttribs == 0)
    {
        //query the context for max vertex attribs
        GLint count = 0;
        glCheck(glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &count));

        if (count <= 0)
        {
            Logger::log("Current device supports 0 or less vertex attributes, creating VAO failed.", Logger::Type::Error, Logger::Output::All);
        }
        MaxVertexAttribs = count;
    }


    //TODO shouldn't we check we've not created more than we can handle?
    //not sure if genVertArrays returns 0 if it can't create any more
    if (MaxVertexAttribs > 0)
    {
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
        glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

        glCheck(glGenVertexArrays(1, &m_id));

        if (m_id != 0)
        {
            glCheck(glBindVertexArray(m_id));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, mesh.getBufferID()));

            const auto& layout = mesh.getVertexLayout();
            
            auto offset = 0;
            auto count = layout.getElementCount();
            for (auto i = 0u; i < count; ++i)
            {
                const VertexLayout::Element& e = layout.getElement(i);
                VertexAttribID attribID;

                switch (e.type)
                {
                case VertexLayout::Element::Type::Bitangent:
                    attribID = material.getVertexAttributeID(VertexAttribBitangent);
                    break;
                case VertexLayout::Element::Type::Tangent:
                    attribID = material.getVertexAttributeID(VertexAttribTangent);
                    break;
                case VertexLayout::Element::Type::Position:
                    attribID = material.getVertexAttributeID(VertexAttribPosition);
                    break;
                case VertexLayout::Element::Type::Colour:
                    attribID = material.getVertexAttributeID(VertexAttribColour);
                    break;
                case VertexLayout::Element::Type::Normal:
                    attribID = material.getVertexAttributeID(VertexAttribNormal);
                    break;
                case VertexLayout::Element::Type::UV0:
                    attribID = material.getVertexAttributeID(VertexAttribUV0);
                    break;
                case VertexLayout::Element::Type::UV1:
                    attribID = material.getVertexAttributeID(VertexAttribUV1);
                    break;
                case VertexLayout::Element::Type::BlendIndices:
                    attribID = material.getVertexAttributeID(VertexAttribBlendIndices);
                    break;
                case VertexLayout::Element::Type::BlendWeights:
                    attribID = material.getVertexAttributeID(VertexAttribBlendWeights);
                    break;
                default: attribID = -1;
                    break;
                }

                if (attribID != -1)
                {
                    //void* pointer = ptr ? static_cast<void*>(static_cast<UInt8*>(ptr) + offset) : (void*)offset;
                    //setVertAttribPointer(attribID, static_cast<GLint>(e.size), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(layout.getVertexSize()), pointer);

                    glCheck(glVertexAttribPointer(attribID, static_cast<GLint>(e.size), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(layout.getVertexSize()), (void*)offset));
                    glCheck(glEnableVertexAttribArray(attribID));
                }

                offset += e.size * sizeof(float);
            }
        }

        else
        {
            Logger::log("Failed creating the VAO", xy::Logger::Type::Error, Logger::Output::All);
        }
    }
}

VertexAttribBinding::~VertexAttribBinding()
{
    if (m_id)
    {
        glCheck(glDeleteVertexArrays(1, &m_id));
    }
}

//public
void VertexAttribBinding::bind()
{
    glCheck(glBindVertexArray(m_id));
}

void VertexAttribBinding::unbind()
{
    glCheck(glBindVertexArray(0));
}