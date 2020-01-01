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

#include "xyginext/graphics/BitmapFont.hpp"
#include "xyginext/core/Assert.hpp"

using namespace xy;

namespace
{
    const int CountX = 10;
    const int CountY = 10;
}

BitmapFont::BitmapFont()
{

}

BitmapFont::BitmapFont(const std::string& path)
{
    loadTextureFromFile(path);
}

//public
bool BitmapFont::loadTextureFromFile(const std::string& path)
{
    if (!m_texture.loadFromFile(path))
    {
        return false;
    }

    m_textureSize = sf::Vector2f(m_texture.getSize());
    m_charSize = { m_textureSize.x / CountX, m_textureSize.y / CountY };
    return true;
}

sf::FloatRect BitmapFont::getGlyph(char character) const
{
    XY_ASSERT(character > 31 && character < 127, "character out of range");

    char c = character - 32;
    int xIndex = c % CountX;
    int yIndex = c / CountX;

    return { xIndex * m_charSize.x, yIndex * m_charSize.y, m_charSize.x, m_charSize.y };
}