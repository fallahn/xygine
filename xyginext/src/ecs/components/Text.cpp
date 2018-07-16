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

#include <SFML/Graphics/Font.hpp>

#include "xyginext/ecs/components/Text.hpp"

using namespace xy;

Text::Text()
    : m_font            (nullptr),
    m_charSize          (30),
    m_verticalSpacing   (0.f),
    m_fillColour        (sf::Color::White),
    m_dirty             (true),
    m_alignment         (Alignment::Left),
    m_croppingArea      (-DefaultSceneSize / 2.f, DefaultSceneSize * 2.f),
    m_cropped           (false)
{

}

Text::Text(const sf::Font& font)
    : m_font            (nullptr),
    m_charSize          (30),
    m_verticalSpacing   (0.f),
    m_fillColour        (sf::Color::White),
    m_dirty             (true),
    m_alignment         (Alignment::Left),
    m_croppingArea      (-DefaultSceneSize / 2.f, DefaultSceneSize * 2.f),
    m_cropped           (false)
{
    setFont(font);
}

//public
void Text::setFont(const sf::Font& font)
{
    m_font = &font;
    m_dirty = true;
}

void Text::setCharacterSize(sf::Uint32 size)
{
    m_charSize = size;
    m_dirty = true;
}

void Text::setVerticalSpacing(float spacing)
{
    m_verticalSpacing = spacing;
    m_dirty = true;
}

void Text::setString(const sf::String& str)
{
    if (m_string != str)
    {
        m_string = str;
        m_dirty = true;
    }
}

void Text::setFillColour(sf::Color colour)
{
    m_fillColour = colour;
    if (!m_dirty)
    {
        for (auto& v : m_vertices)
        {
            v.color = colour;
        }
        return;
    }
}

void Text::setShader(sf::Shader* shader)
{
    m_states.shader = shader;
}

void Text::setBlendMode(sf::BlendMode mode)
{
    m_states.blendMode = mode;
}

const sf::Font* Text::getFont() const
{
    return m_font;
}

sf::Uint32 Text::getCharacterSize() const
{
    return m_charSize;
}

float Text::getVerticalSpacing() const
{
    return m_verticalSpacing;
}

const sf::String& Text::getString() const
{
    return m_string;
}

sf::Color Text::getFillColour() const
{
    return m_fillColour;
}

const sf::Shader* Text::getShader() const
{
    return m_states.shader;
}

sf::BlendMode Text::getBlendMode() const
{
    return m_states.blendMode;
}

sf::FloatRect Text::getLocalBounds() const
{
    if (m_dirty)
    {
        // Calculate local bounds if needed.
        // Mostly a copy from xy::TextRenderer, but other solutions would
        // involve changing the add component flow
        
        float xOffset = static_cast<float>(m_font->getGlyph(L' ', m_charSize, false).advance);
        float yOffset = static_cast<float>(m_font->getLineSpacing(m_charSize));
        
        float x = 0.f;
        float y = static_cast<float>(m_charSize);
        
        float minX = x;
        float minY = y;
        float maxX = 0.f;
        float maxY = 0.f;
        sf::Uint32 prevChar = 0;
        for (auto i = 0u; i < m_string.getSize(); ++i)
        {
            sf::Uint32 currChar = m_string[i];
            
            x += m_font->getKerning(prevChar, currChar, m_charSize);
            prevChar = currChar;
            
            //whitespace chars
            if (currChar == ' ' || currChar == '\t' || currChar == '\n')
            {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                
                switch (currChar)
                {
                    default: break;
                    case ' ':
                        x += xOffset;
                        break;
                    case '\t':
                        x += xOffset * 4.f; //4 spaces for tab suckas
                        break;
                    case '\n':
                        y += yOffset + m_verticalSpacing;
                        x = 0.f;
                        break;
                }
                
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
                
                continue; //skip quad for whitespace
            }
            
            const auto& glyph = m_font->getGlyph(currChar, m_charSize, false);
            
            float left = glyph.bounds.left;
            float top = glyph.bounds.top;
            float right = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top + glyph.bounds.height;
            
            minX = std::min(minX, x + left);
            maxX = std::max(maxX, x + right);
            minY = std::min(minY, y + top);
            maxY = std::max(maxY, y + bottom);
            
            x += glyph.advance;
        }
        
        m_localBounds.left = minX;
        m_localBounds.top = minY;
        m_localBounds.width = maxX - minX;
        m_localBounds.height = maxY - minY;
    }

    return m_localBounds;
}

void Text::setCroppingArea(sf::FloatRect area)
{
    m_croppingArea = area;
    m_dirty = true;
}

void Text::setAlignment(Text::Alignment alignment)
{
    m_alignment = alignment;
    m_dirty = true;
}

//private

