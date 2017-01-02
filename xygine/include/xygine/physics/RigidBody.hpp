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

//creates a rigidbody component allowing the entity to be updated
//by the physics world. A valid physics world object must exist when
//these components are added to an entity, and require shape
//fixtures attached to them to have any affect. Entities with a rigid
//body attached will be unable to have their position set manually as 
//it will be automatically overriden by the physics simulation.

#ifndef XY_PHYSICS_RIGIDBODY_HPP_
#define XY_PHYSICS_RIGIDBODY_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/physics/CollisionShape.hpp>
#include <xygine/physics/Joint.hpp>
#include <xygine/physics/World.hpp>

#include <Box2D/Dynamics/b2Body.h>

#include <vector>
#include <memory>

using namespace std::placeholders;

namespace xy
{
    namespace Physics
    {
        enum class BodyType
        {
            Static = b2_staticBody,
            Dynamic = b2_dynamicBody,
            Kinematic = b2_kinematicBody
        };

        /*!
        \brief Creates a rigidbody component allowing the entity to be updated
        by the physics world. 
        
        A valid physics world object must exist when these components are added
        to an entity, and require shape fixtures attached to them to have any 
        affect. Entities with a RigidBody attached will be unable to have their 
        position set manually as it will be automatically overriden by the physics
        simulation.
        */
        class XY_EXPORT_API RigidBody final : public Component
        {
            friend class DistanceJoint;
            friend class FrictionJoint;
            friend class HingeJoint;
            friend class SliderJoint;
            friend class WheelJoint;
            friend class RopeJoint;
            friend class WeldJoint;
            friend class MotorJoint;
        public:
            using Ptr = std::unique_ptr<RigidBody>;

            RigidBody(MessageBus&, BodyType);
            ~RigidBody() = default;

            RigidBody(const RigidBody&) = delete;
            const RigidBody& operator = (const RigidBody&) = delete;

            Component::Type type() const override
            {
                return Component::Type::Physics;
            }

            void entityUpdate(Entity&, float) override;
            void onStart(Entity&) override;
            void destroy() override;

            /*!
            \brief Set the linear velocity of the body in world coordinates
            */
            void setLinearVelocity(const sf::Vector2f&);
            /*!
            \brief Set the angular velocity of the body
            */
            void setAngularVelocity(float);
            /*!
            \brief Set the linear damping which will be applied to this body
            */
            void setLinearDamping(float);
            /*!
            \brief Set the angular damping which will be applied to this body
            */
            void setAngularDamping(float);
            /*!
            \brief Set allowing this body to sleep when inactive
            */
            void allowSleep(bool);
            /*!
            \brief Set whether or not this body is awake initially
            */
            void awakeOnSpawn(bool);
            /*!
            \brief Set whether or not this body can rotate
            */
            void fixedRotation(bool);
            /*!
            Set this to true if this body is expected to be a fast moving object to reduce tunneling.
            
            This can be process intensive so should be used sparingly
            */
            void isBullet(bool);
            /*!
            \brief Sets whether or not this body is active on spawn
            */
            void activeOnSpawn(bool);
            /*!
            \brief Sets the scale of the gravity applied to this body
            
            Negative values can be used
            */
            void setGravityScale(float);
            /*!
            \brief Adds a collision shape to this body. 
            
            The original shape is unmodified (so it can be attached to 
            multiple bodies) and a pointer to the newly created shape is returned.
            Be aware that this pointer will become invalid should the body be destroyed
            \returns Pointer to newly created collision shape attached to this body.
            */
            template <typename T>
            T* addCollisionShape(const T& cs)
            {
                static_assert(std::is_base_of<CollisionShape, T>::value, "Can only add shapes of collision type");
                m_collisionShapes.emplace_back(std::make_unique<T>(cs));
                auto& newShape = m_collisionShapes.back();
                if (m_body)
                {
                    newShape->m_fixture = m_body->CreateFixture(&newShape->m_fixtureDef);
                    newShape->m_fixture->SetUserData(newShape.get());
                    newShape->destructionCallback = std::bind(&RigidBody::removeCollisionShape, this, _1, true);
                    newShape->registerCallbacks();
                }
                else
                {
                    m_pendingShapes.push_back(newShape.get());
                }
                return dynamic_cast<T*>(newShape.get());
            }

            /*!
            \brief Adds a joint to this body.
            
            The rigid body with which the joint was constructed must not be the same as this one.
            Returns a pointer to the created joint. Be aware this pointer will become invalid if
            either attached bodies are destroyed.
            \returns Pointer to newly created attachment
            */
            template <typename T>
            T* addJoint(const T& joint)
            {
                static_assert(std::is_base_of<Joint, T>::value, "Must be of Joint type");
                
                XY_ASSERT(joint.getRigidBodyA() != this, "Cannot attach body to self");

                m_joints.emplace_back(std::make_unique<T>(joint));
                auto& newJoint = m_joints.back();
                newJoint->setRigidBodyB(this);
                if (m_body)
                {
                    newJoint->m_joint = m_body->GetWorld()->CreateJoint(newJoint->getDefinition());
                    newJoint->m_joint->SetUserData(newJoint.get());
                    newJoint->destructionCallback = std::bind(&RigidBody::removeJoint, this, _1, true);
                }
                else
                {
                    m_pendingJoints.push_back(newJoint.get());
                }
                return dynamic_cast<T*>(newJoint.get());
            }

            /*!
            \brief Get the world position of this body's centre of mass
            */
            sf::Vector2f getWorldCentre() const;
            /*!
            \brief Get the local position of this body's centre of mass
            */
            sf::Vector2f getLocalCentre() const;
            /*!
            \brief Get the linear velocity of this body
            */
            sf::Vector2f getLinearVelocity() const;
            /*!
            \brief Get the angular velocity of this body
            */
            float getAngularVelocity() const;
            /*!
            \brief Get the linear damping of this body
            */
            float getLinearDamping() const;
            /*!
            \brief Get the angular damping of this body
            */
            float getAngularDamping() const;
            /*!
            \brief Get the gravity scale of this body
            */
            float getGravityScale() const;
            /*!
            \brIef Returns true if the isBullet property is set
            */
            bool isBullet() const;
            /*!
            \brief Returns true if allowSleep flag is set
            */
            bool allowSleep() const;
            /*!
            \brief Returns true if this body has fixed rotation
            */
            bool fixedRotation() const;
            /*!
            \brief Get the mass, in kilograms, of this body
            */
            float getMass() const;
            /*!
            \brief Get the inertia of this body
            */
            float getInertia() const;

            /*!
            \brief Set whether or not this body is awake
            */
            void awake(bool);
            /*!
            \brief Returns true if this body is awake
            */
            bool awake() const;
            /*!
            \brief Set whether or not this body is active
            */
            void active(bool);
            /*!
            \brief Returns true if this body is active
            */
            bool active() const;

            /*!
            \brief Applies a given force to this body at the given point

            If wake is true sleeping bodies will first be woken
            */
            void applyForce(const sf::Vector2f& force, const sf::Vector2f& point, bool wake = true);
            /*!
            \brief Applies a given force to the centre of mass of this body

            If wake is true then sleeping bodies will be woken
            */
            void applyForceToCentre(const sf::Vector2f& force, bool wake = true);
            /*!
            Applies a given torque (rotation) to this body

            If wake is true then a sleeping body will be woken
            */
            void applyTorque(float, bool wake = true);
            /*!
            \brief Applies a given linear impulse at a given point to the body

            If wake is true then this body will be woken when sleeping
            */
            void applyLinearImpulse(const sf::Vector2f& impulse, const sf::Vector2f& point, bool wake = true);
            /*!
            \brief Applies an angular impulse to the body.
            
            If wake is true the body will be woken if it is sleeping
            */
            void applyAngularImpulse(float, bool wake = true);

            /*!
            \brief Returns a vector of CollisionShapes attached to his body
            */
            const std::vector<CollisionShape::Ptr>& getCollisionShapes() const;
            /*!
            \brief Returns a vector of Joints attached to this body
            */
            const std::vector<Joint::Ptr>& getJoints() const;

        private:

            b2BodyDef m_bodyDef;
            b2Body* m_body;

            std::vector<CollisionShape::Ptr> m_collisionShapes;
            std::vector<CollisionShape*> m_pendingShapes;

            std::vector<Joint::Ptr> m_joints;
            std::vector<Joint*> m_pendingJoints;

            void removeJoint(const Joint*, bool);
            void removeCollisionShape(const CollisionShape*, bool);

            static void messageHandler(Component*, const Message&);
        };
    }
}

#endif //XY_PHYSICS_RIGIDBODY
