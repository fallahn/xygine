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

#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/UniformBuffer.hpp>

#include <SFML/Graphics/Shader.hpp>

using namespace xy;

Material::Material(sf::Shader& shader)
    : m_shader      (shader)
{
    XY_ASSERT(shader.getNativeHandle(), "Must compile shader first!");
    
    //look up the shader attributes and store them
    GLint activeAttribs;
    glCheck(glGetProgramiv(shader.getNativeHandle(), GL_ACTIVE_ATTRIBUTES, &activeAttribs));
    if (activeAttribs > 0)
    {
        GLint length;
        glCheck(glGetProgramiv(shader.getNativeHandle(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &length));
        if (length > 0)
        {
            std::vector<GLchar> attribName(length + 1);
            GLint attribSize;
            GLenum attribType;
            GLint attribLocation;

            for (auto i = 0; i < activeAttribs; ++i)
            {
                glCheck(glGetActiveAttrib(shader.getNativeHandle(), i, length, nullptr, &attribSize, &attribType, attribName.data()));
                attribName[length] = '\0';

                glCheck(attribLocation = glGetAttribLocation(shader.getNativeHandle(), attribName.data()));
                m_vertexAttributes.insert(std::make_pair(attribName.data(), attribLocation));
            }
        }
    }
}

//public
void Material::bind() const
{
    for (const auto& p : m_properties)
    {
        p.apply(m_shader);
    }

    for (auto & ubo : m_uniformBuffers)
    {
        ubo.second->bind(m_shader.getNativeHandle(), ubo.first);
    }
    sf::Shader::bind(&m_shader);
}

void Material::addProperty(const Material::Property& prop)
{
    m_properties.push_back(prop);
}

Material::Property* Material::getProperty(const std::string& name)
{
    auto result = std::find_if(m_properties.begin(), m_properties.end(),
        [&name](const Material::Property& prop)
    {
        return (prop.getName() == name);
    });

    if ( result != m_properties.end())
    {
        return &(*result);
    }
    return nullptr;
}

bool Material::addUniformBuffer(const UniformBuffer& ubo)
{
    //look up the block name in the shader and add the UBO if it exists
    UniformBlockID id;
    glCheck(id = glGetUniformBlockIndex(m_shader.getNativeHandle(), ubo.getName().c_str()));
    if (id != GL_INVALID_INDEX)
    {
        m_uniformBuffers.emplace_back(std::make_pair(id, &ubo));
        return true;
    }
    LOG("Uniform block " + ubo.getName() + " not found in Material", xy::Logger::Type::Error);
    return false;
}

VertexAttribID Material::getVertexAttributeID(const std::string& str) const
{
    auto result = m_vertexAttributes.find(str);
    return (result == m_vertexAttributes.end()) ? -1 : result->second;
}