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

//creates a motor joint to position or rotate a body to a specific target

#ifndef XY_MOTOR_JOINT_HPP_
#define XY_MOTOR_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2MotorJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        class MotorJoint final : public Joint
        {
        public:
            explicit MotorJoint(const RigidBody& rbA);
            ~MotorJoint() = default;
            MotorJoint(const MotorJoint&) = default;
            MotorJoint& operator = (const MotorJoint&) = default;

            //returs the type of this joint
            Joint::Type type() const override { return Joint::Type::Motor; }

            //set Whether or not the attached bodies can collide with each other
            //has no effect once two bodies are joint
            void canCollide(bool) override;
            //returns true if attached bodies can collide with each other
            bool canCollide() const override;

            //set the target offset between the bodies in pixels
            void setTargetLinearOffset(const sf::Vector2f&);
            //get the targt offset between the two bodies in pixels
            sf::Vector2f getTargetLinearOffset() const;
            //set the target angular offset of the bodies in degrees
            void setTargetAngularOffset(float);
            //gets the current target angular offset in degrees
            float getTargetAngularOffset() const;
            //sets the maximum friction force
            void setMaxForce(float);
            //gets the maximum friction force
            float getMaxForce() const;
            //set the maximum friction torque
            void setMaxTorque(float);
            //get the maximum friction torque value
            float getMaxTorque() const;
            //set the correction factor in range 0 - 1
            void setCorrectionFactor(float);
            //get the current correction factor
            float getCorrectionFactor() const;


        private:
            b2MotorJointDef m_definition;
            const b2JointDef* getDefinition() override;
        };
    }
}

#endif //XY_MOTOR_JOINT_HPP_