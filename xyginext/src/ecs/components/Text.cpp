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

#include <SFML/Graphics/Font.hpp>

#include "xyginext/ecs/components/Text.hpp"
#include "xyginext/ecs/components/Drawable.hpp"

#include "xyginext/core/Log.hpp"

using namespace xy;

Text::Text()
    : m_font            (nullptr),
    m_charSize          (30),
    m_verticalSpacing   (0.f),
    m_fillColour        (sf::Color::White),
    m_outlineThickness  (0.f),
    m_dirty             (true),
    m_alignment         (Alignment::Left)
{

}

Text::Text(const sf::Font& font)
    : m_font            (nullptr),
    m_charSize          (30),
    m_verticalSpacing   (0.f),
    m_fillColour        (sf::Color::White),
    m_outlineThickness  (0.f),
    m_dirty             (true),
    m_alignment         (Alignment::Left)
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
    if (m_fillColour != colour)
    {
        //TODO rather than dirty just update the vert
        //colours if the text is not already dirty
        m_fillColour = colour;
        m_dirty = true;
    }
}

void Text::setOutlineColour(sf::Color colour)
{
    if (m_outlineColour != colour)
    {
        m_outlineColour = colour;
        m_dirty = true;
    }
}

void Text::setOutlineThickness(float thickness)
{
    if (m_outlineThickness != thickness)
    {
        m_outlineThickness = thickness;
        m_dirty = true;
    }
}

void Text::setShader(sf::Shader* shader)
{
    LOG("DEPRECATED: use Drawable::setShader() instead", xy::Logger::Type::Warning);
}

void Text::setBlendMode(sf::BlendMode mode)
{
    LOG("DEPRECATED: use Drawable::setBlendMode() instead", xy::Logger::Type::Warning);
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

sf::Color Text::getOutlineColour() const
{
    return m_outlineColour;
}

float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}

const sf::Shader* Text::getShader() const
{
    LOG("DEPRECATED: use Drawable::getShader() instead", xy::Logger::Type::Warning);
    return nullptr;
}

sf::BlendMode Text::getBlendMode() const
{
    LOG("DEPRECATED: use Drawable::getBlendMode() instead", xy::Logger::Type::Warning);
    return {};
}

sf::FloatRect Text::getLocalBounds(xy::Entity entity)
{
   XY_ASSERT(entity.hasComponent<xy::Text>() && entity.hasComponent<xy::Drawable>(), "Invalid Entity");

    auto& text = entity.getComponent<xy::Text>();
    auto& drawable = entity.getComponent<xy::Drawable>();
    if (text.m_dirty)
    {
        text.updateVertices(drawable);
        drawable.setTexture(&text.getFont()->getTexture(text.getCharacterSize()));
        drawable.setPrimitiveType(sf::PrimitiveType::Triangles);
    }
    return drawable.getLocalBounds();
}

void Text::setCroppingArea(sf::FloatRect area)
{
    LOG("DEPRECATED: Use Drawable::setCroppingArea() instead.", xy::Logger::Type::Warning);
}

sf::FloatRect Text::getCroppingArea() const
{
    LOG("DEPRECATED: use Drawable::getCroppingArea() instead", xy::Logger::Type::Warning);
    return {};
}

void Text::setAlignment(Text::Alignment alignment)
{
    m_alignment = alignment;
    m_dirty = true;
}

//private
void Text::updateVertices(Drawable& drawable)
{
    m_dirty = false;
    
    auto& vertices = drawable.getVertices();

    vertices.clear();
    sf::FloatRect localBounds;
    
    //skip if nothing to build
    if (!m_font || m_string.isEmpty())
    {
        drawable.updateLocalBounds(localBounds);
        return;
    }
    
    //update glyphs - TODO here we could check for bold fonts in the future
    float xOffset = static_cast<float>(m_font->getGlyph(L' ', m_charSize, false).advance);
    float yOffset = static_cast<float>(m_font->getLineSpacing(m_charSize));
    float x = 0.f;
    float y = static_cast<float>(m_charSize);
    
    float minX = x;
    float minY = y;
    float maxX = 0.f;
    float maxY = 0.f;
    
    sf::Uint32 prevChar = 0;
    const auto& string = m_string;
    for (auto i = 0u; i < string.getSize(); ++i)
    {
        sf::Uint32 currChar = string[i];
        
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
        
        //create the quads.
        auto addOutline = [&]()
        {
            const auto& glyph = m_font->getGlyph(currChar, m_charSize, false, m_outlineThickness);

            float left = glyph.bounds.left;
            float top = glyph.bounds.top;
            float right = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top + glyph.bounds.height;

            //add the outline glyph to the vertices
            addQuad(vertices, sf::Vector2f(x, y), m_outlineColour, glyph, m_outlineThickness);

            minX = std::min(minX, x + left - m_outlineThickness);
            maxX = std::max(maxX, x + right - m_outlineThickness);
            minY = std::min(minY, y + top - m_outlineThickness);
            maxY = std::max(maxY, y + bottom - m_outlineThickness);
        };

        //if outline is larger, add first
        if (m_outlineThickness > 0)
        {
            addOutline();
        }

        const auto& glyph = m_font->getGlyph(currChar, m_charSize, false);
        addQuad(vertices, sf::Vector2f(x, y), m_fillColour, glyph);
        
        //else add outline on top
        if (m_outlineThickness < 0)
        {
            addOutline();
        }

        //only do this if not outlined
        if (m_outlineThickness == 0)
        {
            float left = glyph.bounds.left;
            float top = glyph.bounds.top;
            float right = glyph.bounds.left + glyph.bounds.width;
            float bottom = glyph.bounds.top + glyph.bounds.height;

            minX = std::min(minX, x + left);
            maxX = std::max(maxX, x + right);
            minY = std::min(minY, y + top);
            maxY = std::max(maxY, y + bottom);
        }

        x += glyph.advance;
    }
    
    localBounds.left = minX;
    localBounds.top = minY;
    localBounds.width = maxX - minX;
    localBounds.height = maxY - minY;
    
    
    //check for alignment
    float offset = 0.f;
    if (m_alignment == Text::Alignment::Centre)
    {
        offset = localBounds.width / 2.f;
    }
    else if (m_alignment == Text::Alignment::Right)
    {
        offset = localBounds.width;
    }
    if (offset > 0)
    {
        for (auto& v : vertices)
        {
            v.position.x -= offset;
        }
        localBounds.left -= offset;
    }

    drawable.updateLocalBounds(localBounds);
}

void Text::addQuad(std::vector<sf::Vertex>& vertices, sf::Vector2f position, sf::Color colour, const sf::Glyph& glyph,  float outlineThickness)
{
    float left = glyph.bounds.left;
    float top = glyph.bounds.top;
    float right = glyph.bounds.left + glyph.bounds.width;
    float bottom = glyph.bounds.top + glyph.bounds.height;
    
    float u1 = static_cast<float>(glyph.textureRect.left);
    float v1 = static_cast<float>(glyph.textureRect.top);
    float u2 = static_cast<float>(glyph.textureRect.left + glyph.textureRect.width);
    float v2 = static_cast<float>(glyph.textureRect.top + glyph.textureRect.height);
    
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left - outlineThickness, position.y + top - outlineThickness), colour, sf::Vector2f(u1, v1)));
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right - outlineThickness, position.y + top - outlineThickness), colour, sf::Vector2f(u2, v1)));
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left - outlineThickness, position.y + bottom - outlineThickness), colour, sf::Vector2f(u1, v2)));
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + left - outlineThickness, position.y + bottom - outlineThickness), colour, sf::Vector2f(u1, v2)));
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right - outlineThickness, position.y + top - outlineThickness), colour, sf::Vector2f(u2, v1)));
    vertices.push_back(sf::Vertex(sf::Vector2f(position.x + right - outlineThickness, position.y + bottom - outlineThickness), colour, sf::Vector2f(u2, v2)));
}

