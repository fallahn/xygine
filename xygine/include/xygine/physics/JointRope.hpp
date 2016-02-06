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

//constrains two bodies to a maximum distance. Changing the maximum
//length of the rope once it is attached to both bodies is not supported

#ifndef XY_ROPE_JOINT_HPP_
#define XY_ROPE_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2RopeJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Constrains two bodies to a maximum distance from each other

        NOTE length of the rope cannot be updated once two bodies are connected
        */
        class XY_EXPORT_API RopeJoint final : public Joint
        {
        public:
            /*!
            \brief Constructor
            \param bodyA First body to attach to this joint
            \param worldAnchorPosA Position in world coordinates at which to attach the joint on bodyA
            \param worldAnchorPosB Position in world coordinates at which to attach the joint on bodyB

            The maximum rope length is the distance between the two world anchor points
            */
            RopeJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPosA, const sf::Vector2f& worldAnchorPosB);
            ~RopeJoint() = default;
            RopeJoint(const RopeJoint&) = default;
            RopeJoint& operator = (const RopeJoint&) = default;

            /*!
            \brief Returns the type of this joint
            */
            Joint::Type type() const override { return Joint::Type::Rope; }
            /*!
            \brief Sets whether or not the two attached bodies can collide with
            each other.
            
            Changing this once both bodies are attached has no effect.
            */
            void canCollide(bool) override;
            /*!
            \brief Returns true if joined bodies can collide with each other
            */
            bool canCollide() const override;

            /*!
            \brief Sets the maximum length of the rope, in pixels.
            
            Changing this once both bodies are attached has no effect
            */
            void setMaximumLength(float);
            /*!
            \brief Returns the maximum length of the rope, in pixels
            */
            float getMaximumLength() const;

        private:
            b2RopeJointDef m_definition;
            b2Vec2 m_worldAnchorA;
            b2Vec2 m_worldAnchorB;

            b2JointDef* getDefinition() override;
        };
    }
}

#endif //XY_ROPE_JOINT_HPP_