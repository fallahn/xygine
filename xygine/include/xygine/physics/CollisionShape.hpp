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

//ABC for creating a shape which can be attached to a physics rigid body
//multiple collision shapes can be attached to a single body

#ifndef XY_COLLISION_SHAPE_HPP_
#define XY_COLLISION_SHAPE_HPP_

#include <xygine/physics/AffectorConstantForce.hpp>
#include <xygine/physics/AffectorAreaForce.hpp>
#include <xygine/physics/AffectorPointForce.hpp>
#include <xygine/physics/World.hpp>

#include <Box2D/Dynamics/b2Fixture.h>

#include <functional>
#include <vector>
#include <set>

namespace xy
{
    namespace Physics
    {
        class Contact;
        class CollisionShape
        {
            friend class RigidBody;
            friend class PointForceAffector;
        public:
            enum class Type
            {
                Circle,
                Box,
                Polygon,
                Edge
            };

            using Ptr = std::unique_ptr<CollisionShape>;

            CollisionShape();
            virtual ~CollisionShape();
            CollisionShape(const CollisionShape& other) = default;
            CollisionShape& operator = (const CollisionShape&) = default;

            virtual Type type() const = 0;

            //sets this collision shape's friction
            void setFriction(float);
            //sets this shape's restitution (bounciness)
            void setRestitution(float);
            //sets the denisty of this shape
            void setDensity(float);
            //sets whether or not this shape is used as a sensor
            //in collision detection
            void setIsSensor(bool);
            //adds a sets of filter flags to define which type of shapes
            //this shape may collide with
            void setFilter(CollisionFilter);
            //returns the currently set collision filter
            CollisionFilter getFilter() const;
            //allow explicitly destroying this shape. note that although
            //this raises an event on the message bus callbacks registered
            //with the physics world will not be invoked
            void destroy();

            //adds an affector which modifies the body to which
            //this or a colliding CollisionShape is attached
            void addAffector(const ConstantForceAffector&);
            void addAffector(const AreaForceAffector&);
            void addAffector(const PointForceAffector&);
            //removes all affectors assigned to this CollisionShape
            void clearAffectors();

        protected:
            const b2FixtureDef getFixtureDef() const
            {
                return m_fixtureDef;
            }

            void setShape(b2Shape& shape)
            {
                m_fixtureDef.shape = &shape;
            }

        private:
            b2FixtureDef m_fixtureDef;
            b2Fixture* m_fixture;

            std::vector<ConstantForceAffector> m_constForceAffectors;
            std::vector<AreaForceAffector> m_areaAffectors;
            std::vector<PointForceAffector> m_pointAffectors;

            using AffectorPair = std::pair<Affector*, RigidBody*>;
            std::vector<AffectorPair> m_activeAffectors;

            using PointTuple = std::tuple<PointForceAffector*, CollisionShape*, CollisionShape*>;
            std::vector<PointTuple> m_activePointAffectors;

            World::CallbackIndex m_beginCallbackIndex;
            World::CallbackIndex m_endCallbackIndex;
            bool m_removeCallbacks;

            void beginContactCallback(Contact&);
            void endContactCallback(Contact&);
            void preSolveContactCallback(Contact&);
            void registerCallbacks();

            void applyAffectors();
            void activateAffectors(CollisionShape*, CollisionShape*);
            void deactivateAffectors(CollisionShape*, CollisionShape*);

            std::function<void(const CollisionShape*)> destructionCallback;
        };
    }
}

#endif //XY_COLLISION_SHAPE_HPP_
