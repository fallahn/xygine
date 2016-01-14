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

#include <xygine/ui/Container.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace xy;
using namespace UI;

namespace
{
    const float deadzone = 40.f;
}

Container::Container()
    : m_selectedIndex   (0),
    m_background        (sf::Vector2f(1920.f, 1080.f))
{
    m_background.setFillColor(sf::Color::Transparent);
}

//public
void Container::addControl(Control::Ptr c)
{    
    m_controls.push_back(c);

    if (!hasSelection() && c->selectable())
        select(m_controls.size() - 1);
}

bool Container::selectable() const
{
    return false;
}

void Container::handleEvent(const sf::Event& e, const sf::Vector2f& mousePos)
{
    //pass event to selected control
    if (hasSelection() && m_controls[m_selectedIndex]->active())
    {
        m_controls[m_selectedIndex]->handleEvent(e, mousePos);
    }
    //keyboard input
    else if (e.type == sf::Event::KeyReleased)
    {
        if (e.key.code == sf::Keyboard::Up
            || e.key.code == sf::Keyboard::Left)
        {
            selectPrevious();
        }
        else if (e.key.code == sf::Keyboard::Down
            || e.key.code == sf::Keyboard::Right)
        {
            selectNext();
        }
        else if (e.key.code == sf::Keyboard::Return/* ||
            e.key.code == sf::Keyboard::Space*/)
        {
            if (hasSelection())
            {
                m_controls[m_selectedIndex]->activate();
            }
        }
    }
    //controller navigation
    else if (e.type == sf::Event::JoystickMoved)
    {
        if (e.joystickMove.axis == sf::Joystick::Axis::PovY)
        {
            if (e.joystickMove.position > deadzone)
            {
                selectPrevious();
            }
            else if (e.joystickMove.position < -deadzone)
            {
                selectNext();
            }
        }//axis is inverse so needs its own branch :/
        else if (e.joystickMove.axis == sf::Joystick::Axis::PovX)
        {
            if (e.joystickMove.position > deadzone)
            {
                selectNext();
            }
            else if (e.joystickMove.position < -deadzone)
            {
                selectPrevious();
            }
        }
    }
    //controller button
    else if (e.type == sf::Event::JoystickButtonReleased)
    {
        //TODO check button mapping (A and START on xbawx)
        if (e.joystickButton.button == 0 || e.joystickButton.button == 7)
        {
            if (hasSelection())
            {
                m_controls[m_selectedIndex]->activate();
            }
        }
    }
    //mouse input
    else if (e.type == sf::Event::MouseMoved)
    {
        for (auto i = 0u; i < m_controls.size(); ++i)
        {
            if (m_controls[i]->contains(mousePos))
            {
                m_controls[i]->select();
                m_selectedIndex = i;
            }
            else
            {
                m_controls[i]->deselect();
            }
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed
        && e.mouseButton.button == sf::Mouse::Left)
    {
        if (hasSelection())
        {
            if (m_controls[m_selectedIndex]->contains(mousePos))
            {
                m_controls[m_selectedIndex]->activate();
            }
        }
    }
}

void Container::update(float dt)
{
    for (auto& c : m_controls)
        c->update(dt);
}

void Container::setBackgroundColour(const sf::Color& colour)
{
    m_background.setFillColor(colour);
}

void Container::setBackgroundTexture(const sf::Texture& t)
{
    m_background.setTexture(&t);
}

//private
bool Container::hasSelection() const
{
    return m_controls.empty() ? false : (m_selectedIndex >= 0);
}

void Container::select(Index index)
{
    if (m_controls[index]->selectable())
    {
        if (hasSelection())
            m_controls[m_selectedIndex]->deselect();

        m_controls[index]->select();
        m_selectedIndex = index;

        //m_soundPlayer.play(SoundPlayer::AudioId::UIMove);
    }
}

void Container::selectNext()
{
    if (!hasSelection()) return;

    auto next = m_selectedIndex;

    do
    {
        next = (next + 1) % m_controls.size();
    } while (!m_controls[next]->selectable());

    select(next);
}

void Container::selectPrevious()
{
    if (!hasSelection()) return;

    auto prev = m_selectedIndex;

    do
    {
        prev = (prev + m_controls.size() - 1) % m_controls.size();
    } while (!m_controls[prev]->selectable());

    select(prev);
}

void Container::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_background, states);
    for (const auto& c : m_controls)
    {
        rt.draw(*c, states);
    }
}