/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include <SFML/System/String.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <vector>

namespace sf
{
    class Font;
}

namespace xy
{
    /*!
    \brief ECS friendly implementation of Text.
    Text components should appear on entities which
    also have a transform and drawable component. Text
    is drawn with a RenderSystem. NOTE As text is rendered
    via a drawable component in the same way as sprites and other
    drawables, the drawable component should use setDepth() to
    increase the depth value of a text renderable so that it
    appears above other drawables. This should be the first
    thing to check if text appears 'invisible'.
    */
    class XY_EXPORT_API Text final
    {
    public:
        /*!
        \brief Default constructor
        */
        Text();
        /*!
        \brief Construct an instance with a given font
        */
        Text(const sf::Font&);

        /*!
        \brief Set the font used with this text
        */
        void setFont(const sf::Font&);

        /*!
        \brief Set the character size of the text
        */
        void setCharacterSize(sf::Uint32);

        /*!
        \brief Sets the vertical spacing between rows of text
        */
        void setVerticalSpacing(float);

        /*!
        \brief Set the string to be rendered by the text
        */
        void setString(const sf::String&);

        /*!
        \brief Set the fill colour of the text
        */
        void setFillColour(sf::Color);

        /*!
        \brief Set the shader to be applied when rendering this text.
        Passing nullptr removes any active shader.
        */
        void setShader(sf::Shader*);

        /*!
        \brief Sets the blend mode used when rendering this text.
        Defaults to sf::BlendAlpha
        */
        void setBlendMode(sf::BlendMode);

        /*!
        \brief Return a pointer to the active font
        */
        const sf::Font* getFont() const;

        /*!
        \brief Return the current character size of the text
        */
        sf::Uint32 getCharacterSize() const;

        /*!
        \brief Retrurns the current vertical spacing between text rows
        */
        float getVerticalSpacing() const;

        /*!
        \brief Return the current string rendered by the text
        */
        const sf::String& getString() const;

        /*!
        \brief Return the current fill colour of the text
        */
        sf::Color getFillColour() const;

        /*!
        \brief Returns a pointer to this text's active shader.
        May be nullptr.
        */
        const sf::Shader* getShader() const;

        /*!
        \brief Returns this text's current blend mode
        */
        sf::BlendMode getBlendMode() const;

        /*!
        \brief Returns the local (pre-transform) AABB of the text
        */
        sf::FloatRect getLocalBounds() const;

        /*!
        \brief Set an area to which to crop the text.
        The given rectangle should be in local coordinates, relative to
        the text.
        */
        void setCroppingArea(sf::FloatRect);

        /*!
        \brief Returns the current cropping area
        */
        sf::FloatRect getCroppingArea() const { return /*m_croppingArea*/{}; }

        enum class Alignment
        {
            Left, Right, Centre
        };
        
        /*!
        \brief Sets whether the text should be aligned
        left, right or centrally about the origin.
        Only affects the X axis
        */
        void setAlignment(Alignment);

        /*!
        \brief Returns the text's current alignment
        */
        Alignment getAlignment() const { return m_alignment; }

    private:
        
        void updateVertices(std::vector<sf::Vertex>&, sf::FloatRect&);
        void addQuad(sf::Vector2f position, const sf::Glyph& glyph, std::vector<sf::Vertex>&);

        sf::String m_string;
        const sf::Font* m_font;
        sf::Uint32 m_charSize;
        float m_verticalSpacing;
        sf::Color m_fillColour;
        //std::vector<sf::Vertex> m_vertices;
        //mutable sf::FloatRect m_localBounds;
        //sf::FloatRect m_globalBounds;
        bool m_dirty;
        //sf::RenderStates m_states;
        Alignment m_alignment;

        /*sf::FloatRect m_croppingArea;
        sf::FloatRect m_croppingWorldArea;
        bool m_cropped;*/

        friend class TextSystem;
    };
}
