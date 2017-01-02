/*********************************************************************
Matt Marchant 2014 - 2017
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

//abstract base class for physics affectors

#ifndef XY_AFFECTOR_HPP_
#define XY_AFFECTOR_HPP_

#include <xygine/Config.hpp>

#include <memory>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Abstract base class for physics affectors

        Affectors can be attachd to collision shapes to influence
        parent rigidbodies or the bodies belonging to colliding shapes
        */
        class XY_EXPORT_API Affector
        {
        public:
            /*!
            \brief Used to identify the specific type of concrete classes
            */
            enum class Type
            {
                ConstantForce,
                AreaForce,
                Buoyancy,
                Conveyor,
                PointForce,
                Platform
            };
            /*!
            \brief Used to identify whether or not forces should be applied
            the centre of parent rigidbosies or the centre of the CollisionShape
            with which the affector is associated
            */
            enum class Centroid
            {
                RigidBody,
                CollisionShape
            };

            Affector() = default;
            virtual ~Affector() = default;

            virtual Type type() const = 0;
            /*!
            \brief Implemented by concrete classes to apply their effect
            \param RigidBody RigidBody to which to apply the effect.
            */
            virtual void apply(RigidBody*) = 0;

        private:

        };
    }
}

#endif //XY_AFFECTOR_HPP_