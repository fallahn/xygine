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

#include <PhysicsDemoLine.hpp>

#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace PhysDemo;

LineDrawable::LineDrawable(xy::MessageBus& mb)
    :Component(mb, this),
    m_vertices(2),
    m_alpha(0u){}

//public
void LineDrawable::entityUpdate(xy::Entity& entity, float)
{
    m_transform = entity.getWorldTransform().getInverse();
}

void LineDrawable::setColour(sf::Color c)
{
    c.a = m_alpha;
    m_vertices[0].color = c;
    m_vertices[1].color = c;
}

void LineDrawable::enable(bool enable)
{
    m_alpha = (enable) ? 255u : 0u;
    m_vertices[0].color.a = m_alpha;
    m_vertices[1].color.a = m_alpha;
}

void LineDrawable::setPoints(const sf::Vector2f& a, const sf::Vector2f& b)
{
    m_vertices[0].position = a;
    m_vertices[1].position = b;
}

//private
void LineDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= m_transform;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Lines, states);
}