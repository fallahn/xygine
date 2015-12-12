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

#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/World.hpp>

#include <xygine/Entity.hpp>

using namespace xy;
using namespace xy::Physics;

RigidBody::RigidBody(MessageBus& mb, BodyType type)
    : Component (mb, this),
    m_body      (nullptr)
{
    m_bodyDef.type = static_cast<b2BodyType>(type);
}

//public
void RigidBody::entityUpdate(Entity& entity, float dt)
{
    entity.setRotation(World::boxToSfAngle(m_body->GetAngle()));
    entity.setWorldPosition(World::boxToSfVec(m_body->GetPosition()));
}

void RigidBody::handleMessage(const Message&) {}

void RigidBody::onStart(Entity& entity)
{
    //set rotation / position from entity
    m_bodyDef.angle = World::sfToBoxAngle(entity.getRotation());
    m_bodyDef.position = World::sfToBoxVec(entity.getWorldPosition());

    m_body = World::m_world->CreateBody(&m_bodyDef);
    XY_ASSERT(m_body, "Failed to create physics body");
}

void RigidBody::destroy()
{
    Component::destroy();
    if (m_body)
    {
        m_body->GetWorld()->DestroyBody(m_body);
        m_body = nullptr;
    }
}

void RigidBody::setLinearVelocity(const sf::Vector2f& velocity)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.linearVelocity = World::sfToBoxVec(velocity);
}

void RigidBody::setAngularVelocity(float velocity)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.angularVelocity = World::sfToBoxAngle(velocity);
}

void RigidBody::setLinearDamping(float damping)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.linearDamping = damping;
}

void RigidBody::setAngularDamping(float damping)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.angularDamping = damping;
}

void RigidBody::allowSleep(bool allow)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.allowSleep = allow;
}

void RigidBody::awakeOnSpawn(bool awake)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.awake = awake;
}

void RigidBody::fixedRotation(bool fixed)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.fixedRotation = fixed;
}

void RigidBody::isBullet(bool bullet)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.bullet = bullet;
}

void RigidBody::activeOnSpawn(bool active)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.active = active;
}

void RigidBody::setGravityScale(float scale)
{
    XY_ASSERT(m_body = nullptr, "Body properties cannot be updated once attached to an entity");
    m_bodyDef.gravityScale = scale;
}

//private