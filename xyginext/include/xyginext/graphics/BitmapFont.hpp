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

#include <SFML/Graphics/Texture.hpp>

namespace xy
{
    /*!
    \brief Bitmap font class.
    Bitmap fonts are created by loading a font atlas with the following layout:
    10 characters wide, by 10 characters high, starting with ASCII character 32
    (space) and ending with ASCII character 126 (tilde). Some or all of the characters
    may be implemented but unimplemented characters should be left blank. As
    bitmap fonts are very simplistic anything other than ASCII encoding is not
    supported.

    Bitmap fonts are used with the BitmapText and BitmapTextSystem classes. As
    BitmapFonts contain their own texture instances they must live at least as
    long as any BitmapText instances which use them, and should be resource managed
    as regular fonts, preferably with xy::ResourceHandler
    \see BitmapText
    */

    class XY_EXPORT_API BitmapFont final
    {
    public:
        BitmapFont();

        /*!
        \brief Constructor
        \param path String containing the path to a texture
        atlas containing the bitmap font.
        */
        explicit BitmapFont(const std::string& path);

        /*!
        \brief Load a texture from file to use for this font.
        \returns True on success else false on failure
        */
        bool loadTextureFromFile(const std::string&);

        /*!
        \brief returns the texture rect for the requested
        character
        */
        sf::FloatRect getGlyph(char) const;

        /*!
        \brief Returns a pointer to the loaded texture
        */
        const sf::Texture* getTexture() const { return &m_texture; }

        /*!
        \brief Returns the width / height of a character
        */
        sf::Vector2f getCharacterSize() const { return m_charSize; }

    private:
        sf::Texture m_texture;
        sf::Vector2f m_textureSize;
        sf::Vector2f m_charSize;
    };
}