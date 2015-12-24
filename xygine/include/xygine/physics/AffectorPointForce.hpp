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

//applies a force between two points of colliding shapes. the force
//can be negative or positive

#ifndef XY_AFFECTOR_POINT_FORCE_HPP_
#define XY_AFFECTOR_POINT_FORCE_HPP_

#include <xygine/physics/Affector.hpp>
#include <xygine/physics/CollisionFilter.hpp>

#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class CollisionShape;
        class PointForceAffector final : public Affector
        {
            friend class CollisionShape;
        public:
            PointForceAffector(float magnitude, bool wake = false);
            ~PointForceAffector() = default;

            PointForceAffector(const PointForceAffector&) = default;
            PointForceAffector& operator = (const PointForceAffector&) = default;

            Affector::Type type() const override { return Type::PointForce; }
            void apply(RigidBody*) override;

            enum class Centroid
            {
                RigidBody,
                CollisionShape
            };

            //sets whether the target point of the force applied
            //should be the centre of mass of the target body
            //or the centroid of the colliding shape
            void setTargetPoint(Centroid point) { m_targetPoint = point; }
            //sets whether the source point of the force applied
            //should be the centre of mass of the target body
            //or the centroid of the colliding shape
            void setSourcePoint(Centroid point) { m_sourcePoint = point; }
            //set the magnitide of the force vector applied
            void setMagnitude(float magnitude) { m_magnitude = magnitude; }
            //set whether or not this affector should wake
            //sleeping bodies on contact
            void setWake(bool wake) { m_wake = wake; }
            //set the amount of linear drag applied to colliding
            //bdies in the range 0-1
            void setLinearDrag(float);
            //set the amount of angular drag applied to colliding
            //bodies in the range 0-1
            void setAngularDrag(float);
            //set whether or not this affectors collision mask
            //should be compared to that of colliding shapes
            void useCollisionMask(bool mask) { m_useCollisionMask = mask; }
            //sets the collision filter to use when masking collisions
            void setCollisionMask(CollisionFilter filter) { m_collisionMask = filter; }

            //get which target point is currently set for this affector
            Centroid getTargetPoint() const { return m_targetPoint; }
            //get which source point is set for this affector
            Centroid getSourcePoint() const { return m_sourcePoint; }
            //get the current magnitude of this affector's force
            float getMagnitude() const { return m_magnitude; }
            //get the amount of linear drag applied to colliding bodies
            float getLinearDrag() const { return m_linearDrag; }
            //get the amount of angular drag applied to colliding bodies
            float getAngularDrag() const { return m_angularDrag; }
            //returns true if this affector is set to apply its collision
            //mask to colliding shapes
            bool useCollisionMask() const { return m_useCollisionMask; }
            //get the current collision filter settings for this
            //affectors collision mask
            const CollisionFilter& getCollisionMask() const { return m_collisionMask; }

        private:

            Centroid m_targetPoint;
            Centroid m_sourcePoint;
            float m_magnitude;
            bool m_wake;
            float m_linearDrag;
            float m_angularDrag;
            bool m_useCollisionMask;
            CollisionFilter m_collisionMask;

            sf::Vector2f m_force;
            void calcForce(CollisionShape* source, CollisionShape* dest);
        };
    }
}

#endif //XY_AFFECTOR_POINT_FORCE_HPP_