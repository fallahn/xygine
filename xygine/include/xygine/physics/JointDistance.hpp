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
        class DistanceJoint final : public Joint
        {
        public:
            DistanceJoint(const RigidBody& bodyA, const sf::Vector2f& worldPosA, const sf::Vector2f& worldPosB);
            ~DistanceJoint() = default;

            DistanceJoint(const DistanceJoint&) = delete;
            DistanceJoint& operator = (const DistanceJoint&) = delete;

            Joint::Type type() const override { return Joint::Type::Distance; }
            void canCollide(bool collide) override { m_definition.collideConnected = collide; }


            //TODO get/set freq etc

        private:
            const b2JointDef* getDefinition() override;

            b2DistanceJointDef m_definition;
            b2Vec2 m_anchorA;
            b2Vec2 m_anchorB;

            const RigidBody* m_bodyA;
            const RigidBody* m_bodyB;
        };
    }
}

#endif //XY_DISTANCE_JOINT_HPP_