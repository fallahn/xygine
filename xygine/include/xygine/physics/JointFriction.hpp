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

//creates a friction joint between two bodies, providing 2D translational
//and rotational friction. Useful for top-down scenarios where no gravity
//is used, such as a hokey puck or billiard ball

#ifndef XY_FRICTION_JOINT_HPP_
#define XY_FRICTION_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2FrictionJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        class FrictionJoint final : public Joint
        {
        public:
            FrictionJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos);
            ~FrictionJoint() = default;
            FrictionJoint(const FrictionJoint&) = default;
            FrictionJoint& operator = (const FrictionJoint&) = default;

            //returns the type of this joint
            Joint::Type type() const override { return Joint::Type::Friction; }
            //set whether the joined bodies can collide with each other
            //cannot be updated once the joint has been added to a rigidbody
            void canCollide(bool collide) override;
            //returns true if joined bodies are allowed to collide with each other
            bool canCollide() const override;
            //set the maximum amount of translational friction applied in newtons
            void setMaxFrictionForce(float);
            //get the current maximum amount of translational friction applied in newtons
            float getMaxFrictionForce() const;
            //set the maximum amount of rotational friction applied in newtonmetres
            void setMaxFrictionTorque(float);
            //get the current maximum amount of rotational friction applied
            float getMaxFrictionTorque() const;


        private:
            const b2JointDef* getDefinition() override;

            b2FrictionJointDef m_definition;
            b2Vec2 m_anchor;
        };
    }
}

#endif //XY_FRICTION_JOINT_HPP_