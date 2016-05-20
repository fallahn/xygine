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

#ifndef XY_MATERIAL_HPP_
#define XY_MATERIAL_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/Glsl.hpp>

#include <string>
#include <vector>
#include <unordered_map>

namespace sf
{
    class Shader;
    class Texture;
}

namespace xy
{
    class UniformBuffer;

    /*!
    \brief Materials are used when rendering Model components.
    One or more materials can be assigned to a single Model to
    be applied to the Model's Mesh and SubMeshes. Materials contain,
    at the very least, an OpenGL 3.2+ compatible shader, as well as
    allowing specifying multiple properties (in the guise of uniform
    values) which are applied to the shader, such as textures or matrices.
    Materials may also have UniformBuffers added to them for convenience,
    and are required for the view/projection matrices at the very least
    when rendering with the MeshRenderer. The UniformBuffer for these
    (and the optional buffer for lighting properties) can be retrieved
    with MeshRenderer::getMatrixUniforms(). This should be the first thing
    to check if a material is apprently not renderering. The 
    MaterialResource class is provided as a convenience for managing the
    lifetime of Materials as the are a shared resource. Bear also in mind
    that, as a shared resource, updating the property of a Material
    instance will affect ALL models to which thie Material is applied.
    */
    class XY_EXPORT_API Material final
    {
    public:
        /*!
        Material Property.
        Wraps material specific settings which are applied to the
        material's shader via uniform values when the material is drawn.
        */
        class XY_EXPORT_API Property final
        {
            friend class Material;
        public:
            Property(const std::string&, float);
            Property(const std::string&, const sf::Vector2f&);
            Property(const std::string&, const sf::Vector3f&);
            Property(const std::string&, const sf::Color&);
            Property(const std::string&, const sf::Transform&);
            Property(const std::string&, const sf::Texture&);
            ~Property() = default;
            void setValue(float);
            void setValue(const sf::Vector2f&);
            void setValue(const sf::Vector3f&);
            void setValue(const sf::Color&);
            void setValue(const sf::Transform&);
            void setValue(const sf::Texture&);
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
            };
            std::string m_name;
            void apply(sf::Shader&) const;
        };

        explicit Material(sf::Shader&);
        ~Material() = default;

        /*!
        \brief Binds this material for drawing
        */
        void bind() const;

        /*!
        \brief Adds a new property to the Material
        */
        void addProperty(const Property&);
        /*!
        \brief Returns a Property with the given name if it
        exists, else returns nullptr
        */
        Property* getProperty(const std::string&);
        /*!
        \brief Add a UniformBuffer to be bound when drawing
        with the Material.
        UniformBuffers are useful for setting blocks of values
        used by multiple materials such as projection matrices.
        Care should be taken to make sure the UniformBuffer outlives
        the Material.
        \returns true If the UniformBuffer name was found in the shader
        and the UniformBuffer was successfully added, else returns false
        */
        bool addUniformBuffer(const UniformBuffer&);

        /*!
        \brief Returns the attribute ID of the requested vertex attribute if it
        exists in the Material's shader, else returns -1
        */
        VertexAttribID getVertexAttributeID(const std::string&) const;

        /*!
        \brief Returns a reference to the Material shader
        */
        sf::Shader& getShader() const { return m_shader; }

    private:
        sf::Shader& m_shader;

        std::vector<Property> m_properties;
        std::vector<std::pair<UniformBlockID, const UniformBuffer*>> m_uniformBuffers;

        std::unordered_map<std::string, VertexAttribID> m_vertexAttributes;
    };
}

#endif //XY_MATERIAL_HPP_
