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

//creates a 2-sided chain or loop from a series of points. useful for
//outlining static geometry or in some cases where concave polygons are needed

#ifndef XY_COLLISION_EDGE_SHAPE_HPP_
#define XY_COLLISION_EDGE_SHAPE_HPP_

#include <xygine/physics/CollisionShape.hpp>
#include <Box2D/Collision/Shapes/b2ChainShape.h>
#include <SFML/System/Vector2.hpp>

#include <vector>

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Creates a 2-sided chain or loop CollisionShape from a series of points.

        Useful for outlining static geometry or in some cases where concave polygons are needed
        */
        class XY_EXPORT_API CollisionEdgeShape final : public CollisionShape
        {
        public:
            enum class Option
            {
                None,
                GhostPoints,
                Loop
            };
            /*!
            \brief Creates an edge shape from the given list of points.

            If the GhostPoints option is supplied the first and last points will be created as ghost vertices. If
            the Loop option is supplied then the edges will created as a closed loop.

            \param points std:vector of sf::Vector2f representing point positions.
            \param option Can be None, GhostPoints or Loop
            */
            CollisionEdgeShape(const std::vector<sf::Vector2f>& points, Option = Option::None);
            ~CollisionEdgeShape() = default;
            CollisionEdgeShape(const CollisionEdgeShape&);
            const CollisionEdgeShape& operator = (const CollisionEdgeShape&) = delete;

            CollisionShape::Type type() const override { return CollisionShape::Type::Edge; }

            /*!
            \brief Creates an edge shape from the given list of points.

            If the GhostPoints option is supplied the first and last points will be created as ghost vertices. If
            the Loop option is supplied then the edges will created as a closed loop.

            \param points std:vector of sf::Vector2f representing point positions.
            \param option Can be None, GhostPoints or Loop
            */
            void setPoints(const std::vector<sf::Vector2f>&, Option = Option::None);

        private:
            b2ChainShape m_chainShape;

            //b2Shapes don't override the assignment operator or copy ctor
            //so we have to manually manage this.
            struct CloneContext
            {
                std::vector<b2Vec2> points;
                Option option;
            }m_cloneContext;
            void applyCloneContext();
        };
    }
}

#endif //XY_COLLISION_EDGE_SHAPE_HPP_