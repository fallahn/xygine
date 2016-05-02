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

//defines a rectangle collision shape which can be attached to rigid bodies

#ifndef XY_COLLISION_RECTANGLE_SHAPE_HPP_
#define XY_COLLISION_RECTANGLE_SHAPE_HPP_

#include <xygine/physics/CollisionShape.hpp>
#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Defines a rectangular CollisionShape which can be attached to a RigidBody
        */
        class XY_EXPORT_API CollisionRectangleShape final : public CollisionShape
        {
        public:
            /*!
            \brief Creates a box from a size and optional position.

            When attached to a rigidbody the position is relative to its parent,
            else it is in world coordinates

            \param size Size of the box in world units
            \param position Position of the box relative to the parent body
            */
            CollisionRectangleShape(const sf::Vector2f& size, const sf::Vector2f& position = sf::Vector2f());
            ~CollisionRectangleShape() = default;
            CollisionRectangleShape(const CollisionRectangleShape&);
            const CollisionRectangleShape& operator = (const CollisionRectangleShape&) = delete;

            CollisionShape::Type type() const override { return CollisionShape::Type::Box; }

            /*!
            \brief Resets the box's size and optional position.

            When attached to a rigidbody the position is relative to its parent,
            else it is in world coordinates

            \param size Size of the box in world units
            \param position Position of the box relative to the parent RigidBody
            */
            void setRect(const sf::Vector2f& size, const sf::Vector2f& position = sf::Vector2f());
            /*!
            \brief Resets the box's size and optional position.

            When attached to a rigidbody the position is relative to its parent,
            else it is in world coordinates

            \param sf::FloatRect an sf::FloatRect representing the size and position of the
            box in world units
            */
            void setRect(const sf::FloatRect&);

            /*!
            \brief Returns the current size of the box
            */
            const sf::Vector2f& getSize() const;
            /*!
            \brief Returns the current position of the box
            
            Coordinates are relative to the parent body if the CollsiionShape
            has one, else they are in world coordinates.
            */
            const sf::Vector2f& getPosition() const;

        private:

            b2PolygonShape m_rectangleShape;
            sf::Vector2f m_size;
            sf::Vector2f m_position;
        };
    }
}

#endif // XY_COLLISION_RECTANGLE_SHAPE_HPP_