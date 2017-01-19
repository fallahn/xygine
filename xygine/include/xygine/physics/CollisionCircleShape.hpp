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

//represents a circle shape which can be used for collision detection

#ifndef XY_COLLISION_CIRCLE_SHAPE_HPP_
#define XY_COLLISION_CIRCLE_SHAPE_HPP_

#include <xygine/physics/CollisionShape.hpp>

#include <Box2D/Collision/Shapes/b2CircleShape.h>

#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Circle shape specialisation of CollisionShape
        */
        class XY_EXPORT_API CollisionCircleShape final : public CollisionShape
        {
        public:
            explicit CollisionCircleShape(float radius);
            ~CollisionCircleShape() = default;
            CollisionCircleShape(const CollisionCircleShape&);
            const CollisionCircleShape& operator = (const CollisionCircleShape&) = delete;

            CollisionShape::Type type() const override { return CollisionShape::Type::Circle; }

            /*!
            \brief Sets the position of this shape relative to the parent rigidbody
            */
            void setPosition(const sf::Vector2f&);
            /*!
            \brief Gets the circle's position relative to its parent rigidbody
            */
            sf::Vector2f getPosition() const;
            /*!
            \brief Sets the circle's radius in pixels
            */
            void setRadius(float);
            /*!
            \brief Gets the circle's current radius in pixels
            */
            float getRadius() const;

        private:
            b2CircleShape m_circleShape;
        };
    }
}

#endif //XY_COLLISION_CIRCLE_SHAPE_HPP_