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

//abstract base class for physics affectors

#ifndef XY_AFFECTOR_HPP_
#define XY_AFFECTOR_HPP_

#include <memory>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        class Affector
        {
        public:
            enum class Type
            {
                ConstantForce,
                AreaForce,
                Buoyancy,
                Conveyor,
                PointForce,
                Platform
            };

            enum class Centroid
            {
                RigidBody,
                CollisionShape
            };

            Affector() = default;
            virtual ~Affector() = default;

            virtual Type type() const = 0;
            virtual void apply(RigidBody*) = 0;

        private:

        };
    }
}

#endif //XY_AFFECTOR_HPP_