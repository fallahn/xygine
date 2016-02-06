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

//creates a convex polygon shape with up to 8 points. concave shapes
//are not supported, but can be created by attaching multiple convex
//shapes to a single rigid body, or you might consider using a loop
//created with CollionEdgeShape instead

#ifndef XY_COLLISION_POLYGON_SHAPE_HPP_
#define XY_COLLISION_POLYGON_SHAPE_HPP_

#include <xygine/physics/CollisionShape.hpp>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <SFML/System/Vector2.hpp>

#include <vector>

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Creates a convex polygon shape with up to 8 points.

        Concave shapes are not supported, but can be created by attaching 
        multiple convex shapes to a single rigidbody, or you might consider
        using a loop created with CollionEdgeShape instead
        */
        class XY_EXPORT_API CollisionPolygonShape final : public CollisionShape
        {
        public:
            /*!
            \brief Constructor
            
            \param Points is a vector of vertex positions. These are in world space when
            not attached to a rigid body, else they are relative to the rigidbody's position
            */
            explicit CollisionPolygonShape(const std::vector<sf::Vector2f>& points);
            ~CollisionPolygonShape() = default;
            CollisionPolygonShape(const CollisionPolygonShape&);
            const CollisionPolygonShape& operator = (const CollisionPolygonShape&) = delete;

            CollisionShape::Type type() const override { return CollisionShape::Type::Polygon; }

            /*!
            \brief Sets the vertice of the CollisionPolygonShape

            \param Points is a vector of vertex positions. These are in world space when
            not attached to a rigid body, else they are relative to the rigidbody's position
            */
            void setPoints(const std::vector<sf::Vector2f>& points);

        private:
            b2PolygonShape m_polyShape;
        };
    }
}

#endif //XY_COLLISION_POLYGON_SHAPE_HPP_