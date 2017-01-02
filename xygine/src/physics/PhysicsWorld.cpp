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

#include <xygine/physics/World.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Dynamics/Joints/b2Joint.h>
#include <Box2D/Dynamics/Contacts/b2Contact.h>

#include <cstring>

using namespace xy;
using namespace xy::Physics;

namespace
{
    float worldScale = 100.f;
    b2Vec2 gravity = { 0.f, -9.8f };
    sf::Uint32 velocityIterations = 6u;
    sf::Uint32 positionIterations = 2u;
}

std::function<void(float)> World::update = [](float) {};

World::Ptr World::m_world = nullptr;
World* World::m_instance = nullptr;

World::World(MessageBus& mb)
    : m_contactListener(mb), m_destructionListener(mb)
{
    XY_ASSERT(!m_world, "Physics world already created");
    m_world = std::make_unique<b2World>(gravity);

    update = [this](float dt)
    {
        XY_ASSERT(m_world, "Physics world has not been created");
        m_contactListener.resetBuffers();
        m_world->Step(dt, velocityIterations, positionIterations);
    };

    m_world->SetContactListener(&m_contactListener);
    m_world->SetDestructionListener(&m_destructionListener);

    m_instance = this;

    LOG("CLIENT created physics world", Logger::Type::Info);
}

World::~World()
{
    m_world.reset();
    update = [](float) {};
    m_instance = nullptr;
    LOG("CLIENT destroyed physics world", Logger::Type::Info);
}

void World::setGravity(const sf::Vector2f& g)
{
    gravity = sfToBoxVec(g);
    if (m_world) m_world->SetGravity(gravity);
}

void World::setPixelScale(float scale)
{
    XY_ASSERT((scale > 5 && scale < 5000), "Reasonable scales range from 10 to 1000 pixels per metre");
    worldScale = scale;
}

void World::setVelocityIterationCount(sf::Uint32 count)
{
    velocityIterations = count;
}

void World::setPositionIterationCount(sf::Uint32 count)
{
    positionIterations = count;
}

void World::addJointDestroyedCallback(const JointDestroyedCallback& jdc)
{
    m_destructionListener.addCallback(jdc);
}

void World::addCollisionShapeDestroyedCallback(const CollisionShapeDestroyedCallback& csdc)
{
    m_destructionListener.addCallback(csdc);
}

World::CallbackIndex World::addContactPreSolveCallback(const ContactCallback& psc)
{
    return m_contactListener.addContactPreSolveCallback(psc);
}

World::CallbackIndex World::addContactPostSolveCallback(const ContactCallback& psc)
{
    return m_contactListener.addContactPostSolveCallback(psc);
}

World::CallbackIndex World::addContactBeginCallback(const ContactCallback& cb)
{
    return m_contactListener.addContactBeginCallback(cb);
}

World::CallbackIndex World::addContactEndCallback(const ContactCallback& cb)
{
    return m_contactListener.addContactEndCallback(cb);
}

void World::removeContactPreSolveCallback(World::CallbackIndex idx)
{
    m_contactListener.removeContactPreSolveCallback(idx);
}

void World::removeContactPostSolveCallback(World::CallbackIndex idx)
{
    m_contactListener.removeContactPostSolveCallback(idx);
}

void World::removeContactBeginCallback(World::CallbackIndex idx)
{
    m_contactListener.removeContactBeginCallback(idx);
}

void World::removeContactEndCallback(World::CallbackIndex idx)
{
    m_contactListener.removeContactEndCallback(idx);
}

void World::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!m_debugDraw)
    {
        m_debugDraw = std::make_unique<xy::Physics::DebugDraw>(rt);
        m_world->SetDebugDraw(m_debugDraw.get());
    }
    m_world->DrawDebugData();
}

//conversion functions
b2Vec2 World::sfToBoxVec(const sf::Vector2f& vec)
{
    return b2Vec2(vec.x / worldScale, -vec.y / worldScale);
}

sf::Vector2f World::boxToSfVec(const b2Vec2& vec)
{
    return sf::Vector2f(vec.x, -vec.y) * worldScale;
}

float World::sfToBoxFloat(float val)
{
    return val / worldScale;
}

float World::boxToSfFloat(float val)
{
    return val * worldScale;
}

float World::getWorldScale() const
{
    return worldScale;
}

//contact callbacks
namespace
{
    const std::size_t contactSize = sizeof(b2Contact);
    const std::size_t maxBufferedContact = 1024u;
}
World::ContactListener::ContactListener(MessageBus& mb)
    : m_messageBus      (mb),
    m_contactBuffer     (maxBufferedContact * contactSize),
    m_contactBufferPtr  (m_contactBuffer.data()),
    m_messageContacts   (maxBufferedContact),
    m_messageIndex      (0u)
{


}

void World::ContactListener::BeginContact(b2Contact* contact)
{
    XY_ASSERT(m_messageIndex < maxBufferedContact, "Buffer index out of range");
    
    m_messageContacts[m_messageIndex].m_contact = bufferContact(contact);
    
    auto msg = m_messageBus.post<Message::PhysicsEvent>(xy::Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::BeginContact;
    msg->contact = &m_messageContacts[m_messageIndex++];

    m_currentContact.m_contact = contact;
    for (auto& cb : m_beginCallbacks)
    {
        cb(m_currentContact);
    }
}

void World::ContactListener::EndContact(b2Contact* contact)
{
    XY_ASSERT(m_messageIndex < maxBufferedContact, "Buffer index out of range");
    
    m_messageContacts[m_messageIndex].m_contact = bufferContact(contact);

    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::EndContact;
    msg->contact = &m_messageContacts[m_messageIndex++];

    m_currentContact.m_contact = contact;
    for (auto& cb : m_endCallbacks)
    {
        cb(m_currentContact);
    }
}

void World::ContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    m_currentContact.m_contact = contact;
    for (auto& cb : m_preSolveCallbacks)
    {
        cb(m_currentContact);
    }
}

void World::ContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    m_currentContact.m_contact = contact;
    for (auto cb : m_postSolveCallbacks)
    {
        cb(m_currentContact);
    }
}

World::CallbackIndex World::ContactListener::addContactPreSolveCallback(const ContactCallback& psc)
{
    m_preSolveCallbacks.push_back(psc);
    return std::next(m_preSolveCallbacks.end(), - 1);
}

World::CallbackIndex World::ContactListener::addContactPostSolveCallback(const ContactCallback& psc)
{
    m_postSolveCallbacks.push_back(psc);
    return std::next(m_postSolveCallbacks.end(), -1);
}

World::CallbackIndex World::ContactListener::addContactBeginCallback(const ContactCallback& cb)
{
    m_beginCallbacks.push_back(cb);
    return std::next(m_beginCallbacks.end(), -1);
}

World::CallbackIndex World::ContactListener::addContactEndCallback(const ContactCallback& cb)
{
    m_endCallbacks.push_back(cb);
    return std::next(m_endCallbacks.end(), -1);
}

void World::ContactListener::removeContactPreSolveCallback(World::CallbackIndex idx)
{
    XY_ASSERT(idx != m_preSolveCallbacks.end(), "invalid iterator");
    m_preSolveCallbacks.erase(idx);
}

void World::ContactListener::removeContactPostSolveCallback(World::CallbackIndex idx)
{
    XY_ASSERT(idx != m_postSolveCallbacks.end(), "invalid iterator");
    m_postSolveCallbacks.erase(idx);
}

void World::ContactListener::removeContactBeginCallback(World::CallbackIndex idx)
{
    XY_ASSERT(idx != m_beginCallbacks.end(), "invalid iterator");
    m_beginCallbacks.erase(idx);
    LOG("PHYSICS removed begin contact callback", Logger::Type::Info);
}

void World::ContactListener::removeContactEndCallback(World::CallbackIndex idx)
{
    XY_ASSERT(idx != m_endCallbacks.end(), "invalid iterator");
    m_endCallbacks.erase(idx);
    LOG("PHYSICS removed end contact callback", Logger::Type::Info);
}

void World::ContactListener::resetBuffers()
{
    m_contactBufferPtr = m_contactBuffer.data();
    m_messageIndex = 0u;
}

b2Contact* World::ContactListener::bufferContact(const b2Contact* contact)
{
    XY_ASSERT((m_contactBuffer.size() - (std::uint64_t)m_contactBufferPtr) > contactSize, "This would cause a buffer overflow!");
    b2Contact* retVal = reinterpret_cast<b2Contact*>(m_contactBufferPtr);
    std::memcpy(m_contactBufferPtr, contact, contactSize);
    m_contactBufferPtr += contactSize;
    return retVal;
}

//destruction callbacks
void World::DestructionListener::SayGoodbye(b2Joint* joint)
{
    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::JointDestroyed;
    msg->joint = static_cast<Joint*>(joint->GetUserData());

    for (const auto& cb : m_jointCallbacks)
    {
        cb(*msg->joint);
    }
}

void World::DestructionListener::SayGoodbye(b2Fixture* fixture)
{
    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::CollisionShapeDestroyed;
    msg->collisionShape = static_cast<CollisionShape*>(fixture->GetUserData());

    for (const auto& cb : m_collisionShapeCallbacks)
    {
        cb(*msg->collisionShape);
    }
}

void World::DestructionListener::addCallback(const World::JointDestroyedCallback& jc)
{
    m_jointCallbacks.push_back(jc);
}

void World::DestructionListener::addCallback(const World::CollisionShapeDestroyedCallback& cscb)
{
    m_collisionShapeCallbacks.push_back(cscb);
}