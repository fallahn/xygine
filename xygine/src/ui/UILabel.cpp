/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <xygine/ui/Label.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;
using namespace UI;

Label::Label(const sf::Font& font)
    : m_text            ("", font),
    m_currentAlignment  (Alignment::TopLeft)
{

}

//public
bool Label::selectable() const
{
    return false;
}

void Label::handleEvent(const sf::Event&, const sf::Vector2f&){}

void Label::setAlignment(Alignment alignment)
{
    auto bounds = m_text.getLocalBounds();
    switch (alignment)
    {
    case Alignment::BottomLeft:
        setOrigin(0.f, bounds.height);
        break;
    case Alignment::BottomRight:
        setOrigin(bounds.width, bounds.height);
        break;
    case Alignment::Centre:
        Util::Position::centreOrigin(m_text);
        break;
    case Alignment::TopLeft:
        setOrigin({});
        break;
    case Alignment::TopRight:
        setOrigin(bounds.width, 0.f);
        break;
    default: break;
    }

    m_currentAlignment = alignment;
}

void Label::setString(const std::string& str)
{
    m_text.setString(str);
    updateText();
}

void Label::setColour(const sf::Color& colour)
{
    m_text.setFillColor(colour);
}

void Label::setCharacterSize(sf::Uint32 size)
{
    m_text.setCharacterSize(size);
    updateText();
}

//private
void Label::updateText()
{
    setAlignment(m_currentAlignment);
}

void Label::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.shader = getActiveShader();
    rt.draw(m_text, states);
}