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

//applies a constant force to its parent body

#ifndef XY_AFFECTOR_CONSTANT_HPP_
#define XY_AFFECTOR_CONSTANT_HPP_

#include <xygine/physics/Affector.hpp>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class ConstantForceAffector final : public Affector
        {
        public:
            ConstantForceAffector(const sf::Vector2f& force, float torque = 0.f, bool wake = false);
            ~ConstantForceAffector() = default;

            ConstantForceAffector(const ConstantForceAffector&) = default;
            ConstantForceAffector& operator = (const ConstantForceAffector&) = default;

            Affector::Type type() const override { return Affector::Type::ConstantForce; }
            void operator()(RigidBody*) override;

            //sets the force as a vector to be applied
            //to the parent rigidbody each update
            void setForce(const sf::Vector2f&);
            //sets the torque to be applied to the parent
            //body each frame
            void setTorque(float);
            //sets whether or not to wake a sleeping body
            void setWake(bool);

            //returns the current force value
            const sf::Vector2f& getForce() const;
            //returns the current torque value
            float getTorque() const;
            //returns true if this force should wake
            //a sleeping body
            bool getWake() const;

        private:
            sf::Vector2f m_force;
            float m_torque;
            bool m_wake;
        };
    }
}

#endif //XY_AFFECTOR_CONSTANT_HPP_