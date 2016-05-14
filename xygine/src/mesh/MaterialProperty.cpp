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

#include <SFML/Graphics/Shader.hpp>

using namespace xy;

Material::Property::Property(const std::string& name)
    : m_type    (Type::None),
    m_name      (name)
{

}

//public
void Material::Property::setValue(float val)
{
    m_type = Type::Float;
    number = val;
}

void Material::Property::setValue(const sf::Vector2f& val)
{
    m_type = Type::Vec2;
    vec2 = val;
}

void Material::Property::setValue(const sf::Vector3f& val)
{
    m_type = Type::Vec3;
    vec3 = val;
}

void Material::Property::setValue(const sf::Color& val)
{
    m_type = Type::Vec4;
    colour = val;
}

void Material::Property::setValue(const sf::Transform& val)
{
    m_type = Type::Mat4;
    transform = &val;
}

void Material::Property::setValue(const sf::Texture& val)
{
    m_type = Type::Texture;
    texture = &val;
}

//private
void Material::Property::apply(sf::Shader& shader) const
{
    switch (m_type)
    {
    default:
    case Type::None: break;
    case Type::Float:
        shader.setUniform(m_name, number);
        break;
    case Type::Vec2:
        shader.setUniform(m_name, vec2);
        break;
    case Type::Vec3:
        shader.setUniform(m_name, vec3);
        break;
    case Type::Vec4:
        shader.setUniform(m_name, sf::Glsl::Vec4(colour));
        break;
    case Type::Mat3:

        break;
    case Type::Mat4:
        shader.setUniform(m_name, sf::Glsl::Mat4(*transform));
        break;
    case Type::Texture:
        shader.setUniform(m_name, *texture);
        break;
    }
}