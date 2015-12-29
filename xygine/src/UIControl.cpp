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

#include <xygine/ui/Control.hpp>

using namespace xy;
using namespace UI;

Control::Control()
    : m_selected    (false),
    m_active        (false),
    m_visible       (true),
    m_index         (0u)
{

}

//public
bool Control::selected() const
{
    return m_selected;
}

void Control::select()
{
    m_selected = true;
}

void Control::deselect()
{
    m_selected = false;
}

bool Control::active() const
{
    return m_active;
}

void Control::activate()
{
    m_active = true;
}

void Control::deactivate()
{
    m_active = false;
}

bool Control::contains(const sf::Vector2f& mousePos) const
{
    return false;
}

void Control::setVisible(bool visible)
{
    m_visible = visible;
}

bool Control::visible() const
{
    return m_visible;
}