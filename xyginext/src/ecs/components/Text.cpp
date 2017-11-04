/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/ecs/components/Text.hpp>

using namespace xy;

Text::Text()
    : m_font        (nullptr),
    m_charSize      (30),
    m_fillColour    (sf::Color::White),
    m_dirty         (true),
    m_alignment     (Alignment::Left),
    m_croppingArea  (-DefaultSceneSize / 2.f, DefaultSceneSize * 2.f),
    m_cropped       (false)
{

}

Text::Text(const sf::Font& font)
    : m_font        (nullptr),
    m_charSize      (30),
    m_fillColour    (sf::Color::White),
    m_dirty         (true),
    m_alignment     (Alignment::Left),
    m_croppingArea  (-DefaultSceneSize / 2.f, DefaultSceneSize * 2.f),
    m_cropped       (false)
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
        //TODO we want to be able to query this without having to wait for
        //the scene to update at least once first - eg when creating text
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

