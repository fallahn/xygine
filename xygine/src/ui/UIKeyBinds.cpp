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

#include <xygine/ui/KeyBinds.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;
using namespace xy::UI;

KeyBinds::KeyBinds(const sf::Font& font)
{
    float vertPos = 0.f;
    const float spacing = 60.f;
    for (auto& item : m_items)
    {
        item.setPosition(0.f, vertPos);
        vertPos += spacing;

        item.setFont(font);
    }
}

//public
void KeyBinds::handleEvent(const sf::Event& evt, const sf::Vector2f& mousePosition)
{
    /*
    TODO use mouse click to check if position is inside an input box
    */
    auto point = getInverseTransform().transformPoint(mousePosition);
}

void KeyBinds::setAlignment(Alignment alignment)
{
    /*
    TODO set origin appropriately
    */
}


//private
void KeyBinds::updateLayout()
{
    /*
    TODO update all items from information in keybindings
    */
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
    const sf::Vector2f inputSize(40.f, 40.f);
    const std::array<sf::Vector2f, 2u> inputPositions = { sf::Vector2f(300.f, 0.f), sf::Vector2f(600.f, 0.f) };
    const sf::Color inputColour(127, 127, 127);
}

///-----Item-----//
KeyBinds::Item::Item()
    : m_activeInput(ItemInput::None)
{
    //TODO
    /*
    set up bounds for input testing
    set up text positions
    */

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

void KeyBinds::Item::setValue(const std::string& name, const std::string& v1, const std::string& v2)
{
    /*
    TODO
    set values and recalculate label position - maybe make label setter separate?
    */
}

void KeyBinds::Item::setFont(const sf::Font& f)
{
    for (auto& t : m_texts)
    {
        t.setFont(f);
    }
    m_texts[2].setPosition(inputPositions[1].x - m_texts[2].getLocalBounds().width, 0.f);
}

KeyBinds::ItemInput KeyBinds::Item::onInput(const sf::Vector2f& mousePosition) const
{
    auto point = getInverseTransform().transformPoint(mousePosition);

    if (m_boundingBoxes.first.contains(point)) return ItemInput::First;
    if (m_boundingBoxes.second.contains(point)) return ItemInput::Second;
    return ItemInput::None;
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