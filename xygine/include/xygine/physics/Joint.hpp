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

//abstract base class for joint types

#ifndef XY_JOINT_HPP_
#define XY_JOINT_HPP_

#include <Box2D/Dynamics/Joints/b2Joint.h>

#include <type_traits>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        class Joint
        {
            friend class RigidBody;
        public:
            enum class Type
            {
                Distance = b2JointType::e_distanceJoint,
                Friction = b2JointType::e_frictionJoint,
                Hinge = b2JointType::e_revoluteJoint,
                Slider = b2JointType::e_prismaticJoint,
                Wheel = b2JointType::e_wheelJoint,
                Rope = b2JointType::e_ropeJoint,
                Weld = b2JointType::e_weldJoint,
                Motor = b2JointType::e_motorJoint
            };

            Joint() : m_joint(nullptr), m_bodyA(nullptr), m_bodyB(nullptr){}
            virtual ~Joint() = default;

            virtual Type type() const = 0;
            virtual void canCollide(bool) = 0;
            virtual bool canCollide() const = 0;

        protected:

            virtual const b2JointDef* getDefinition() = 0;

            void setRigidBodyA(const RigidBody* b) { m_bodyA = b; }
            void setRigidBodyB(const RigidBody* b) { m_bodyB = b; }
            const RigidBody* getRigidBodyA() const { return m_bodyA; }
            const RigidBody* getRigidBodyB() const { return m_bodyB; }

            template <typename T>
            T* getJointAs() const
            {
                static_assert(std::is_base_of<b2Joint, T>::value, "Cannot cast to this type");
                return dynamic_cast<T*>(m_joint);
            }

        private:
            b2Joint* m_joint;
            const RigidBody* m_bodyA;
            const RigidBody* m_bodyB;
        };
    }
}

#endif //XY_JOINT_HPP_
