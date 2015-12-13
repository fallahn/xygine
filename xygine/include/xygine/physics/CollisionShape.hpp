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

//ABC for creating a shape which can be attached to a physics rigid body
//multiple collision shapes can be attached to a single body

#ifndef XY_COLLISION_SHAPE_HPP_
#define XY_COLLISION_SHAPE_HPP_

#include <Box2D/Dynamics/b2Fixture.h>

#include <SFML/Config.hpp>

namespace xy
{
    namespace Physics
    {
        struct CollisionFilter
        {
            sf::Uint16 categoryFlags = 0u;
            sf::Uint16 maskFlags = 0u;
            sf::Int16 groupIndex = 0;
        };

        class CollisionShape
        {
            friend class RigidBody;
        public:
            enum class Type
            {
                Circle,
                Box,
                Polygon,
                Edge
            };

            CollisionShape();
            virtual ~CollisionShape() = default;
            CollisionShape(const CollisionShape&) = default;
            CollisionShape& operator = (const CollisionShape&) = default;

            virtual Type type() const = 0;

            //sets this collision shape's friction
            void setFriction(float);
            //sets this shape's restitution (bounciness)
            void setRestitution(float);
            //sets the denisty of this shape
            void setDensity(float);
            //sets whether or not this shape is used as a sensor
            //in collision detection
            void setIsSensor(bool);
            //adds a sets of filter flags to define which type of shapes
            //this shape may collide with
            void addFilter(CollisionFilter);

        protected:
            const b2FixtureDef getFixtureDef() const
            {
                return m_fixtureDef;
            }

            void setShape(b2Shape& shape)
            {
                m_fixtureDef.shape = &shape;
            }

        private:
            b2FixtureDef m_fixtureDef;
            b2Fixture* m_fixture;
        };
    }
}

#endif //XY_COLLISION_SHAPE_HPP_
