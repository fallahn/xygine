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
        /*!
        \brief Point Force Affector

        Point force affectors apply a set force between two points
        on the RigidBody of the CollisionShape hosting the affector
        and the RigidBody of the intersecting CollisionShape. The
        points can either be the centroid of either of the two 
        RigidBodies, or the centroid of the intersecting CollisionShapes.
        The force applied is along the axis between the two points
        with the given magnitude of the affector. The magnitude can be
        either positive or negative, foirceing the bodies apart or
        pulling them toward each other.
        */
        class XY_EXPORT_API PointForceAffector final : public Affector
        {
            friend class CollisionShape;
        public:
            PointForceAffector(float magnitude, bool wake = false);
            ~PointForceAffector() = default;

            PointForceAffector(const PointForceAffector&) = default;
            PointForceAffector& operator = (const PointForceAffector&) = default;

            Affector::Type type() const override { return Type::PointForce; }
            void apply(RigidBody*) override;

            /*!
            \brief Sets the target point of the force.
            
            The target point of the force can be the centre of mass
            of the target body or the centroid of the colliding shape

            \param point Either Centroid::RigidBody or Centroid CollisionShape
            */
            void setTargetPoint(Centroid point) { m_targetPoint = point; }
            /*!
            \brief Sets the source point of the force applied.

            The source point of the force can be the centre of mass of
            the target body or the centroid of the colliding shape

            \param point Either Centroid::RigidBody or Centroid CollisionShape
            */
            void setSourcePoint(Centroid point) { m_sourcePoint = point; }
            /*!
            \brief Set the magnitide of the force vector applied

            The magnitude can be either positive or negative

            \param magnitude Size pf the force value in world units
            */
            void setMagnitude(float magnitude) { m_magnitude = magnitude; }
            /*!
            \brief Set whether or not this affector should wake sleeping bodies on contact

            \param wake set to true to wake sleeping bodies
            */
            void setWake(bool wake) { m_wake = wake; }
            /*!
            \brief Set the amount of linear drag applied
            
            linear drag is a force opposite to that of the colliding
            bodies current velocity multiplied by the drag value.
            Values should range 0 (no drag) to 1 (full drag, the force
            applied is exactly opposite to the current velocity)

            \param float The amount of drag to apply in range 0-1
            */
            void setLinearDrag(float);
            /*!
            \brief Set the amount of angular drag applied

            angular drag is a force opposite to that of the colliding
            bodies current angular velocity multiplied by the drag value.
            Values should range 0 (no drag) to 1 (full drag, the force
            applied is exactly opposite to the current velocity)

            \param float The amount of drag to apply in range 0-1
            */
            void setAngularDrag(float);
            /*!
            \brief Set whether or not to use a collision mask

            A collision mask can be used with this affector to define
            specific groups of intersecting entities which can be influenced
            by this affector.
            \see CollisionFilter
            \param mask Set to true to use the collision mask
            */
            void useCollisionMask(bool mask) { m_useCollisionMask = mask; }
            /*!
            \brief Set the collision mask to use to filter out fixtures

            \see useCollisionMask
            \param filter The CollisionMask to use
            */
            void setCollisionMask(CollisionFilter filter) { m_collisionMask = filter; }

            /*!
            \brief Get which target point is currently set for this affector
            \see setTargetPoint
            */
            Centroid getTargetPoint() const { return m_targetPoint; }
            /*!
            \brief Get which source point is set for this affector
            \see setSourcePoint
            */
            Centroid getSourcePoint() const { return m_sourcePoint; }
            /*!
            \brief Get the current magnitude of this affector's force
            */
            float getMagnitude() const { return m_magnitude; }
            /*!
            \brief Get the amount of linear drag applied to colliding bodies
            */
            float getLinearDrag() const { return m_linearDrag; }
            /*!
            \brief Get the amount of angular drag applied to colliding bodies
            */
            float getAngularDrag() const { return m_angularDrag; }
            /*!
            \brief Returns true if this affector is set to apply its collision
            mask to colliding shapes
            */
            bool useCollisionMask() const { return m_useCollisionMask; }
            /*!
            \brief Get the current collision filter settings for this
            affectors collision mask
            */
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