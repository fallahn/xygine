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

#include "xyginext/ecs/components/BitmapText.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/graphics/BitmapFont.hpp"

#include <SFML/Graphics/Vertex.hpp>

using namespace xy;

namespace
{
    void addCharacter(std::vector<sf::Vertex>& verts, sf::Vector2f position, sf::FloatRect glyph, sf::Color colour)
    {
        verts.emplace_back(position, colour, sf::Vector2f(glyph.left, glyph.top));
        verts.emplace_back(sf::Vector2f(position.x + glyph.width, position.y), colour, sf::Vector2f(glyph.left + glyph.width, glyph.top));
        verts.emplace_back(sf::Vector2f(position.x + glyph.width, position.y + glyph.height), colour, sf::Vector2f(glyph.left + glyph.width, glyph.top + glyph.height));
        verts.emplace_back(sf::Vector2f(position.x, position.y + glyph.height), colour, sf::Vector2f(glyph.left, glyph.top + glyph.height));
    }
}

BitmapText::BitmapText()
    : m_font    (nullptr),
    m_colour    (sf::Color::White),
    m_dirty     (true)
{

}

BitmapText::BitmapText(const BitmapFont& font)
    : m_font    (nullptr),
    m_colour    (sf::Color::White),
    m_dirty     (true)
{
    setFont(font);
}

//public
void BitmapText::setFont(const BitmapFont& font)
{
    m_font = &font;
    m_dirty = true;
}

void BitmapText::setString(const std::string& str)
{
    if (str != m_string)
    {
        m_string = str;
        m_dirty = true;
    }
}

void BitmapText::setColour(sf::Color c)
{
    if (c != m_colour)
    {
        m_colour = c;
        m_dirty = true;
    }
}

const BitmapFont* BitmapText::getFont() const
{
    return m_font;
}

const std::string& BitmapText::getString() const
{
    return m_string;
}

sf::Color BitmapText::getColour() const
{
    return m_colour;
}

sf::FloatRect BitmapText::getLocalBounds(xy::Entity entity)
{
    auto& text = entity.getComponent<xy::BitmapText>();
    auto& drawable = entity.getComponent<xy::Drawable>();
    if (text.m_dirty)
    {
        text.updateVertices(drawable);
        drawable.setTexture(text.getFont()->getTexture());
        drawable.setPrimitiveType(sf::PrimitiveType::Triangles);
    }
    return drawable.getLocalBounds();
}

//private
void BitmapText::updateVertices(Drawable& drawable)
{
    m_dirty = false;

    auto& verts = drawable.getVertices();
    verts.clear();

    if (!m_font || m_string.empty())
    {
        drawable.updateLocalBounds(sf::FloatRect());
        return;
    }

    float left = 0.f;
    float top = 0.f;
    for (auto c : m_string)
    {
        if (c == '\n')
        {
            top += m_font->getCharacterSize().y;
            left = 0.f;
        }
        else if (c == '\t')
        {
            for (auto i = 0; i < 4; ++i)
            {
                auto glyph = m_font->getGlyph(' ');
                addCharacter(verts, { left, top }, glyph, m_colour);
                left += m_font->getCharacterSize().x;
            }
        }
        else if (c > 31 && c < 127)
        {
            addCharacter(verts, { left, top }, m_font->getGlyph(c), m_colour);
            left += m_font->getCharacterSize().x;
        }
    }

    drawable.updateLocalBounds();
}