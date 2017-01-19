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

#include <xygine/ui/Button.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace xy;
using namespace UI;

Button::Button(const sf::Font& font, const sf::Texture& texture)
    : m_texture     (texture),
    m_text          ("", font, 30u),
    m_toggleButton  (false)
{
    sf::IntRect subrect({ 0, 0 }, sf::Vector2i(texture.getSize()));
    subrect.height /= State::Count;

    for (auto i = 0; i < State::Count; ++i)
    {
        m_subRects.push_back(subrect);
        subrect.top += subrect.height;
    }

    m_sprite.setTexture(m_texture);
    m_sprite.setTextureRect(m_subRects[State::Normal]);
    
    auto bounds = m_sprite.getLocalBounds();
    m_text.setPosition(bounds.width / 2.f, bounds.height / 2.f);
}

//public
bool Button::selectable() const
{
    return true;
}

void Button::select()
{
    Control::select();
    m_sprite.setTextureRect(m_subRects[State::Selected]);
}

void Button::deselect()
{
    Control::deselect();
    m_sprite.setTextureRect(m_subRects[State::Normal]);
}

void Button::activate()
{
    Control::activate();
    if (m_toggleButton)
        m_sprite.setTextureRect(m_subRects[State::Active]);

    for (auto& cb : m_callbacks)
    {
        cb();
    }

    if (!m_toggleButton)
        deactivate();
}

void Button::deactivate()
{
    Control::deactivate();
    if (m_toggleButton)
    {
        if (selected())
        {
            m_sprite.setTextureRect(m_subRects[State::Selected]);
        }
        else
        {
            m_sprite.setTextureRect(m_subRects[State::Normal]);
        }
    }
}

void Button::handleEvent(const sf::Event& e, const sf::Vector2f& mousePos)
{

}

void Button::setAlignment(Alignment a)
{
    switch (a)
    {
    case Alignment::TopLeft:
        setOrigin(0.f, 0.f);
        break;
    case Alignment::BottomLeft:
        setOrigin(0.f, static_cast<float>(m_subRects[0].height));
        break;
    case Alignment::Centre:
        setOrigin(static_cast<float>(m_subRects[0].width / 2), static_cast<float>(m_subRects[0].height / 2));
        break;
    case Alignment::TopRight:
        setOrigin(static_cast<float>(m_subRects[0].width), 0.f);
        break;
    case Alignment::BottomRight:
        setOrigin(static_cast<float>(m_subRects[0].width), static_cast<float>(m_subRects[0].height));
        break;
    default:break;
    }
}

bool Button::contains(const sf::Vector2f& mousePos) const
{
    return getTransform().transformRect(m_sprite.getGlobalBounds()).contains(mousePos);
}

void Button::addCallback(const Callback& cb)
{
    m_callbacks.push_back(cb);
}

void Button::setString(const std::string& text)
{
    m_text.setString(text);
    Util::Position::centreOrigin(m_text);
}

void Button::setTextColour(const sf::Color& colour)
{
    m_text.setFillColor(colour);
}

void Button::setFontSize(sf::Uint16 size)
{
    m_text.setCharacterSize(size);
    Util::Position::centreOrigin(m_text);
}

void Button::setTogglable(bool b)
{
    m_toggleButton = b;
}

//private
void Button::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.shader = getActiveShader();
    rt.draw(m_sprite, states);
    rt.draw(m_text, states);
}