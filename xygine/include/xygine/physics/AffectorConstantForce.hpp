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

//applies a constant force to its parent body

#ifndef XY_AFFECTOR_CONSTANT_HPP_
#define XY_AFFECTOR_CONSTANT_HPP_

#include <xygine/physics/Affector.hpp>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class CollisionShape;
        /*!
        \brief Applies a constant force to the parent body
        of the owning CollisionShape
        */
        class XY_EXPORT_API ConstantForceAffector final : public Affector
        {
            friend class CollisionShape;
        public:

            ConstantForceAffector(const sf::Vector2f& force, float torque = 0.f, bool wake = false);
            ~ConstantForceAffector() = default;

            ConstantForceAffector(const ConstantForceAffector&) = default;
            ConstantForceAffector& operator = (const ConstantForceAffector&) = default;

            Affector::Type type() const override { return Affector::Type::ConstantForce; }
            void apply(RigidBody*) override;

            /*!
            \brief Sets the force as a vector to be applied
            to the parent rigidbody each update

            \param sf::Vector2f A vector representing the direction
            and magnitude of the force to be applied
            */
            void setForce(const sf::Vector2f&);
            /*!
            \brief Sets the torque to be applied to the parent
            body each frame

            \param float Amount of torque to apply
            */
            void setTorque(float);
            /*!
            \brief Sets whether or not to wake a sleeping body
            
            \param bool Set true to wake sleeping bodies
            */
            void setWake(bool);
            /*!
            \brief Sets if the force should be applied to the
            centre of mass of the collision shape or
            the centre of the collision shape's parent

            \param Centroid Either Centroid::RigidBody or Centroid::CollisionShape
            */
            void setCentre(Centroid);

            /*!
            \brief Returns the current force value
            */
            const sf::Vector2f& getForce() const;
            /*!
            \brief Returns the current torque value
            */
            float getTorque() const;
            /*!
            \brief Returns true if this force should wake
            a sleeping body
            */
            bool getWake() const;
            /*!
            \brief Returns whether or not this force is set
            to be applied at the centre of mass of
            the collision shape or its parent body
            */
            Centroid getCentre() const;

        private:
            sf::Vector2f m_force;
            float m_torque;
            bool m_wake;
            Centroid m_centroid;
            CollisionShape* m_parentShape;
        };
    }
}

#endif //XY_AFFECTOR_CONSTANT_HPP_