/*********************************************************************
(c) Matt Marchant 2017 - 2020
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
#include "xyginext/ecs/Entity.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <string>

namespace xy
{
    class BitmapFont;
    class Drawable;

    /*!
    \brief BitmapText class.
    BitmapText components are used to render text with a BitmapFont.
    BitmapText work similarly to Text components, in that they require
    a reference to a BitmapFont that exists at least as long as the
    BitmapText instance. Entities with a BitmapText component also
    require a Drawable component and a Transform component.

    BitmapText components also require a BitmapTextSystem active in
    the current scene to render them.
    \see BitmapFont
    */
    class XY_EXPORT_API BitmapText final
    {
    public:
        /*!
        \brief Default constructor
        */
        BitmapText();

        /*!
        \brief Constructs a BitmapText component
        with the given BitmapFont
        */
        explicit BitmapText(const BitmapFont&);

        /*!
        \brief Sets the font used with this text
        */
        void setFont(const BitmapFont&);

        /*
        \brief Set the string to render with this text.
        Note that BitmapFonts only support ASCII characters.
        */
        void setString(const std::string&);

        /*!
        \brief Set the text colour.
        As with other drawable items, this colour is multiplied
        with the base colour of the texture used by the current font.
        */
        void setColour(sf::Color);

        /*!
        \brief Returns a pointer to the currently active font
        */
        const BitmapFont* getFont() const;

        /*!
        \brief Returns a reference to the active string used
        by the BitmapText component.
        */
        const std::string& getString() const;

        /*!
        \brief Returns the current colour of the BitmapText
        */
        sf::Color getColour() const;

        /*!
        \brief Helper function to get the local bounds of the
        BitmapText.
        The passed entity should have at least a BitmapText
        component and Drawable component attached to it.
        */
        static sf::FloatRect getLocalBounds(Entity);

    private:

        void updateVertices(Drawable&);

        std::string m_string;
        const BitmapFont* m_font;
        sf::Color m_colour;

        bool m_dirty;

        friend class BitmapTextSystem;
    };
}
