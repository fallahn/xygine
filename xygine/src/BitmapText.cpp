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

#include <xygine/BitmapText.hpp>
#include <xygine/BitmapFont.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace xy;



BitmapText::BitmapText()
    : m_font    (nullptr),
    m_texture   (nullptr),
    m_colour    (sf::Color::White)
{}

BitmapText::BitmapText(const BitmapFont& font)
    : m_font    (&font),
    m_texture   (&font.getTexture()),
    m_colour    (sf::Color::White)
{}

//public
void BitmapText::setFont(const BitmapFont& font)
{
    m_font = &font;
    m_texture = &font.getTexture();
    updateVerts();
}

void BitmapText::setString(const std::string& str)
{
    m_string = str;
    updateVerts();
}

void BitmapText::setColour(sf::Color colour)
{
    m_colour = colour;
    for (auto& v : m_vertices)
    {
        v.color = colour;
    }
}

sf::FloatRect BitmapText::getLocalBounds() const
{
    return m_localBounds;
}

sf::FloatRect BitmapText::getGlobalBounds() const
{
    return getTransform().transformRect(m_localBounds);
}

//private
void BitmapText::updateVerts()
{
    if (m_font)
    {
        m_vertices.clear();
        m_vertices.reserve(m_string.size() * 4u);

        m_localBounds.width = m_localBounds.height = 0.f;

        sf::Vector2f start;
        sf::FloatRect glyphSize = m_font->getGlyph(' ');

        for (auto c : m_string)
        {            
            if (c == '\n')
            {
                start.x = 0;
                start.y += glyphSize.height;
            }
            else
            {
                auto glyph = m_font->getGlyph(c);
                m_vertices.emplace_back
                (
                    start,
                    m_colour,
                    sf::Vector2f(glyph.left, glyph.top)
                );

                m_vertices.emplace_back
                (
                    sf::Vector2f(start.x + glyphSize.width, start.y),
                    m_colour,
                    sf::Vector2f(glyph.left + glyph.width, glyph.top)
                );

                m_vertices.emplace_back
                (
                    sf::Vector2f(start.x + glyphSize.width, start.y + glyphSize.height),
                    m_colour,
                    sf::Vector2f(glyph.left + glyph.width, glyph.top + glyph.height)
                );

                m_vertices.emplace_back
                (
                    sf::Vector2f(start.x, start.y + glyphSize.height),
                    m_colour,
                    sf::Vector2f(glyph.left, glyph.top + glyph.height)
                );

                start.x += glyphSize.width;
            }
        }
        m_localBounds.width = start.x;
        m_localBounds.height = start.y;
    }
}

void BitmapText::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}