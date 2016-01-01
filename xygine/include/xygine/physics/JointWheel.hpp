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

//definies a wheel joint where the second body is attached to the first
//via sprung suspension, and can be rotated via motor

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
        class WheelJoint final : public Joint
        {
        public:
            WheelJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& axis);
            ~WheelJoint() = default;

            WheelJoint(const WheelJoint&) = default;
            WheelJoint& operator = (const WheelJoint&) = default;

            //get the type of this joint
            Joint::Type type() const { return Joint::Type::Wheel; }
            //set whether or not the joined bodies collide with each other
            //updating this once the joint is added to a rigidbody has n effect
            void canCollide(bool) override;
            //returns true if the joined bodies are allowed t ocollide
            bool canCollide() const override;

            //get joint translation, in pixels
            float getTranslation() const;
            //get joint speed in pixels per second
            float getSpeed() const;
            //set whether or not to enable the motor
            void motorEnabled(bool);
            //returns true if motor is enabled
            bool motorEnabled() const;
            //set the speed of the motor in degrees per second
            void setMotorSpeed(float);
            //get the motor speed in degrees per second
            float getMotorSpeed() const;
            //set the max torque of the motor
            void setMaxMotorTorque(float);
            //returns the max motor torque
            float getMaxMotorTorque() const;
            //set suspension spring frequency in hertz
            //a value of zero disables the spring.
            void setSpringFrequency(float);
            //get the frequency of the spring in hertz
            float getSpringFrequency() const;
            //set the damping ratio of the spring
            void setSpringDampingRatio(float);
            //get the spring's damping ratio
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