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

#include <xygine/ui/Selection.hpp>
#include <xygine/Util.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>

using namespace xy;
using namespace ui;

namespace
{
    sf::Vector2f currentMousePos;
    const sf::Color borderColour(160u, 160u, 160u);
    const float borderThickness = 4.f;
}

Selection::Selection(const sf::Font& font, const sf::Texture& texture, float length)
    : m_length          (length),
    m_selectedIndex     (0u),
    m_selectedText      ("", font, 36),
    m_prevArrow         (texture),
    m_nextArrow         (texture),
    m_background        ()
{
    sf::IntRect subrect({ 0, 0 }, sf::Vector2i(texture.getSize()));
    subrect.height /= State::Count;

    for (auto i = 0; i < State::Count; ++i)
    {
        m_subRects.push_back(subrect);
        subrect.top += subrect.height;
    }

    m_prevArrow.setTextureRect(m_subRects[State::Normal]);
    
    //flip one of the sprite arrows
    m_nextArrow.setScale(-1.f, 1.f);
    m_nextArrow.setPosition(length, 0.f);
    m_nextArrow.setTextureRect(m_subRects[State::Normal]);

    m_bounds.width = m_length;
    m_bounds.height = static_cast<float>(subrect.height);

    //m_background.setFillColor(sf::Color::Black);
    //m_background.setOutlineColor(borderColour);
    //m_background.setOutlineThickness(-borderThickness);
    //m_background.setSize({ m_length - static_cast<float>(subrect.width * 2.f), m_bounds.height - (borderThickness * 2.f) });
    //m_background.setOrigin(m_background.getSize() / 2.f);
    //m_background.setPosition(m_bounds.width / 2.f, m_bounds.height / 2.f);
}

//public
bool Selection::selectable() const
{
    return true;
}

void Selection::select()
{
    Control::select();
    m_selectedText.setStyle(sf::Text::Bold);
    updateText();

    if (getTransform().transformRect(m_prevArrow.getGlobalBounds()).contains(currentMousePos))
    {
        m_prevArrow.setTextureRect(m_subRects[State::Selected]);
        m_selectedButton = SelectedButton::Prev;
    }
    else if (getTransform().transformRect(m_nextArrow.getGlobalBounds()).contains(currentMousePos))
    {
        m_nextArrow.setTextureRect(m_subRects[State::Selected]);
        m_selectedButton = SelectedButton::Next;
    }
}

void Selection::deselect()
{
    Control::deselect();
    m_selectedText.setStyle(sf::Text::Regular);
    updateText();
    m_prevArrow.setTextureRect(m_subRects[State::Normal]);
    m_nextArrow.setTextureRect(m_subRects[State::Normal]);
}

void Selection::activate()
{
    Control::activate();
    select();

    if (m_selectedButton == SelectedButton::Prev)
    {
        m_selectedIndex = (m_selectedIndex + m_items.size() - 1) % m_items.size();
    }
    else
    {
        m_selectedIndex = (m_selectedIndex + 1) % m_items.size();
    }
    setSelectedIndex(m_selectedIndex);

    deactivate();
}

void Selection::deactivate()
{
    Control::deactivate();

}

void Selection::handleEvent(const sf::Event& evt, const sf::Vector2f& mousePos)
{

}

void Selection::setAlignment(Alignment a)
{
    switch (a)
    {
    case Alignment::BottomLeft:
        setOrigin(0.f, m_bounds.height);
        break;
    case Alignment::BottomRight:
        setOrigin(m_bounds.width, m_bounds.height);
        break;
    case Alignment::Centre:
        setOrigin(m_bounds.width / 2.f, m_bounds.height / 2.f);
        break;
    case Alignment::TopLeft:
        setOrigin(sf::Vector2f());
        break;
    case Alignment::TopRight:
        setOrigin(m_bounds.width, 0.f);
        break;
    default: break;
    }
}

bool Selection::contains(const sf::Vector2f& mousePos) const
{
    currentMousePos = mousePos;
    return getTransform().transformRect(m_bounds).contains(mousePos);
}

void Selection::addItem(const std::string& name, sf::Int32 value)
{
    m_items.emplace_back(std::make_unique<Item>(name, value));

    setSelectedIndex(m_items.size() - 1);
}

const std::string& Selection::getSelectedText() const
{
    assert(m_items.size());
    return m_items[m_selectedIndex]->name;
}

sf::Int32 Selection::getSelectedValue() const
{
    assert(m_items.size());
    return m_items[m_selectedIndex]->value;
}

void Selection::setSelectedIndex(sf::Uint16 index)
{
    assert(index < m_items.size());
    m_selectedIndex = index;
    m_selectedText.setString(m_items[index]->name);
    updateText();
    if (m_selectionChanged) m_selectionChanged(this);
}

sf::Uint32 Selection::itemCount() const
{
    return m_items.size();
}

void Selection::selectItem(const std::string& name)
{
    auto result = std::find_if(m_items.begin(), m_items.end(),
        [&name](const Item::Ptr& p)
    {
        return (p->name == name);
    });

    if (result != m_items.end())
    {
        setSelectedIndex(result - m_items.begin());
    }
}

void Selection::selectItem(sf::Uint16 val)
{
    auto result = std::find_if(m_items.begin(), m_items.end(),
        [val](const Item::Ptr& p)
    {
        return (p->value == val);
    });

    if (result != m_items.end())
    {
        setSelectedIndex(result - m_items.begin());
    }
}

void Selection::setCallback(Callback callback)
{
    m_selectionChanged = std::move(callback);
}

//private
void Selection::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    //rt.draw(m_background, states);
    rt.draw(m_prevArrow, states);
    rt.draw(m_nextArrow, states);
    rt.draw(m_selectedText, states);
}

void Selection::updateText()
{
    Util::Position::centreOrigin(m_selectedText);
    m_selectedText.setPosition(m_length / 2.f, m_selectedText.getLocalBounds().height / 4.f);
}

///---------///
Selection::Item::Item(const std::string& n, sf::Int32 v)
    : name(n), value(v){}