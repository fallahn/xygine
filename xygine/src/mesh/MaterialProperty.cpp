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

#include <xygine/mesh/MaterialProperty.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/Shader.hpp>

using namespace xy;

MaterialProperty::MaterialProperty(const std::string& name, float val)
    : m_type    (Type::Float),
    number      (val),
    m_name      (name),
    m_uid       (-1),
    m_size(0){}

MaterialProperty::MaterialProperty(const std::string& name, const sf::Vector2f& val)
    : m_type    (Type::Vec2),
    vec2        (val),
    m_name      (name),
    m_uid       (-1),
    m_size(0) {}

MaterialProperty::MaterialProperty(const std::string& name, const sf::Vector3f& val)
    : m_type    (Type::Vec3),
    vec3        (val),
    m_name      (name),
    m_uid       (-1),
    m_size(0) {}

MaterialProperty::MaterialProperty(const std::string& name, const sf::Color& val)
    : m_type    (Type::Vec4),
    colour      (val),
    m_name      (name),
    m_uid       (-1),
    m_size(0) {}


MaterialProperty::MaterialProperty(const std::string& name, const sf::Transform& val)
    : m_type    (Type::Mat4),
    transform   (&val),
    m_name      (name),
    m_uid       (-1),
    m_size(0) {}

MaterialProperty::MaterialProperty(const std::string& name, const sf::Texture& val)
    : m_type    (Type::Texture),
    texture     (&val),
    m_name      (name),
    m_uid       (-1),
    m_size(1) {}

MaterialProperty::MaterialProperty(const std::string& name, const float* data, std::size_t size)
    : m_type    (Type::Mat4Array),
    floatArray  (data),
    m_name      (name),
    m_uid       (-1),
    m_size      (size) {}

//public
void MaterialProperty::setValue(float val)
{
    m_type = Type::Float;
    number = val;
}

void MaterialProperty::setValue(const sf::Vector2f& val)
{
    m_type = Type::Vec2;
    vec2 = val;
}

void MaterialProperty::setValue(const sf::Vector3f& val)
{
    m_type = Type::Vec3;
    vec3 = val;
}

void MaterialProperty::setValue(const sf::Color& val)
{
    m_type = Type::Vec4;
    colour = val;
}

void MaterialProperty::setValue(const sf::Transform& val)
{
    m_type = Type::Mat4;
    transform = &val;
}

void MaterialProperty::setValue(const sf::Texture& val)
{
    m_type = Type::Texture;
    texture = &val;
}

void MaterialProperty::setValue(const float* value, std::size_t size)
{
    XY_ASSERT(value, "Value must not be nullptr");
    m_type = Type::Mat4Array;
    floatArray = value;
    m_size = size;
}

//private
void MaterialProperty::apply(sf::Shader& shader) const
{
    switch (m_type)
    {
    default:
    case Type::None: break;
    case Type::Float:
        glCheck(glUniform1f(m_uid, number));
        break;
    case Type::Vec2:
        glCheck(glUniform2f(m_uid, vec2.x, vec2.y));
        break;
    case Type::Vec3:
        glCheck(glUniform3f(m_uid, vec3.x, vec3.y, vec3.z));
        break;
    case Type::Vec4:
        glCheck(glUniform4f(m_uid, static_cast<float>(colour.r) / 255.f, static_cast<float>(colour.g) / 255.f,
                                    static_cast<float>(colour.b) / 255.f, static_cast<float>(colour.a) / 255.f));
        break;
    case Type::Mat3:

        break;
    case Type::Mat4:
        glCheck(glUniformMatrix4fv(m_uid, m_size, GL_FALSE, sf::Glsl::Mat4(*transform).array));
        break;
    case Type::Texture:
        shader.setUniform(m_name, *texture);
        break;
    case Type::Mat4Array:
        glCheck(glUniformMatrix4fv(m_uid, m_size, GL_FALSE, floatArray));
        break;
    }
}