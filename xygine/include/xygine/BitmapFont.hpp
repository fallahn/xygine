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

#ifndef XY_BFONT_HPP_
#define XY_BFONT_HPP_

#include <xygine/Config.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace sf
{
    class Texture;
}

namespace xy
{
    /*!
    \brief Bitmap font class.
    Bitmap fonts offer simplified text representation loaded from a single 
    texture atlas. They assume an ASCII only charset starting at 0x20, with
    the option of using extended characters up to 0xFF. These fonts are used
    in much the same way as sf::Font, along with xy::BitmapText, with similar
    rules of lifetime applying. Generally speaking BitmapFont and BitmapText
    are used in cases where a specific look is required, and the use of the
    SFML text drawables is preferred for most text.
    */
    class XY_EXPORT_API BitmapFont final
    {
    public:
        /*!
        \brief Constructor
        \param Texture containing the atlas of characters in the font
        \param vector containing the size of a glyph/character within the atlas
        */
        BitmapFont(const sf::Texture&, sf::Vector2f);
        ~BitmapFont() = default;

        BitmapFont(const BitmapFont&) = delete;
        const BitmapFont& operator = (const BitmapFont&) = delete;

        /*!
        \brief returns the glyph representing the given char within the font (assuming an ASCII charset)
        */
        sf::FloatRect getGlyph(char) const;
        /*!
        \brief returns a reference to the texture used by the font
        */
        const sf::Texture& getTexture() const { return m_texture; }

    private:
        const sf::Texture& m_texture;
        sf::Vector2f m_glyphSize;
        sf::Vector2f m_glyphCount;
    };
}

#endif //XY_BFONT_HPP_