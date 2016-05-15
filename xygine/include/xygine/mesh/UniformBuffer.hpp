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

#ifndef XY_UNIFORM_BUFFER_HPP_
#define XY_UNIFORM_BUFFER_HPP_

#include <xygine/Config.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/Assert.hpp>

#include <string>
#include <cstring>
#include <typeinfo>

namespace xy
{
    /*!
    \brief Uniform Buffer class.
    Uniform buffers provide a single point of access for updating
    multiple uniforms, used by multple shaders or materials. The class
    is templated to allow the use of custom structs of data to be sent
    to a shader. Valid data types can be found in the 
    <a href="https://www.opengl.org/wiki/Interface_Block_(GLSL)">OpenGL
    Documentation</a>. The UniformBuffer class assumes the use of std140
    layout in shaders to which they are bound.
    */
    class XY_EXPORT_API UniformBuffer final
    {
    public:
        /*!
        \brief Constructor
        \param string The name of the UBO as it appears in the shader
        with which the UniformBuffer is associated.
        */
        UniformBuffer(const std::string&);
        ~UniformBuffer();
        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator = (const UniformBuffer&) = delete;

        /*!
        \brief Creates (or re-creates) the buffer with the given data.
        \param data Struct of data matching uniform block in shader.
        To update an existing UBO use the update() function rather than
        recreating the UniformBuffer each time.
        */
        template <typename T>
        bool create(const T& data)
        {
            if (m_index > 0)
            {
                if (m_id)
                {
                    glCheck(glDeleteBuffers(1, &m_id));
                    m_id = 0;
                    m_typeIndex = 0;
                }
                glCheck(glGenBuffers(1, &m_id));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
                glCheck(glBufferData(GL_UNIFORM_BUFFER, sizeof(T), &data, GL_DYNAMIC_DRAW));
                glCheck(glBindBuffer(GL_UNIFORM_BUFFER, 0));

                if (glGetError() == GL_NO_ERROR)
                {
                    m_typeIndex = typeid(T).hash_code();
                    return true;
                }
                return false;
            }

            Logger::log("No free UBO indices available, " + m_name + " has not been created.", Logger::Type::Error);
            return false;
        }

        /*!
        \brief Updates the UniformBuffer
        \param data Struct of data to upload to buffer
        */
        template <typename T>
        void update(const T& data)
        {
            XY_ASSERT(m_id, "No valid UBO ID found");
            {
                XY_ASSERT(m_typeIndex == typeid(T).hash_code(), "Cannot update UBO with data of this type");
                {
                    glCheck(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
                    GLvoid* ptr;
                    glCheck(ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
                    std::memcpy(ptr, &data, sizeof(T));
                    glCheck(glUnmapBuffer(GL_UNIFORM_BUFFER));
                }
            }
        }

        /*!
        \brief Binds the UniformBuffer to the provided Material for drawing
        */
        void bind(ShaderID, UniformBlockID) const;

        /*!
        \brief Returns the name of the UniformBuffer
        */
        const std::string& getName() const { return m_name; }

    private:
        std::string m_name;
        UniformBufferID m_id;
        GLint m_index;
        std::size_t m_typeIndex;
    };
}

#endif //XY_UNIFORM_BUFFER_HPP_
