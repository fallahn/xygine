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

//maintains the physics world instance as well as unit conversion functions

#ifndef XY_PHYSICS_WORLD_HPP_
#define XY_PHYSICS_WORLD_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Config.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Color.hpp>

#include <Box2D/Common/b2Math.h>
#include <Box2D/Dynamics/b2World.h>
#include <Box2D/Dynamics/b2WorldCallbacks.h>

#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>

#include <xygine/physics/DebugDrawer.hpp>
#include <xygine/physics/Contact.hpp>

#include <memory>
#include <functional>
#include <list>

namespace xy
{
    class MessageBus;
    /*!
    \brief Classes relating to xygine's physics binding
    */
    namespace Physics
    {
        class Joint;
        class CollisionShape;
        /*!
        \brief Physics World

        The physics world is home to all physics simulation. A single instance
        of the physics world must exist in the same scope as any scene which
        has entities which physics components attached to them.
        */
        class XY_EXPORT_API World final : public sf::Drawable
        {
            friend class RigidBody;
            friend class CollisionShape;
            friend class CollisionCircleShape;
            friend class CollisionRectangleShape;
            friend class CollisionPolygonShape;
            friend class CollisionEdgeShape;
            friend class DebugDraw;
            friend class DistanceJoint;
            friend class FrictionJoint;
            friend class HingeJoint;
            friend class SliderJoint;
            friend class WheelJoint;
            friend class RopeJoint;
            friend class WeldJoint;
            friend class MotorJoint;
            friend class ConstantForceAffector;
            friend class PointForceAffector;

            using Ptr = std::unique_ptr<b2World>;
        public:
            explicit World(MessageBus& mb);
            ~World();
            
            World(const World&) = delete;
            const World& operator = (const World&) = delete;

            /*!
            \brief Set the physics world gravity in pixels per second per second

            Default is (0.f, 980.f) - approximately earth's gravitational pull
            when used with the default world scale
            */
            void setGravity(const sf::Vector2f& gravity);

            /*!
            \brief Sets the number of pixels drawn per metre in the physics world

            Recommended values are between 10 and 1000, default is 100 pixels per metre
            */
            void setPixelScale(float scale);

            /*!
            \brief Sets the number of velocity iterations perform per physics step. 
            Default is 6
            */
            void setVelocityIterationCount(sf::Uint32 count);

            /*!
            \brief Sets the number of position iterations performed per physics step.
            Default is 2
            */
            void setPositionIterationCount(sf::Uint32 count);

            /*!
            \brief Performs a single physics step.
            
            This is automatically called by xygine and should not be called elsewhere
            from within an application.
            */
            static std::function<void(float)> update;


            using JointDestroyedCallback = std::function<void(const Joint&)>;
            using CollisionShapeDestroyedCallback = std::function<void(const CollisionShape&)>;
            using ContactCallback = std::function<void(Contact&)>;
            using CallbackIndex = std::list<ContactCallback>::iterator;

            /*!
            \brief Adds a callback to be performed each time a joint is destroyed
            */
            void addJointDestroyedCallback(const JointDestroyedCallback&);
            /*!
            \brief Adds a callback to be performed each time a collision shape is destroyed
            */
            void addCollisionShapeDestroyedCallback(const CollisionShapeDestroyedCallback&);
            /*!
            \brief Adds a callback to the pre-solve listener.            
            \returns Callback index so that callback my be unregistered again
            */
            CallbackIndex addContactPreSolveCallback(const ContactCallback&);
            /*!
            \brief Adds a callback to the post-solve listener. 
            \returns Callback index so callback my be unregistered again
            */
            CallbackIndex addContactPostSolveCallback(const ContactCallback&);
            /*!
            \brief Adds a callback to the contact begin listener.
            \returns Callback index so callback my be unregistered again
            */
            CallbackIndex addContactBeginCallback(const ContactCallback&);
            /*!
            \brief Adds a callback to the contact end listener.
            \returns Callback index so callback my be unregistered again
            */
            CallbackIndex addContactEndCallback(const ContactCallback&);
            /*!
            \brief Unregisters the presolve callback at the given index
            */
            void removeContactPreSolveCallback(CallbackIndex);
            /*!
            \brief Unregisters the post solve callback at the given index
            */
            void removeContactPostSolveCallback(CallbackIndex);
            /*!
            \brief Unregisters the contact begin callback at the given index
            */
            void removeContactBeginCallback(CallbackIndex);
            /*!
            \brief Unregisters the contact end callback at the given index
            */
            void removeContactEndCallback(CallbackIndex);
        private:

            class ContactListener final : public b2ContactListener
            {
            public:
                explicit ContactListener(MessageBus& mb);
                ~ContactListener() = default;
                ContactListener(const ContactListener&) = delete;
                const ContactListener& operator = (const ContactListener&) = delete;

                void BeginContact(b2Contact* contact) override;
                void EndContact(b2Contact* contact) override;
                void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
                void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

                CallbackIndex addContactPreSolveCallback(const ContactCallback&);
                CallbackIndex addContactPostSolveCallback(const ContactCallback&);
                CallbackIndex addContactBeginCallback(const ContactCallback&);
                CallbackIndex addContactEndCallback(const ContactCallback&);

                void removeContactPreSolveCallback(CallbackIndex);
                void removeContactPostSolveCallback(CallbackIndex);
                void removeContactBeginCallback(CallbackIndex);
                void removeContactEndCallback(CallbackIndex);

                void resetBuffers();
            private:
                MessageBus& m_messageBus;
                Contact m_currentContact;

                std::list<ContactCallback> m_preSolveCallbacks;
                std::list<ContactCallback> m_postSolveCallbacks;
                std::list<ContactCallback> m_beginCallbacks;
                std::list<ContactCallback> m_endCallbacks;

                std::vector<std::uint8_t> m_contactBuffer;
                std::uint8_t* m_contactBufferPtr;

                std::vector<Contact> m_messageContacts;
                std::size_t m_messageIndex;

                b2Contact* bufferContact(const b2Contact*);
            }m_contactListener;

            class DestructionListener final : public b2DestructionListener
            {
            public:
                explicit DestructionListener(MessageBus& mb) : m_messageBus(mb) {}
                ~DestructionListener() = default;
                DestructionListener(const DestructionListener&) = delete;
                DestructionListener& operator = (const DestructionListener&) = delete;

                void SayGoodbye(b2Joint*) override;
                void SayGoodbye(b2Fixture*) override;

                void addCallback(const JointDestroyedCallback&);
                void addCallback(const CollisionShapeDestroyedCallback&);

            private:
                MessageBus& m_messageBus;
                std::vector<JointDestroyedCallback> m_jointCallbacks;
                std::vector<CollisionShapeDestroyedCallback> m_collisionShapeCallbacks;
            }m_destructionListener;

            static Ptr m_world;
            //ugh I can't believe I resorted to this
            static World* m_instance;

            mutable std::unique_ptr<DebugDraw> m_debugDraw;

            //unit conversion functions. Made private because we don't
            //need to expose them to the overall API
            static b2Vec2 sfToBoxVec(const sf::Vector2f& vec);

            static sf::Vector2f boxToSfVec(const b2Vec2& vec);

            static float sfToBoxFloat(float val);

            static float boxToSfFloat(float val);

            static inline float sfToBoxAngle(float degrees)
            {
                return -degrees * 0.0174533f;
            }

            static inline float boxToSfAngle(float rads)
            {
                return -rads * 57.29578f;
            }

            static inline b2Color sfToBoxColour(const sf::Color& colour)
            {
                return
                {
                    static_cast<float>(colour.r) / 255.f,
                    static_cast<float>(colour.g) / 255.f,
                    static_cast<float>(colour.b) / 255.f,
                    static_cast<float>(colour.a) / 255.f
                };
            }

            static inline sf::Color boxToSfColour(const b2Color& colour)
            {
                return
                {
                    static_cast<sf::Uint8>(colour.r * 255.f),
                    static_cast<sf::Uint8>(colour.g * 255.f),
                    static_cast<sf::Uint8>(colour.b * 255.f),
                    static_cast<sf::Uint8>(colour.a * 255.f)
                };
            }

            float getWorldScale() const;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}

#endif //XY_PHYSICS_WORLD_HPP_