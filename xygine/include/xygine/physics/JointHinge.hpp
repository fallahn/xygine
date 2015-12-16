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

//creates a hinged joint allowing up to two bodies to revolve around a single point

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
        class HingeJoint : public Joint
        {
        public:
            HingeJoint(const RigidBody& bodyA, const sf::Vector2f& worldAnchorPos);
            ~HingeJoint() = default;
            HingeJoint(const HingeJoint&) = default;
            HingeJoint& operator = (const HingeJoint&) = default;

            //returns the type of this joint
            Joint::Type type() const override { return Joint::Type::Hinge; }
            //sets whether or not the joined bodies can collide with each other
            //cannot be set once both bodies have been joined
            void canCollide(bool) override;
            //returns true if joined bodies can collide with each other
            bool canCollide() const override;

            //TODO add property functions

        private:
            const b2JointDef* getDefinition() override;

            b2RevoluteJointDef m_definition;
            b2Vec2 m_anchor;
        };
    }
}

#endif //XY_HINGE_JOINT_HPP_