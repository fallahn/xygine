/*********************************************************************
Matt Marchant 2014 - 2016
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

#include <xygine/ui/CheckBox.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace xy;
using namespace UI;

namespace
{
    const float textPadding = 20.f;
}

CheckBox::CheckBox(const sf::Font& font, const sf::Texture& t)
    : m_texture (t),
    m_text      ("", font, 36u),
    m_checked   (false),
    m_alignment (Alignment::TopLeft)
{
    sf::IntRect subRect(0, 0, t.getSize().x, t.getSize().y / 4);
    for (auto i = 0u; i < 4u; ++i)
    {
        m_subRects.push_back(subRect);
        subRect.top += subRect.height;
    }

    m_sprite.setTexture(t);
    m_sprite.setTextureRect(m_subRects[State::Normal]);

    m_text.setPosition(static_cast<float>(m_sprite.getLocalBounds().width) + textPadding, -6.f);
}

//public
bool CheckBox::selectable() const
{
    return true;
}

void CheckBox::select()
{
    Control::select();
    (m_checked) ?
        m_sprite.setTextureRect(m_subRects[State::CheckedSelected]) :
        m_sprite.setTextureRect(m_subRects[State::Selected]);
}

void CheckBox::deselect()
{
    Control::deselect();
    (m_checked) ?
        m_sprite.setTextureRect(m_subRects[State::CheckedNormal]) :
        m_sprite.setTextureRect(m_subRects[State::Normal]);
}

void CheckBox::activate()
{
    Control::activate();
    m_checked = !m_checked;
    select(); //updates texture rect
    deactivate();
    if (m_checkChanged) m_checkChanged(this);
}

void CheckBox::deactivate()
{
    Control::deactivate();
}

void CheckBox::handleEvent(const sf::Event&, const sf::Vector2f& mousePos){}

void CheckBox::setAlignment(Alignment a)
{
    switch (a)
    {
    case Alignment::BottomLeft:
    {
        float height = m_sprite.getLocalBounds().height + m_text.getLocalBounds().height;
        setOrigin(0.f, height);
    }
        break;
    case Alignment::BottomRight:
    {
        float width = m_sprite.getLocalBounds().width + m_text.getLocalBounds().width + textPadding;
        float height = m_sprite.getLocalBounds().height + m_text.getLocalBounds().height;
        setOrigin(width, height);
    }
        break;
    case Alignment::Centre:
    {
        float width = m_sprite.getLocalBounds().width + m_text.getLocalBounds().width + textPadding;
        setOrigin(width / 2.f, static_cast<float>(m_subRects[0].height) / 2.f);
    }
        break;
    case Alignment::TopLeft:
        setOrigin(0.f, 0.f);
        break;
    case Alignment::TopRight:
        setOrigin(0.f, static_cast<float>(m_subRects[0].height));
        break;
    default: break;
    }
    m_alignment = a;
}

bool CheckBox::contains(const sf::Vector2f& mousePos) const
{
    return getTransform().transformRect(m_sprite.getGlobalBounds()).contains(mousePos);
}

void CheckBox::setText(const std::string& text)
{
    m_text.setString(text);
    setAlignment(m_alignment);
}

void CheckBox::setTextColour(const sf::Color& c)
{
    m_text.setColor(c);
}

void CheckBox::setFont(const sf::Font& font)
{
    m_text.setFont(font);
    setAlignment(m_alignment);
}

void CheckBox::setFontSize(sf::Uint8 size)
{
    m_text.setCharacterSize(size);
    setAlignment(m_alignment);
}

bool CheckBox::checked() const
{
    return m_checked;
}

void CheckBox::check(bool checked)
{
    m_checked = checked;
    if (m_checkChanged) m_checkChanged(this);
    deselect();
}

void CheckBox::setCallback(Callback c, Event evt)
{
    switch (evt)
    {
    case Event::CheckChanged:
        m_checkChanged = std::move(c);
        break;
    default: break;
    }
}

//private
void CheckBox::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_sprite, states);
    rt.draw(m_text, states);
}