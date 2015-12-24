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

//applies a force to any bodies intersecting collision shapes connected
//to a rigid body which has this affector applied to it

#ifndef XY_AREA_AFFECTOR_HPP_
#define XY_AREA_AFFECTOR_HPP_

#include <xygine/physics/Affector.hpp>
#include <xygine/physics/CollisionFilter.hpp>

#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class AreaForceAffector final : public Affector
        {
        public:

            AreaForceAffector(const sf::Vector2f& force, float torque = 0.f, bool wake = false);
            ~AreaForceAffector() = default;
            AreaForceAffector(const AreaForceAffector&) = default;
            AreaForceAffector& operator = (const AreaForceAffector&) = default;

            Affector::Type type() const override { return Affector::Type::AreaForce; }
            void apply(RigidBody*) override;

            //sets the force as a vector to be applied
            //to the parent rigidbody each update
            void setForce(const sf::Vector2f& force) { m_force = force; }
            //sets the torque to be applied to the parent
            //body each frame
            void setTorque(float torque) { m_torque = torque; }
            //sets whether or not to wake a sleeping body
            void setWake(bool wake) { m_wake = wake; }
            //set the amount of linear drag to apply to the body
            void setLinearDrag(float);
            //set the amount of angular drag to apply to the body
            void setAngularDrag(float);
            //set whether or not to use a collision mask
            void useCollisionMask(bool m) { m_useMask = m; }
            //set the collision ask to use to filter out fixtures
            void setCollisionMask(CollisionFilter& cm) { m_collisionMask = cm; }

            //returns the current force value
            const sf::Vector2f& getForce() const { return m_force; }
            //returns the current torque value
            float getTorque() const { return m_torque; }
            //returns true if this force should wake
            //a sleeping body
            bool getWake() const { return m_wake; }
            //returns the current amount of linear drag to be applied
            float getLinearDrag()const { return m_linearDrag; }
            //returns the current amount of angular drag to be applied
            float getAngluarDrag()const { return m_angularDrag; }
            //returns true if this filter is set to use its collision mask
            bool useCollisionMask() const { return m_useMask; }
            //returns the affectors current collision mask
            const CollisionFilter& getCollisionMask() const { return m_collisionMask; }

        private:
            sf::Vector2f m_force;
            float m_torque;
            bool m_wake;
            float m_linearDrag;
            float m_angularDrag;
            bool m_useMask;
            CollisionFilter m_collisionMask;
        };
    }
}

#endif //XY_AREA_AFFECTOR_HPP_