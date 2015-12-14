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

#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/World.hpp>

using namespace xy;
using namespace xy::Physics;

CollisionCircleShape::CollisionCircleShape(float radius)
{
    m_circleShape.m_radius = World::sfToBoxFloat(radius);
    setShape(m_circleShape);
}

CollisionCircleShape::CollisionCircleShape(const CollisionCircleShape& other)
{
    std::memcpy(this, &other, sizeof(CollisionCircleShape));
    setShape(m_circleShape);
}

void CollisionCircleShape::operator=(const CollisionCircleShape& other)
{
    if (&other == this) return;
    std::memcpy(this, &other, sizeof(CollisionCircleShape));
    setShape(m_circleShape);
}

//public
void CollisionCircleShape::setPosition(const sf::Vector2f& position)
{
    m_circleShape.m_p = World::sfToBoxVec(position);
}

sf::Vector2f CollisionCircleShape::getPosition() const
{
    return World::boxToSfVec(m_circleShape.m_p);
}

void CollisionCircleShape::setRadius(float radius)
{
    m_circleShape.m_radius = World::sfToBoxFloat(radius);
}

float CollisionCircleShape::getRadius() const
{
    return World::boxToSfFloat(m_circleShape.m_radius);
}