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
        /*!
        \brief Creates a joint which allows two bodies to translate along a single axis in opposite directions
        */
        class XY_EXPORT_API SliderJoint final : public Joint
        {
        public:
            /*!
            \brief Construtor
            \param bodyA First body to which this joint is attached
            \param worldAnchorPos Position in world coordinates at which this joint is connected to bodyA
            \param slideVector direction in which the second body is moved relative to the world anchor point
            */
            SliderJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos, const sf::Vector2f& slideVector);
            ~SliderJoint() = default;
            SliderJoint(const SliderJoint&) = default;
            SliderJoint& operator = (const SliderJoint&) = default;

            /*!
            \brief Returns the type of this joint
            */
            Joint::Type type() const override { return Joint::Type::Slider; }
            /*!
            \brief Sets whether or not the two bodies can collide with each other

            Has no effect once both bodies are joined
            */
            void canCollide(bool) override;
            /*!
            \brief Returns true if joined bodies can collide with each other
            */
            bool canCollide() const override;

            /*!
            \brief Returns the distance which the joint has slid, in pixels
            */
            float getTranslation() const;
            /*!
            \brief Returns the current sliding speed, in pixels per second
            */
            float getSpeed() const;
            /*!
            \brief Set whether or not the slide limit is enabled

            If the slide limiti is enable the attached bodies will slide only
            as far as the upper and lower limit from the anchor point
            */
            void limitEnabled(bool);
            /*!
            \brief Returns true if the slide limit is enabled
            */
            bool limitEnabled() const;
            /*!
            \brief Returns the lower limit distance, in pixels, from the anchor point
            */
            float getLowerLimit() const;
            /*!
            \brief Returns the upper limit distance, in pixels, from the anchor point
            */
            float getUpperLimit() const;
            /*!
            \brief Set the upper and lower limits of the slider in pixels
            \see limitEnabled
            */
            void setLimits(float upper, float lower);
            /*!
            \brief Set whether or not the motor is enabled

            When the motor is enable the joint will attempt to move at
            the current set speed up to any limits if they are enabled.
            The motors actual movement speed is affected by the amount
            of force it has and the mass of the bodies attached to the joint.
            A higher forcee is required to move a greater mass.
            */
            void motorEnabled(bool);
            /*!
            \brief Returns true if the motor is enabled
            */
            bool motorEnabled() const;
            /*!
            \brief Set the motor speed, in pixels per second
            \see motorEnabled
            */
            void setMotorSpeed(float);
            /*!
            \brief Gets the current motor speed in pixels per second
            */
            float getMotorSpeed() const;
            /*!
            \brief Set the maximum motor force
            \see motorEnabled
            */
            void setMaxMotorForce(float);
            /*!
            \brief Gets the maximum motor force
            */
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