/*********************************************************************
(c) Matt Marchant 2017 - 2019
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/Config.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/System/Vector3.hpp>

#include <vector>
#include <string>
#include <array>

namespace xy
{
    /*!
    \brief Drawable component.
    The drawable component encapsulates an sf::VertexArray which can be used to draw
    custom shapes, as well as being required for entities which have a sprite or text
    component. The purpose of the Drawable component is to allow mixing Sprite, Text
    and custom drawable types in a single drawing pass with variable depth. A Scene
    must have a RenderSystem added to it to enable any drawable entities.
    */
    class XY_EXPORT_API Drawable final : public sf::Drawable
    {
    public:
        Drawable();
        explicit Drawable(const sf::Texture&);
        
        /*!
        \brief Sets the texture with which to render this drawable.
        Setting this to nullptr removes the texture. This has no effect
        when used with Sprite or Text components which supply their own
        textures.
        \see Sprite::setTexture()
        */
        void setTexture(const sf::Texture*);

        /*!
        \brief Sets the shader used when drawing.
        Passing nullptr removes any active shader.
        */
        void setShader(sf::Shader*);

        /*!
        \brief Adds a uniform binding to be applied to any shader this
        drawable may have.
        When sharing a shader between multiple drawables it may be, for
        instance, desirable to the apply a different texture for each drawble.
        This function allows mapping a uniform name (assuming it is available
        in the current shader) to a texture or other value.
        Bindings are limited to 6 per type.
        */
        void bindUniform(const std::string& name, const sf::Texture& texture);

        void bindUniform(const std::string& name, float value);

        void bindUniform(const std::string& name, sf::Vector2f value);

        void bindUniform(const std::string& name, sf::Vector3f value);

        void bindUniform(const std::string& name, bool value);

        void bindUniform(const std::string& name, sf::Color colour);

        /*!
        \brief Binds a pointer to a float array containing a 4x4 matrix
        */
        void bindUniform(const std::string& name, const float* value);

        /*!
        \brief Binds the given uniform to the value of sf::Shader::CurrentTexture.
        Up to 6 uniforms may be bound to this value.
        */
        void bindUniformToCurrentTexture(const std::string& name);

        /*!
        \brief Sets the blend mode used when drawing
        */
        void setBlendMode(sf::BlendMode);

        /*!
        \brief Set filter flags for rendering.
        Filter flags allow certain drawables to be omitted from a drawing
        pass by the render system. This is useful for multipass effects which
        require rendering different entities to different render buffers.
        For example lighting objects may be flagged as such so that
        when the lighting only flag is active on the render system only
        illuminating objects are rendered to the output.
        Flags are bitwise values so that drawables can be categorised in
        multiple filters by setting the appropriate flags. The default value
        is Uin64::max - ie all flags set (so will always pass the render filter).
        \param flags a bitmask of filter flags
        */
        void setFilterFlags(std::uint64_t flags) { m_filterFlags = flags; }

        /*!
        \brief Returns the current filter flags of this drawable.
        \see setFilterFlags()
        */
        std::uint64_t getFilterFlags() const { return m_filterFlags; }

        /*!
        \brief Returns a pointer to the active texture
        */
        sf::Texture* getTexture();

        /*!
        \brief Returns a pointer to the active shader, if any
        */
        sf::Shader* getShader();

        /*!
        \brief Returns the current blend mode
        */
        sf::BlendMode getBlendMode() const { return m_states.blendMode; }

        /*!
        \brief Sets the z-depth of a drawable.
        The lower the value the further back the entity is drawn,
        inversely the greater the value the further forward it is drawn.
        Default value is 0.
        */
        void setDepth(sf::Int32 depth);

        /*!
        \brief Returns the Z depth value
        \see setDepth();
        */
        sf::Int32 getDepth() const { return m_zDepth; }

        /*!
        \brief Set an area to which to crop the drawable.
        The given rectangle should be in local coordinates, relative to
        the drawable.
        */
        void setCroppingArea(sf::FloatRect);

        /*!
        \brief Returns the current cropping area
        */
        sf::FloatRect getCroppingArea() const { return m_croppingArea; }

        /*!
        \brief Returns a reference to the vertex array used when drawing.
        */
        std::vector<sf::Vertex>& getVertices() { return m_vertices; }
        const std::vector<sf::Vertex>& getVertices() const { return m_vertices; }

        /*!
        \brief Sets the PrimitiveType used by the drawable.
        Uses sf::Quads by default.
        */
        void setPrimitiveType(sf::PrimitiveType type) { m_primitiveType = type; }

        /*!
        \brief Returns the current PrimitiveType used to draw the vertices
        */
        sf::PrimitiveType getPrimitiveType() const { return m_primitiveType; }

        /*!
        \brief Returns the local bounds of the Drawable's vertex array.
        This should be updated by any systems which implement custom drawables
        else culling will failed and drawable will not appear on screen
        */
        sf::FloatRect getLocalBounds() const;

        /*!
        \brief Updates the local bounds.
        This should be called once by a system when it updates the vertex array.
        As this is used by the render system for culling, Drawable components
        will not be drawn if the bounds have not been updated.
        */
        void updateLocalBounds();
        void updateLocalBounds(sf::FloatRect);

        /*!
        \brief Enables or disables viewport culling.
        By default Drawables are culled from rendering when not in the
        viewable area of the active camera. Setting this to true will cause
        the drawable to always be rendered, even if it falls outside the active
        camera's view.
        \param cull Set to true to have the drawble culled from rendering when
        not intersecting the current viewable area.
        */
        void setCulled(bool cull) { m_cull = cull; }

        /*!
        \brief Returns the RenderStates containing the current blend mode,
        PrimitiveType and Shader of the drawable.
        Generally not required, unless implementing a custom renderer.
        */
        sf::RenderStates getStates() const;

        /*!
        \brief Applies all the unform bindings to the current shader.
        Generally not used unless implementing a custom render system,
        in which case this should be called immediately before the
        component is drawn.
        */
        void applyShader() const;

    private:
        sf::PrimitiveType m_primitiveType = sf::Quads;
        sf::RenderStates m_states;
        std::vector<sf::Vertex> m_vertices;

        sf::Int32 m_zDepth = 0;
        bool m_wantsSorting = true;

        std::uint64_t m_filterFlags;

        sf::FloatRect m_localBounds;

        static constexpr std::size_t MaxBindings = 6;
        std::array<std::pair<std::string, const sf::Texture*>, MaxBindings> m_textureBindings;
        std::array<std::pair<std::string, float>, MaxBindings> m_floatBindings;
        std::array<std::pair<std::string, sf::Vector2f>, MaxBindings> m_vec2Bindings;
        std::array<std::pair<std::string, sf::Vector3f>, MaxBindings> m_vec3Bindings;
        std::array<std::pair<std::string, bool>, MaxBindings> m_boolBindings;
        std::array<std::pair<std::string, sf::Glsl::Vec4>, MaxBindings> m_colourBindings;
        std::array<std::pair<std::string, const float*>, MaxBindings> m_matBindings;
        std::array<std::string, MaxBindings> m_currentTexBindings;
        std::size_t m_textureCount;
        std::size_t m_floatCount;
        std::size_t m_vec2Count;
        std::size_t m_vec3Count;
        std::size_t m_boolCount;
        std::size_t m_colourCount;
        std::size_t m_matCount;
        std::size_t m_currentTexCount;

        bool m_cull;

        sf::FloatRect m_croppingArea;
        sf::FloatRect m_croppingWorldArea;
        bool m_cropped;

        friend class RenderSystem;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}
