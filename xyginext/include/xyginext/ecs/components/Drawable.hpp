/*********************************************************************
(c) Matt Marchant 2017
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

#ifndef XY_DRAWABLE_HPP_
#define XY_DRAWABLE_HPP_

#include <xyginext/Config.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

#include <vector>

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
    class XY_EXPORT_API Drawable final
    {
    public:
        Drawable() = default;
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
        \brief Sets the blend mode used when drawing
        */
        void setBlendMode(sf::BlendMode);

        /*!
        \brief Returns a pointer to the active texture
        */
        const sf::Texture* getTexture() const;

        /*!
        \brief Returns a pointer to the active shader, if any
        */
        const sf::Shader* getShader() const;

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
        void setDepth(sf::Int32 depth) { m_zDepth = depth; m_wantsSorting = true; }

        /*!
        \brief Returns the Z depth value
        \see setDepth();
        */
        sf::Int32 getDepth() const { return m_zDepth; }

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
        This should be called once by a system when it updates the vertex array
        */
        void updateLocalBounds();

    private:
        sf::PrimitiveType m_primitiveType = sf::Quads;
        sf::RenderStates m_states;
        std::vector<sf::Vertex> m_vertices;

        sf::Int32 m_zDepth = 0;
        bool m_wantsSorting = true;

        sf::FloatRect m_localBounds;

        friend class RenderSystem;
    };
}

#endif //XY_DRAWABLE_HPP_