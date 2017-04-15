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

#include <xygine/BitmapFont.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/Texture.hpp>

#include <cmath>

using namespace xy;

BitmapFont::BitmapFont(const sf::Texture& texture, sf::Vector2f glyphSize)
    :m_texture  (texture),
    m_glyphSize (glyphSize)
{
    XY_ASSERT(glyphSize.x > 0 && glyphSize.y > 0, "must have positive glyph size");

    auto size = static_cast<sf::Vector2f>(texture.getSize());
    m_glyphCount.x = size.x / glyphSize.x;
    m_glyphCount.y = size.y / glyphSize.y;
}

//public
sf::FloatRect BitmapFont::getGlyph(char c) const
{
    //XY_ASSERT(c > 31, "character not supported");
    c = (c > 31) ? c - 32 : 0; //starts with SPACE char

    auto posX = static_cast<float>(c % static_cast<char>(m_glyphCount.x));
    auto posY = std::floor(c / m_glyphCount.x);

    return{ posX * m_glyphSize.x, posY * m_glyphSize.y, m_glyphSize.x, m_glyphSize.y };
}