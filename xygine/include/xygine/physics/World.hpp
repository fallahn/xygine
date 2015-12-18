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

namespace xy
{
    class MessageBus;
    namespace Physics
    {
        class Joint;
        class CollisionShape;
        class World final : public sf::Drawable
        {
            friend class RigidBody;
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

            using Ptr = std::unique_ptr<b2World>;
        public:
            explicit World(MessageBus& mb)
                : m_contactListener(mb), m_destructionListener(mb)
            {
                XY_ASSERT(!m_world, "Physics world already created");
                m_world = std::make_unique<b2World>(m_gravity);
                
                update = [this](float dt)
                {
                    XY_ASSERT(m_world, "Physics world has not been created");
                    m_world->Step(dt, m_velocityIterations, m_positionIterations);
                };

                m_world->SetContactListener(&m_contactListener);
                m_world->SetDestructionListener(&m_destructionListener);

                LOG("CLIENT created physics world", Logger::Type::Info);
            }
            ~World()
            {
                m_world.reset();
                update = [](float) {};
                LOG("CLIENT destroyed physics world", Logger::Type::Info);
            }
            World(const World&) = delete;
            const World& operator = (const World&) = delete;

            //set the physics world gravity in pixels per second per second
            //default is (0.f, 980.f) - approximately earth's gravitational pull
            //when used with the default world scale
            inline void setGravity(const sf::Vector2f& gravity)
            {
                m_gravity = sfToBoxVec(gravity);
                if (m_world) m_world->SetGravity(m_gravity);
            }

            //sets the number of pixels drawn per metre in the physics world
            //recommended values are between 10 and 1000, default is 100 pixels per mtre
            inline void setPixelScale(float scale)
            {
                XY_ASSERT((scale > 5 && scale < 5000), "Reasonable scales range from 10 to 1000 pixels per metre");
                m_worldScale = scale;
            }

            //sets the number of velocity iterations perform per physics step. default is 6
            inline void setVelocityIterationCount(sf::Uint32 count)
            {
                m_velocityIterations = count;
            }

            //sets the number of position iterations performed per physics step. default is 2
            inline void setPositionIterationCount(sf::Uint32 count)
            {
                m_positionIterations = count;
            }

            //performs a single physics step. this is automatically called by xygine and should
            //not be called elsewhere from within an application.
            static std::function<void(float)> update;


            using JointDestroyedCallback = std::function<void(const Joint&)>;
            using CollisionShapeDestroyedCallback = std::function<void(const CollisionShape&)>;
            using ContactCallback = std::function<void(Contact&)>;

            //adds a callback to be performed each time a joint is destroyed
            void addJointDestroyedCallback(const JointDestroyedCallback&);
            //adds a callback to be performed each time a collision shape is destroyed
            void addCollisionShapeDestroyedCallback(const CollisionShapeDestroyedCallback&);
            //adds a callback to the pre-solve listener
            void addPreSolveCallback(const ContactCallback&);
            //adds a callback to the post-solve listener
            void addPostSolveCallback(const ContactCallback&);
            //adds a callback to the contact begin listener
            void addContactBeginCallback(const ContactCallback&);
            //adds a callback to the contact end listener
            void addContactEndCallback(const ContactCallback&);

        private:

            class ContactListener final : public b2ContactListener
            {
            public:
                explicit ContactListener(MessageBus& mb) : m_messageBus(mb) {}
                ~ContactListener() = default;
                ContactListener(const ContactListener&) = delete;
                const ContactListener& operator = (const ContactListener&) = delete;

                void BeginContact(b2Contact* contact) override;
                void EndContact(b2Contact* contact) override;
                void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
                void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

                void addPreSolveCallback(const ContactCallback&);
                void addPostSolveCallback(const ContactCallback&);
                void addContactBeginCallback(const ContactCallback&);
                void addContactEndCallback(const ContactCallback&);

            private:
                MessageBus& m_messageBus;
                Contact m_currentContact;

                std::vector<ContactCallback> m_preSolveCallbacks;
                std::vector<ContactCallback> m_postSolveCallbacks;
                std::vector<ContactCallback> m_beginCallbacks;
                std::vector<ContactCallback> m_endCallbacks;
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



            static float m_worldScale;
            static b2Vec2 m_gravity;
            static sf::Uint32 m_velocityIterations;
            static sf::Uint32 m_positionIterations;

            static Ptr m_world;

            mutable std::unique_ptr<DebugDraw> m_debugDraw;

            static inline b2Vec2 sfToBoxVec(const sf::Vector2f& vec)
            {
                return b2Vec2(vec.x / m_worldScale, -vec.y / m_worldScale);
            }

            static inline sf::Vector2f boxToSfVec(const b2Vec2& vec)
            {
                return sf::Vector2f(vec.x, -vec.y) * m_worldScale;
            }

            static inline float sfToBoxFloat(float val)
            {
                return val / m_worldScale;
            }

            static inline float boxToSfFloat(float val)
            {
                return val * m_worldScale;
            }

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

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}

#endif //XY_PHYSICS_WORLD_HPP_