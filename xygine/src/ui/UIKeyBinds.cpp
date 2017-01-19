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

#include <xygine/ui/KeyBinds.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <SFML/Window/Event.hpp>

using namespace xy;
using namespace xy::UI;

KeyBinds::KeyBinds(const sf::Font& font)
    : m_selectedIndex   (-1),
    m_selectedInput     (ItemInput::None)
{
    float vertPos = 0.f;
    const float spacing = 60.f;
    for (auto& item : m_items)
    {
        item.setPosition(0.f, vertPos);
        vertPos += spacing;

        item.setFont(font);
    }
    m_bounds.height = vertPos;

    std::array<std::string, Input::KeyCount> names = 
    {
        "Up  ",
        "Down  ",
        "Left  ",
        "Right  ",
        "Start  ",
        "Back  ",
        "ActionOne  ",
        "ActionTwo  ",
        "ActionThree  ",
        "ActionFour  "
    };

    std::size_t i = 0u;
    for (auto& item : m_items)
    {
        item.setLabel(names[i++]);
        if (item.getLocalBounds().left < m_bounds.left)
        {
            m_bounds.left = item.getLocalBounds().left;
            m_bounds.width = item.getLocalBounds().width;
        }
    }
    setOrigin(m_bounds.left, m_bounds.top);

    updateLayout();
}

//public
bool KeyBinds::contains(const sf::Vector2f& mousePosition) const
{
    auto point = getInverseTransform().transformPoint(mousePosition);
    return m_bounds.contains(point);
}

void KeyBinds::handleEvent(const sf::Event& evt, const sf::Vector2f& mousePosition)
{
    auto point = getInverseTransform().transformPoint(mousePosition);

    if (m_selectedIndex < 0)
    {
        //check for mouse clicks
        if (evt.type == sf::Event::MouseButtonReleased
            && evt.mouseButton.button == sf::Mouse::Left)
        {
            for (auto i = 0u; i < m_items.size(); ++i)
            {
                ItemInput ip = ItemInput::None;
                if ((ip = m_items[i].onClick(point)) != ItemInput::None)
                {
                    m_selectedIndex = i;
                    m_selectedInput = ip;
                    break;
                }
            }
        }
    }
    else
    {
        //check for new input
        if (evt.type == sf::Event::KeyReleased)
        {
            sf::Int32 key = evt.key.code;
            switch (evt.key.code)
            {
            case sf::Keyboard::BackSpace:
                key = Input::Unbound;
            default:
                //bind the key first
                if (m_selectedInput == ItemInput::First)
                {
                    Input::bindKey(key, m_selectedIndex);
                }
                else
                {
                    Input::bindAltKey(key, m_selectedIndex);
                }
                updateLayout();
            case sf::Keyboard::Escape:
                //cancel the input
                m_items[m_selectedIndex].clearSelection();
                m_selectedIndex = -1;
                m_selectedInput = ItemInput::None;
                deactivate();
                break;
                //disallowed keys
            case sf::Keyboard::Tab:
            case sf::Keyboard::RSystem:
            case sf::Keyboard::LSystem:
            case sf::Keyboard::Pause:
            case sf::Keyboard::Menu:
            case sf::Keyboard::Unknown:
                break;
            }
        }
    }
}

void KeyBinds::setAlignment(Alignment alignment)
{
    switch (alignment)
    {
    default: break;
    case Alignment::BottomLeft:
        setOrigin(m_bounds.left, std::abs(m_bounds.top) + m_bounds.height);
        break;
    case Alignment::BottomRight:
        setOrigin(std::abs(m_bounds.left) + m_bounds.width, std::abs(m_bounds.top) + m_bounds.height);
        break;
    case Alignment::Centre:
        setOrigin(m_bounds.left + ((std::abs(m_bounds.left) + m_bounds.width) / 2.f), m_bounds.top + ((std::abs(m_bounds.top) + m_bounds.height) / 2.f));
        break;
    case Alignment::TopLeft:
        setOrigin(m_bounds.left, m_bounds.top);
        break;
    case Alignment::TopRight:
        setOrigin(std::abs(m_bounds.left) + m_bounds.width, m_bounds.top);
        break;
    }
}


//private
void KeyBinds::updateLayout()
{
    /*
    update all items from information in keybindings
    */

    for (sf::Int32 i = Input::Up; i < Input::KeyCount; ++i)
    {
        m_items[i].setValues(Input::getKeyAsString(Input::getKey(i)), Input::getKeyAsString(Input::getAltKey(i)));
    }
}

void KeyBinds::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    for (const auto& item : m_items)
    {
        rt.draw(item, states);
    }
}

namespace
{
    const sf::Vector2f inputSize(260.f, 40.f);
    const std::array<sf::Vector2f, 2u> inputPositions = { sf::Vector2f(100.f, 0.f), sf::Vector2f(560.f, 0.f) };
    const sf::Color inputColour(127, 127, 127);
}

///-----Item-----//
KeyBinds::Item::Item()
{
    //vertex array for input boxes
    m_vertices[0].color = inputColour;
    m_vertices[0].position = inputPositions[0];
    m_vertices[1].color = inputColour;
    m_vertices[1].position.x = inputPositions[0].x + inputSize.x;
    m_vertices[2].color = inputColour;
    m_vertices[2].position = inputPositions[0] + inputSize;
    m_vertices[3].color = inputColour;
    m_vertices[3].position.x = inputPositions[0].x;
    m_vertices[3].position.y = inputPositions[0].y + inputSize.y;

    m_vertices[4].color = sf::Color::Transparent;
    m_vertices[4].position = m_vertices[1].position;
    m_vertices[5].color = sf::Color::Transparent;
    m_vertices[5].position = inputPositions[1];
    m_vertices[6].color = sf::Color::Transparent;
    m_vertices[6].position = inputPositions[1];
    m_vertices[6].position.y += inputSize.y;
    m_vertices[7].color = sf::Color::Transparent;
    m_vertices[7].position = m_vertices[2].position;

    m_vertices[8].color = inputColour;
    m_vertices[8].position = m_vertices[5].position;
    m_vertices[9].color = inputColour;
    m_vertices[9].position.x = inputPositions[1].x + inputSize.x;
    m_vertices[10].color = inputColour;
    m_vertices[10].position = inputPositions[1] + inputSize;
    m_vertices[11].color = inputColour;
    m_vertices[11].position.x = inputPositions[1].x;
    m_vertices[11].position.y = inputPositions[1].y + inputSize.y;

    //set up bounds for testing
    m_boundingBoxes.first = { inputPositions[0], inputSize };
    m_boundingBoxes.second = { inputPositions[1], inputSize };

    m_texts[2].setString("Alt: ");
    
}

void KeyBinds::Item::setLabel(const std::string& label)
{
    m_texts[0].setString(label);
    m_texts[0].setPosition(inputPositions[0].x - m_texts[0].getLocalBounds().width, 0.f);
}

void KeyBinds::Item::setValues(const std::string& v1, const std::string& v2)
{
    m_texts[1].setString(v1);
    xy::Util::Position::centreOrigin(m_texts[1]);
    m_texts[1].setPosition(inputPositions[0] + (inputSize / 2.f));

    m_texts[3].setString(v2);
    xy::Util::Position::centreOrigin(m_texts[3]);
    m_texts[3].setPosition(inputPositions[1] + (inputSize / 2.f));
}

void KeyBinds::Item::setFont(const sf::Font& f)
{
    for (auto& t : m_texts)
    {
        t.setFont(f);
    }
    m_texts[2].setPosition(inputPositions[1].x - m_texts[2].getLocalBounds().width, 0.f);
}

KeyBinds::ItemInput KeyBinds::Item::onClick(const sf::Vector2f& mousePosition)
{
    auto point = getInverseTransform().transformPoint(mousePosition);

    if (m_boundingBoxes.first.contains(point))
    {
        for (auto i = 0; i < 4; ++i)
        {
            m_vertices[i].color = sf::Color::Red;
        }
        return ItemInput::First;
    }

    if (m_boundingBoxes.second.contains(point))
    {
        for (auto i = 8; i < 12; ++i)
        {
            m_vertices[i].color = sf::Color::Red;
        }
        return ItemInput::Second;
    }
    return ItemInput::None;
}

void KeyBinds::Item::clearSelection()
{
    for (auto i = 0; i < 4; ++i)
    {
        m_vertices[i].color = inputColour;
    }

    for (auto i = 8; i < 12; ++i)
    {
        m_vertices[i].color = inputColour;
    }
}

sf::FloatRect KeyBinds::Item::getLocalBounds() const
{
    auto pos = m_texts[0].getPosition();
    return{ pos, { sf::Vector2f(std::abs(pos.x), std::abs(pos.y)) + inputPositions[1] + inputSize } };
}

//private
void KeyBinds::Item::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    for (const auto& t : m_texts)
    {
        rt.draw(t, states);
    }
}