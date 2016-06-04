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

#ifndef XY_MATERIAL_PROPERTY_HPP_
#define XY_MATERIAL_PROPERTY_HPP_

#include <xygine/Config.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <string>

namespace sf
{
    class Shader;
    class Texture;
}

namespace xy
{
    /*!
    \brief Defines a property of a Material, updated by 
    the shader uniforms of the material's current RenderPass.
    */

    class XY_EXPORT_API MaterialProperty final
    {
        friend class RenderPass;
    public:
        MaterialProperty(const std::string&, float);
        MaterialProperty(const std::string&, const sf::Vector2f&);
        MaterialProperty(const std::string&, const sf::Vector3f&);
        MaterialProperty(const std::string&, const sf::Color&);
        MaterialProperty(const std::string&, const sf::Transform&);
        MaterialProperty(const std::string&, const sf::Texture&);
        MaterialProperty(const std::string&, const float*, std::size_t);
        ~MaterialProperty() = default;

        //MaterialProperty(MaterialProperty&&) noexcept = default;

        void setValue(float);
        void setValue(const sf::Vector2f&);
        void setValue(const sf::Vector3f&);
        void setValue(const sf::Color&);
        void setValue(const sf::Transform&);
        void setValue(const sf::Texture&);
        void setValue(const float*, std::size_t);
        const std::string& getName() const { return m_name; }
    private:
        enum class Type
        {
            None,
            Float,
            Vec2,
            Vec3,
            Vec4,
            Mat3,
            Mat4,
            Mat4Array,
            Texture
        }m_type;

        union
        {
            float number;
            sf::Vector2f vec2;
            sf::Vector3f vec3;
            sf::Color colour;
            const sf::Transform* transform;
            const sf::Texture* texture;
            const float* floatArray;
        };
        std::string m_name;
        UniformID m_uid;
        std::size_t m_size;
        void apply(sf::Shader&) const;
    };
}

#endif //XY_MATERIAL_PROPERTY_HPP_