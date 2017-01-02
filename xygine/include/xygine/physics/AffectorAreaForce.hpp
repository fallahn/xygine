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
        /*!
        \brief Applies a constant force and torque to intersecting bodies
        
        Any bodies intersecting an area defined by the CollisionShape to which
        this affector belongs will have a constant force and torque applied to
        them.
        */
        class XY_EXPORT_API AreaForceAffector final : public Affector
        {
            friend class CollisionShape;
        public:
            /*!
            \brief Constructor

            \param force Vector representing the force to be applied to intersecting bodies
            \param torque Amount of torque to apply to intersecting bodies
            \param wake If set to true applying the force to sleeping bodies will wake them
            */
            AreaForceAffector(const sf::Vector2f& force, float torque = 0.f, bool wake = false);
            ~AreaForceAffector() = default;
            AreaForceAffector(const AreaForceAffector&) = default;
            AreaForceAffector& operator = (const AreaForceAffector&) = default;

            Affector::Type type() const override { return Affector::Type::AreaForce; }
            void apply(RigidBody*) override;
            /*!
            \brief Sets the force as a vector to be applied
            to the parent rigidbody each update

            \param force A vector representing the force to be applied
            to intersecting bodies
            */
            void setForce(const sf::Vector2f& force) { m_force = force; }
            /*!
            \brief Sets the torque to be applied to the parent body each frame

            \param torque Torque value
            */
            void setTorque(float torque) { m_torque = torque; }
            /*!
            \brief Sets whether or not to wake a sleeping body on contact
            */
            void setWake(bool wake) { m_wake = wake; }
            /*!
            \brief Set the amount of linear drag to apply to the body

            The amount of drag is proportional to the current velocity
            of the intersecting body. Valid values are 0, no drag, to
            1 where the drag applied is equally opposite to that of the
            current intersecting body velocity.

            \param float drag value in range 0-1
            */
            void setLinearDrag(float);
            /*!
            \brief Set the amount of angular drag to apply to the body

            The amount of drag is proportional to the current angular
            velocity of the the intersecting body. Valid values are 0,
            no drag, to 1 where the dragapplied is exactly opposite to
            that or the current intersecting body's angular velocity

            \param float drag value in range 0-1
            */
            void setAngularDrag(float);
            /*!
            \brief Set whether or not to use a collision mask

            A collision mask can be used with this affector to define
            specific groups of intersecting entities which can be influenced
            by this affector.
            \see CollisionFilter
            \param m Set to true to use the collision mask 
            */
            void useCollisionMask(bool m) { m_useMask = m; }
            /*!
            \brief Set the collision mask to use to filter out fixtures

            \see useCollisionMask
            \param cm The CollisionMask to use
            */
            void setCollisionMask(CollisionFilter& cm) { m_collisionMask = cm; }

            /*!
            \brief Returns the current force value
            */
            const sf::Vector2f& getForce() const { return m_force; }
            /*!
            \brief Returns the current torque value
            */
            float getTorque() const { return m_torque; }
            /*!
            \brief Returns true if this force should wake a sleeping body
            */
            bool getWake() const { return m_wake; }
            /*!
            \brief Returns the current amount of linear drag to be applied
            */
            float getLinearDrag()const { return m_linearDrag; }
            /*!
            \brief Returns the current amount of angular drag to be applied
            */
            float getAngluarDrag()const { return m_angularDrag; }
            /*!
            \brief Returns true if this filter is set to use its collision mask
            */
            bool useCollisionMask() const { return m_useMask; }
            /*!
            \brief Returns the affectors current collision mask
            */
            const CollisionFilter& getCollisionMask() const { return m_collisionMask; }

        private:
            sf::Vector2f m_force;
            sf::Vector2f m_targetPoint;
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