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

#ifndef XY_VERT_ATTRIB_HPP_
#define XY_VERT_ATTRIB_HPP_

#include <xygine/Config.hpp>
#include <xygine/detail/Aliases.hpp>
#include <xygine/detail/GLExtensions.hpp>

#include <unordered_map>
#include <string>
#include <vector>

namespace xy
{
    using AttribList = std::unordered_map<std::string, VertexAttribID>;

    class Mesh;
    class Material;
    class XY_EXPORT_API VertexAttribBinding final
    {
    public:
        VertexAttribBinding(const Mesh&, const Material&);
        ~VertexAttribBinding();
        VertexAttribBinding(const VertexAttribBinding&) = delete;
        VertexAttribBinding& operator = (const VertexAttribBinding&) = delete;
        VertexAttribBinding(VertexAttribBinding&&) noexcept = default;
        VertexAttribBinding& operator = (VertexAttribBinding&&) = default;

        void bind() const;
        void unbind() const;

    private:

        GLuint m_id;

        VertexBufferID m_meshBuffer;
        struct VertexAttrib final
        {
            VertexAttrib()
                : enabled(false), size(4), type(GL_FLOAT), normalised(GL_FALSE), stride(0u), ptr(nullptr) {}
            bool enabled;
            GLint size;
            GLenum type;
            GLboolean normalised;
            GLuint stride;
            void* ptr;
        };
        std::vector<VertexAttrib> m_vertexAttribs;
    };
}

#endif //XY_VERT_ATTRIB_HPP_
