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

#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/physics/World.hpp>

#include <cstring>

using namespace xy;
using namespace xy::Physics;

CollisionPolygonShape::CollisionPolygonShape(const std::vector<sf::Vector2f>& points)
{
    setPoints(points);
    setShape(m_polyShape);
}

CollisionPolygonShape::CollisionPolygonShape(const CollisionPolygonShape& other)
{
    std::memcpy(this, &other, sizeof(CollisionPolygonShape));
    setShape(m_polyShape);
}

void CollisionPolygonShape::operator= (const CollisionPolygonShape& other)
{
    if (&other == this) return;
    std::memcpy(this, &other, sizeof(CollisionPolygonShape));
    setShape(m_polyShape);
}

//public
void CollisionPolygonShape::setPoints(const std::vector<sf::Vector2f>& points)
{
    XY_ASSERT(points.size() <= b2_maxPolygonVertices, "Cannot create a polygon shape with more than " + std::to_string(b2_maxPolygonVertices) + " points");
    XY_ASSERT(points.size() > 2, "Not enough points to create a polygon");
    
    std::vector<b2Vec2> newPoints;
    for (const auto& p : points)
    {
        newPoints.push_back(World::sfToBoxVec(p));
    }
    m_polyShape.Set(newPoints.data(), newPoints.size());
}
