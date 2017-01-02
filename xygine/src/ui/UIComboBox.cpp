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

#include <xygine/ui/ComboBox.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <xygine/Assert.hpp>

using namespace xy;
using namespace UI;

namespace
{
    const float joyThresh = 40.f;
}

ComboBox::ComboBox(const sf::Font& font, const sf::Texture& t)
    : m_showItems       (false),
    m_mainShape         (static_cast<sf::Vector2f>(t.getSize())),
    m_dropDownShape     (m_mainShape.getSize()),
    m_highlightShape    (m_mainShape.getSize()),
    m_selectedIndex     (0u),
    m_nextIndex         (m_selectedIndex),
    m_selectedText      ("", font, 26u),
    m_font              (font)
{
    m_mainShape.setTexture(&t);
    m_highlightShape.setFillColor(sf::Color(120u, 120u, 120u));
    m_dropDownShape.setFillColor(sf::Color(90u, 90u, 90u));
    m_dropDownShape.setPosition(0.f, m_mainShape.getSize().y);
}

//public
bool ComboBox::selectable() const
{
    return true;
}

void ComboBox::select()
{
    Control::select();
    m_mainShape.setOutlineThickness(2.f);
}

void ComboBox::deselect()
{
    Control::deselect();
    m_mainShape.setOutlineThickness(0.f);
}

void ComboBox::activate()
{
    Control::activate();

    m_showItems = true;
    m_highlightShape.setPosition(0.f, m_items[m_selectedIndex]->bounds.top);
}

void ComboBox::deactivate()
{
    Control::deactivate();

    m_showItems = false;
}

void ComboBox::handleEvent(const sf::Event& e, const sf::Vector2f& mousePos)
{
    //keyboard
    if (e.type == sf::Event::KeyReleased)
    {
        switch (e.key.code)
        {
        case sf::Keyboard::Up:
            m_nextIndex = (m_nextIndex + m_items.size() - 1) % m_items.size();
            m_highlightShape.setPosition(0.f, m_items[m_nextIndex]->bounds.top);
            break;
        case sf::Keyboard::Down:
            m_nextIndex = (m_nextIndex + 1) % m_items.size();
            m_highlightShape.setPosition(0.f, m_items[m_nextIndex]->bounds.top);
            break;
        case sf::Keyboard::Return:
            setSelectedIndex(m_nextIndex);
            deactivate();
            break;
        default:break;
        }
    }
    //controller
    else if (e.type == sf::Event::JoystickMoved)
    {
        if (e.joystickMove.axis == sf::Joystick::Axis::PovY)
        {
            if (e.joystickMove.position > joyThresh)
            {
                m_nextIndex = (m_nextIndex + m_items.size() - 1) % m_items.size();
                m_highlightShape.setPosition(0.f, m_items[m_nextIndex]->bounds.top);
            }
            else if (e.joystickMove.position < -joyThresh)
            {
                m_nextIndex = (m_nextIndex + 1) % m_items.size();
                m_highlightShape.setPosition(0.f, m_items[m_nextIndex]->bounds.top);
            }
        }
    }
    else if (e.type == sf::Event::JoystickButtonReleased)
    {
        switch (e.joystickButton.button)
        {
        case 0: //A on xbox, therefore accept
            setSelectedIndex(m_nextIndex);
            deactivate();
            break;
        case 1: //B on xbox, therefore back / cancel
            deactivate();
            break;
        default: break;
        }
    }
    //mouse
    else if (e.type == sf::Event::MouseMoved)
    {
        auto localPos = getInverseTransform().transformPoint(mousePos);
        for (auto i = 0u; i < m_items.size(); ++i)
        {
            if (m_items[i]->bounds.contains(localPos))
            {
                m_highlightShape.setPosition(0.f, m_items[i]->bounds.top);
                m_nextIndex = i;
            }
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed
        && e.mouseButton.button == sf::Mouse::Left)
    {
        setSelectedIndex(m_nextIndex);
        deactivate();
    }
}

void ComboBox::setAlignment(Alignment a)
{
    switch (a)
    {
    case Alignment::BottomLeft:
        setOrigin(0.f, m_mainShape.getSize().y);
        break;
    case Alignment::BottomRight:
        setOrigin(m_mainShape.getSize());
        break;
    case Alignment::Centre:
        setOrigin(m_mainShape.getSize() / 2.f);
        break;
    case Alignment::TopLeft:
        setOrigin(0.f, 0.f);
        break;
    case Alignment::TopRight:
        setOrigin(m_mainShape.getSize().x, 0.f);
        break;
    default:break;
    }
}

bool ComboBox::contains(const sf::Vector2f& mousePos) const
{
    return getTransform().transformRect(m_mainShape.getGlobalBounds()).contains(mousePos);
}

void ComboBox::addItem(const std::string& name, sf::Int32 val)
{
    Item::Ptr item = std::make_unique<Item>(name, val, m_font);
    item->bounds = m_highlightShape.getLocalBounds();

    const float dropHeight = item->bounds.height * static_cast<float>(m_items.size() + 1u);
    sf::Vector2f size = m_dropDownShape.getSize();
    size.y = dropHeight;
    m_dropDownShape.setSize(size);

    item->bounds.top = dropHeight;
    item->text.setPosition(2.f, item->bounds.top);
    m_items.push_back(std::move(item));

    setSelectedIndex(m_items.size() - 1u);
}

const std::string& ComboBox::getSelectedText() const
{
    XY_ASSERT(!m_items.empty(), "items are empty");
    return m_items[m_selectedIndex]->name;
}

sf::Int32 ComboBox::getSelectedValue() const
{
    XY_ASSERT(!m_items.empty(), "items ar empty");
    return m_items[m_selectedIndex]->value;
}

void ComboBox::setBackgroundColour(const sf::Color& c)
{
    m_dropDownShape.setFillColor(c);
}

void ComboBox::setHighlightColour(const sf::Color& c)
{
    m_highlightShape.setFillColor(c);
}

void ComboBox::setSelectedIndex(Index index)
{
    XY_ASSERT(index < m_items.size(), "index is out of range");
    m_selectedIndex = index;
    m_selectedText.setString(m_items[index]->name);
}

sf::Uint32 ComboBox::size() const
{
    return m_items.size();
}

void ComboBox::selectItem(const std::string& name)
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

void ComboBox::selectItem(Index val)
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

//private
void ComboBox::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.shader = getActiveShader();
    rt.draw(m_mainShape, states);
    rt.draw(m_selectedText, states);

    if (m_showItems)
    {
        rt.draw(m_dropDownShape, states);
        rt.draw(m_highlightShape, states);
        for (const auto& i : m_items)
            rt.draw(i->text, states);
    }
}

//item ctor
ComboBox::Item::Item(const std::string& n, sf::Int32 val, const sf::Font& f)
    : name(n), value(val), text(n, f, 18u){}