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

//creates a hinged joint allowing two bodies to revolve around a single point

#ifndef XY_HINGE_JOINT_HPP_
#define XY_HINGE_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2RevoluteJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Creates a hinged joint allowing two bodies to revolve around a single point
        */
        class XY_EXPORT_API HingeJoint final : public Joint
        {
        public:
            /*!
            \brief Constructor
            \param bodyA First body to which to attach to this joint
            \param worldAnchorPos Position in world coordinates around which bodies are hinged
            */
            HingeJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos);
            ~HingeJoint() = default;
            HingeJoint(const HingeJoint&) = default;
            HingeJoint& operator = (const HingeJoint&) = default;

            /*!
            \brief Returns the type of this joint
            */
            Joint::Type type() const override { return Joint::Type::Hinge; }
            /*!
            \brief Sets whether or not the joined bodies can collide with each other
            
            Cannot be set once both bodies have been joined
            */
            void canCollide(bool) override;
            /*!
            \brief Returns true if joined bodies can collide with each other
            */
            bool canCollide() const override;

            /*!
            \brief Get the joint angle in degrees
            */
            float getJointAngle() const;
            /*!
            \brief Get the joint speed in degrees per second
            */
            float getJointSpeed() const;
            /*!
            \brief Set whether or not the joint limit is enabled

            When joint limits are enabled the hinge will only rotate as
            far as the upper and lower limits
            */
            void limitEnabled(bool);
            /*!
            \brief Returns true if the joint's limit is enabled
            */
            bool limitEnabled() const;
            /*!
            \brief Get the joint's lower limit in degrees
            */
            float getLowerLimit() const;
            /*!
            \brief Get the joint's upper limit in degrees
            */
            float getUpperLimit() const;
            /*!
            \brief Set the joint's upper and lower limits in degrees

            When limitEnabled is true the hinge will only rotate as far
            as the upper and lower limits set
            */
            void setLimits(float upper, float lower);
            /*!
            \brief Set whether or not the motor is enabled

            When the motor is enabled the motor will attempt to rotate
            the hinge at the current motor speed, prividing the maximum
            motor torque allows it (depening on the total mass of the
            attached bodies)
            */
            void motorEnabled(bool);
            /*!
            \brief Returns true if the motor is enabled
            */
            bool motorEnabled() const;
            /*!
            \brief Sets the motor speed in degrees per second
            \see motorEnabled

            Use a negative speed to rotate the motor in the opposite
            direction.
            */
            void setMotorSpeed(float);
            /*!
            \brief Gets the current speed of the motor in degrees per second
            */
            float getMotorSpeed() const;
            /*!
            \brief Set the max motor torque

            The greater the torque the greater the mass the motor
            will be able to rotate
            */
            void setMaxMotorTorque(float);
            /*!
            \brief Gets the maximum motor torque
            \see setMaxMotorToque
            */
            float getMaxMotorTorque() const;


        private:
            const b2JointDef* getDefinition() override;

            b2RevoluteJointDef m_definition;
            b2Vec2 m_anchor;
        };
    }
}

#endif //XY_HINGE_JOINT_HPP_