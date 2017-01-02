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

//defines a wheel joint where the second body is attached to the first
//via sprung suspension, and can be rotated via a motor

#ifndef XY_WHEEL_JOINT_HPP_
#define XY_WHEEL_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2WheelJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Defines a wheel joint where the second body is attached to the first
        via sprung suspension, and can be rotated via a motor.
        */
        class XY_EXPORT_API WheelJoint final : public Joint
        {
        public:
            /*!
            \brief Constructor
            \param bodyA First body to attach to the joint
            \param worldAnchorPos Position in world coordinates at which the second body is connected
            \param axis A vector realtive to the anchor at which the second body is placed
            */
            WheelJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& axis);
            ~WheelJoint() = default;

            WheelJoint(const WheelJoint&) = default;
            WheelJoint& operator = (const WheelJoint&) = default;

            /*!
            \brief Get the type of this joint
            */
            Joint::Type type() const { return Joint::Type::Wheel; }
            /*!
            \brief Set whether or not the joined bodies collide with each other

            Updating this once the joint is added to a second RigidBody has no effect
            */
            void canCollide(bool) override;
            /*!
            \brief Returns true if the joined bodies are allowed t ocollide
            */
            bool canCollide() const override;

            /*!
            \brief Get joint translation, in pixels
            */
            float getTranslation() const;
            /*
            \brief Get joint speed in pixels per second
            */
            float getSpeed() const;
            /*!
            \brief Set whether or not to enable the motor
            */
            void motorEnabled(bool);
            /*!
            \brief Returns true if motor is enabled
            */
            bool motorEnabled() const;
            /*!
            \brief Set the speed of the motor in degrees per second
            */
            void setMotorSpeed(float);
            /*!
            \brief Get the motor speed in degrees per second
            */
            float getMotorSpeed() const;
            /*!
            \brief Set the max torque of the motor
            */
            void setMaxMotorTorque(float);
            /*!
            \brief Returns the max motor torque
            */
            float getMaxMotorTorque() const;
            /*!
            \brief Set suspension spring frequency in hertz

            A value of zero disables the spring.
            */
            void setSpringFrequency(float);
            /*!
            \brief Get the frequency of the spring in hertz
            */
            float getSpringFrequency() const;
            /*!
            \brief Set the damping ratio of the spring
            */
            void setSpringDampingRatio(float);
            /*!
            \brief Get the spring's damping ratio
            */
            float getSpringDampingRatio() const;

        private:
            const b2JointDef* getDefinition() override;

            b2WheelJointDef m_definition;
            b2Vec2 m_anchor;
            b2Vec2 m_axis;
        };
    }
}

#endif //XY_WHEEL_JOINT_HPP_