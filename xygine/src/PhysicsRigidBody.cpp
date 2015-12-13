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

    //check for any pending fixture creation
    for (auto s : m_pendingShapes)
    {
        s->m_fixture = m_body->CreateFixture(&s->m_fixtureDef);
    }
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
    m_bodyDef.linearVelocity = World::sfToBoxVec(velocity);
    if (m_body) m_body->SetLinearVelocity(m_bodyDef.linearVelocity);
}

void RigidBody::setAngularVelocity(float velocity)
{
    m_bodyDef.angularVelocity = velocity;
    if (m_body) m_body->SetAngularVelocity(velocity);
}

void RigidBody::setLinearDamping(float damping)
{
    XY_ASSERT(damping >= 0, "Damping value must be positive");
    m_bodyDef.linearDamping = damping;
    if (m_body) m_body->SetLinearDamping(damping);
}

void RigidBody::setAngularDamping(float damping)
{
    XY_ASSERT(damping >= 0, "Damping value must be positive");
    m_bodyDef.angularDamping = damping;
    if (m_body) m_body->SetAngularDamping(damping);
}

void RigidBody::allowSleep(bool allow)
{
    m_bodyDef.allowSleep = allow;
    if (m_body) m_body->SetSleepingAllowed(allow);
}

void RigidBody::awakeOnSpawn(bool awake)
{
    m_bodyDef.awake = awake;
    if (m_body) m_body->SetAwake(awake);
}

void RigidBody::fixedRotation(bool fixed)
{
    m_bodyDef.fixedRotation = fixed;
    if (m_body) m_body->SetFixedRotation(fixed);
}

void RigidBody::isBullet(bool bullet)
{
    m_bodyDef.bullet = bullet;
    if (m_body) m_body->SetBullet(bullet);
}

void RigidBody::activeOnSpawn(bool active)
{
    m_bodyDef.active = active;
    if (m_body) m_body->SetActive(active);
}

void RigidBody::setGravityScale(float scale)
{
    m_bodyDef.gravityScale = scale;
    if (m_body) m_body->SetGravityScale(scale);
}

//private