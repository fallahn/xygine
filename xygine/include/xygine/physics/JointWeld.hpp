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

//attempts to weld and fix two bodies together at a specific point

#ifndef XY_WELD_JOINT_HPP_
#define XY_WELD_JOINT_HPP_

#include <xygine/physics/Joint.hpp>
#include <Box2D/Dynamics/Joints/b2WeldJoint.h>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
        /*!
        \brief Attempts to weld and fix two bodies together at a specific point
        */
        class XY_EXPORT_API WeldJoint final : public Joint
        {
        public:
            /*!
            \brief Constructor
            \param bodyA First RigidBody to attach to the joint
            \param worldWeldPoint Position in world coordinates at which to weld the second body
            */
            WeldJoint(const RigidBody& bodyA, const sf::Vector2f& worldWeldPoint);
            ~WeldJoint() = default;
            WeldJoint(const WeldJoint&) = default;
            WeldJoint& operator = (const WeldJoint&) = default;

            /*!
            \brief Returns the type of this joint
            */
            Joint::Type type() const override { return Joint::Type::Weld; }
            /*!
            \brief Sets whether or not the attached bodies can collide

            This has no affect once two bodies are joined
            */
            void canCollide(bool) override;
            /*!
            \brief Returns true if the attached bodies can collide with each other
            */
            bool canCollide() const;

            /*!
            \brief Returns the reference angle between the bodies in degrees
            */
            float getReferenceAngle() const;
            /*!
            \brief Sets the frequency of the joint in hertz
            */
            void setFrequency(float);
            /*!
            \brief Sets the current frequency of the joint in hertz
            */
            float getFrequency() const;
            /*!
            \brief Set the damping ratio of the joint
            */
            void setDampingRatio(float);
            /*!
            \brief Get the current damping ratio of the joint
            */
            float getDampingRatio() const;

        private:

            b2JointDef* getDefinition() override;

            b2WeldJointDef m_definition;
            b2Vec2 m_anchor;
        };
    }
}

#endif //XY_WELDJOINT_HPP_