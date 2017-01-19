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

#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/World.hpp>

using namespace xy;
using namespace xy::Physics;

CollisionEdgeShape::CollisionEdgeShape(const std::vector<sf::Vector2f>& points, Option option)
{
    setPoints(points, option);    
    applyCloneContext();
    setShape(m_chainShape);
}

CollisionEdgeShape::CollisionEdgeShape(const CollisionEdgeShape& other)
    : CollisionShape(other)
{
    m_cloneContext = other.m_cloneContext;

    applyCloneContext();
    setShape(m_chainShape);
}

//public
void CollisionEdgeShape::setPoints(const std::vector<sf::Vector2f>& points, Option option)
{
    XY_ASSERT(points.size() > 1, "Not enough points to create a chain");
    m_cloneContext.points.clear();
    for (const auto& p : points)
    {
        m_cloneContext.points.push_back(World::sfToBoxVec(p));
    }
    m_cloneContext.option = option;
}

//private
void CollisionEdgeShape::applyCloneContext()
{
    switch (m_cloneContext.option)
    {
    default:
    case Option::None:
        m_chainShape.CreateChain(m_cloneContext.points.data(), m_cloneContext.points.size());
        break;
    case Option::GhostPoints:
    {
        XY_ASSERT(m_cloneContext.points.size() >= 4, "Edge shape requires at least 4 points to have ghost vertices");
        std::vector<b2Vec2> temp;
        for (auto i = 1u; i < m_cloneContext.points.size() - 1; ++i)
        {
            temp.push_back(m_cloneContext.points[i]);
        }
        m_chainShape.CreateChain(temp.data(), temp.size());
        m_chainShape.SetPrevVertex(m_cloneContext.points.front());
        m_chainShape.SetNextVertex(m_cloneContext.points.back());
    }
        break;
    case Option::Loop:
        m_chainShape.CreateLoop(m_cloneContext.points.data(), m_cloneContext.points.size());
        break;
    }
}
