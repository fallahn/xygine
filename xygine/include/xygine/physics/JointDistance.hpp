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

//creates a distance joint which maintains a fixed distance between two bodies

#ifndef XY_DISTANCE_JOINT_HPP_
#define XY_DISTANCE_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2DistanceJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Distance Joints maintain a fixed distance between two joined bodies
        */
        class XY_EXPORT_API DistanceJoint final : public Joint
        {
        public:
            /*!
            \brief Constructor
            \param bodyA The first body this joint should be attached to
            \param worldAnchorA position in world coordinates where this joint should attach to body A
            \param worldAnchorB position in world coordinates where this joint should attach to body B
            */
            DistanceJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorA, const sf::Vector2f& worldAnchorB);
            ~DistanceJoint() = default;

            DistanceJoint(const DistanceJoint&) = default;
            DistanceJoint& operator = (const DistanceJoint&) = default;

            /*!
            \brief Get the type of this joint
            */
            Joint::Type type() const override { return Joint::Type::Distance; }
            /*!
            \brief Set whether or not the joined bodies collide with each other

            Cannot be updated once joint is added to a second rigidbody
            */
            void canCollide(bool collide) override;
            /*!
            \brief Returns true if joined bodies can collide with each other
            */
            bool canCollide() const override;
            /*!
            \brief Get the length of the distance between two joined bodies
            */
            float getLength() const;
            /*!
            \brief Set the frequency, in hertz, of the constraint.

            Recommended to be less than half the physics update rate. Lower values
            soften the constraint allowing bodies to move closer
            */
            void setFrequency(float);
            /*!
            \brief Get the current frequency, in hertz, of the constraint
            */
            float getFrequency() const;
            /*!
            \brief Set the damping ratio of the constraint.

            Must be positive and usually not much more than 1
            */
            void setDampingRatio(float);
            /*!
            \brief Get the current damping ratio of the constraint
            */
            float getDampingRatio() const;

        private:
            const b2JointDef* getDefinition() override;

            b2DistanceJointDef m_definition;
            b2Vec2 m_anchorA;
            b2Vec2 m_anchorB;

        };
    }
}

#endif //XY_DISTANCE_JOINT_HPP_