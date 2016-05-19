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

#include <xygine/mesh/UniformBuffer.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/detail/GLExtensions.hpp>
#include <xygine/Log.hpp>

using namespace xy;

namespace
{
    GLint MaxUBOs = 0;
    GLint NextIndex = 0;
}


UniformBuffer::UniformBuffer(const std::string& name)
    : m_name    (name),
    m_id        (0),
    m_index     (0),
    m_typeIndex (0)
{
    if (MaxUBOs == 0)
    {
        //query our hardware
        glCheck(glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &MaxUBOs));
    }

    if (NextIndex < MaxUBOs)
    {
        m_index = ++NextIndex;
    }
    else
    {
        //TODO we can check if any previously assigned
        //indices have been freed  first and assign that
        xy::Logger::log("Maximum UBO count has been reached, UBO " + name + " will not be valid", xy::Logger::Type::Error);
    }
}

UniformBuffer::~UniformBuffer()
{
    if (m_id)
    {
        //delete our buffer
        glCheck(glDeleteBuffers(1, &m_id));

        //TODO mark our index as free
    }
}

//public
void UniformBuffer::bind(ShaderID shaderID, UniformBlockID blockID) const
{
    glCheck(glBindBufferBase(GL_UNIFORM_BUFFER, m_index, m_id));
    glCheck(glUniformBlockBinding(shaderID, blockID, m_index));
}