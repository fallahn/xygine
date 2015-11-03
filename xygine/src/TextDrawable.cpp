/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/TextDrawable.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace xy;

TextDrawable::TextDrawable(MessageBus& mb)
    : Component(mb, this){}

Component::Type TextDrawable::type() const
{
    return Component::Type::Drawable;
}

void TextDrawable::entityUpdate(Entity&, float){}
void TextDrawable::handleMessage(const Message&){}

sf::FloatRect TextDrawable::localBounds() const
{
    return m_text.getLocalBounds();
}

sf::FloatRect TextDrawable::globalBounds() const
{
    return m_text.getGlobalBounds();
}

sf::FloatRect TextDrawable::getLocalBounds() const
{
    return m_text.getLocalBounds();
}

sf::FloatRect TextDrawable::getGlobalBounds() const
{
    return m_text.getGlobalBounds();
}

void TextDrawable::setString(const std::string& str)
{
    m_text.setString(str);
}

void TextDrawable::setFont(const sf::Font& font)
{
    m_text.setFont(font);
}

void TextDrawable::setCharacterSize(sf::Uint32 size)
{
    m_text.setCharacterSize(size);
}

void TextDrawable::setStyle(sf::Uint32 style)
{
    m_text.setStyle(style);
}

void TextDrawable::setColour(const sf::Color& colour)
{
    m_text.setColor(colour);
}

const sf::String& TextDrawable::getString() const
{
    return m_text.getString();
}

const sf::Font* TextDrawable::getFont() const
{
    return m_text.getFont();
}

sf::Uint32 TextDrawable::getCharacterSize() const
{
    return m_text.getCharacterSize();
}

sf::Uint32 TextDrawable::getStyle() const
{
    return m_text.getStyle();
}

const sf::Color& TextDrawable::getColour() const
{
    return m_text.getColor();
}

sf::Vector2f TextDrawable::findCharacterPos(std::size_t pos) const
{
    return m_text.findCharacterPos(pos);
}

//private
void TextDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_text, states);
}