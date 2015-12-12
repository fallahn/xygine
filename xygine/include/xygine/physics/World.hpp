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

#include <Box2D/Common/b2Math.h>
#include <Box2D/Dynamics/b2World.h>

#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>

#include <memory>
#include <functional>

namespace xy
{
    namespace Physics
    {
        class World final
        {
            friend class RigidBody;
            using Ptr = std::unique_ptr<b2World>;
        public:
            World()
            {
                XY_ASSERT(!m_world, "Physics world already created");
                m_world = std::make_unique<b2World>(m_gravity);
                //update = std::bind(&updateReal, std::placeholders::_1);
                update = [this](float dt)
                {
                    XY_ASSERT(m_world, "Physics world has not been created");
                    m_world->Step(dt, m_velocityIterations, m_positionIterations);
                };

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
        private:

            static float m_worldScale;
            static b2Vec2 m_gravity;
            static sf::Uint32 m_velocityIterations;
            static sf::Uint32 m_positionIterations;

            static Ptr m_world;

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
        };
    }
}

#endif //XY_PHYSICS_WORLD_HPP_