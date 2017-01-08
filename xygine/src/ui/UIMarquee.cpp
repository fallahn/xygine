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

#include <xygine/ui/Marquee.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;
using namespace xy::UI;

Marquee::Marquee(const sf::Font& font, sf::Vector2u size)
    : m_currentIndex    (0),
    m_scrollSpeed       (-200.f)
{
    XY_ASSERT(size.x > 0 && size.y > 0, "Invalid marquee size");

    m_text.setFont(font);
    m_text.setString("Empty");
    m_text.setPosition(-100.f, 0.f);

    m_texture.create(size.x, size.y);

    m_sprite.setTexture(m_texture.getTexture());

    m_bounds.width = static_cast<float>(size.x);
    m_bounds.height = static_cast<float>(size.y);
}


//public
void Marquee::setAlignment(Alignment a)
{
    switch (a)
    {
    default: break;
    case Alignment::TopLeft:
        setOrigin(0.f, 0.f);
        break;
    case Alignment::BottomLeft:
        setOrigin(0.f, m_bounds.height);
        break;
    case Alignment::Centre:
        setOrigin(m_bounds.width / 2.f, m_bounds.height / 2.f);
        break;
    case Alignment::TopRight:
        setOrigin(m_bounds.width, 0.f);
        break;
    case Alignment::BottomRight:
        setOrigin(m_bounds.width, m_bounds.height);
        break;
    }
}

void Marquee::update(float dt)
{
    if (m_strings.empty()) return;

    m_text.move(m_scrollSpeed * dt, 0.f);

    if (m_text.getPosition().x < -m_text.getGlobalBounds().width)
    {
        m_currentIndex = (m_currentIndex + 1) % m_strings.size();
        m_text.setString(m_strings[m_currentIndex]);
        m_text.setPosition(m_bounds.width, 0.f);
    }
}

void Marquee::addString(const std::string& str)
{
    if (!str.empty())
    {
        m_strings.push_back(str);
        m_currentIndex = (m_currentIndex + 1) % m_strings.size();
    }
}

void Marquee::setCharacterSize(sf::Uint32 size)
{
    m_text.setCharacterSize(size);
}

void Marquee::setTextColour(sf::Color colour)
{
    m_text.setFillColor(colour);
}

void Marquee::setScrollSpeed(float speed)
{
    XY_ASSERT(speed > 0, "speed must be a positive value");
    m_scrollSpeed = -speed;
}

//private
void Marquee::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_texture.clear(sf::Color::Transparent);
    m_texture.draw(m_text);
    m_texture.display();

    states.transform *= getTransform();
    rt.draw(m_sprite, states);
}