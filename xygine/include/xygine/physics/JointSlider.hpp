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

//creates a joint which allows two bodies translate along a single axis
//in opposite directions

#ifndef XY_SLIDER_JOINT_HPP_
#define XY_SLIDER_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2PrismaticJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        class SliderJoint final : public Joint
        {
        public:
            SliderJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& slideVector);
            ~SliderJoint() = default;
            SliderJoint(const SliderJoint&) = default;
            SliderJoint& operator = (const SliderJoint&) = default;

            //returns the type of this joint
            Joint::Type type() const override { return Joint::Type::Slider; }
            //sets whether or not the two bodies can collide with each other
            //has no effect once bodies are joined
            void canCollide(bool) override;
            //returns true if joined bodies can collide with each other
            bool canCollide() const override;

            //returns the distance which the joint has slid, in pixels
            float getTranslation() const;
            //returns the current sliding speed, in pixels per second
            float getSpeed() const;
            //set whether or not the slide limit is enabled
            void limitEnabled(bool);
            //returns true if the slide limit is enabled
            bool limitEnabled() const;
            //returns the lower limit distance, in pixels, from the anchor point
            float getLowerLimit() const;
            //returns the upper limit distance, in pixels, from the anchor point
            float getUpperLimit() const;
            //set the upper and lower limits of the slider in pixels
            void setLimits(float upper, float lower);
            //set whether or not the motor is enabled
            void motorEnabled(bool);
            //returns true if the motor is enabled
            bool motorEnabled() const;
            //set the motor speed, in pixels per second
            void setMotorSpeed(float);
            //gets the current motor speed in pixels per second
            float getMotorSpeed() const;
            //set the maximum motor firce
            void setMaxMotorForce(float);
            //gets the maximum motor force
            float getMaxMotorForce() const;

        private:
            const b2JointDef* getDefinition() override;

            b2PrismaticJointDef m_definition;
            b2Vec2 m_anchor;
            b2Vec2 m_direction;
        };
    }
}

#endif //XY_SLIDER_JOINT_HPP_