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

#include <xygine/physics/World.hpp>
#include <xygine/MessageBus.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;
using namespace xy::Physics;

std::function<void(float)> World::update = [](float) {};

float World::m_worldScale = 100.f;
b2Vec2 World::m_gravity = { 0.f, -9.8f };
sf::Uint32 World::m_velocityIterations = 6u;
sf::Uint32 World::m_positionIterations = 2u;

World::Ptr World::m_world = nullptr;

void World::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!m_debugDraw)
    {
        m_debugDraw = std::make_unique<xy::Physics::DebugDraw>(rt);
        m_world->SetDebugDraw(m_debugDraw.get());
    }
    m_world->DrawDebugData();
}

//contact callbacks
void World::ContactCallback::BeginContact(b2Contact* contact)
{
    //TODO set body user data to entity UID
    //TODO create buffer to store contact data for messages (as messages are delayed by 1 frame)
    auto msg = m_messageBus.post<xy::Message::PhysicsEvent>(xy::Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::BeginContact;
}

void World::ContactCallback::EndContact(b2Contact* contact)
{
    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::EndContact;
}

void World::ContactCallback::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{

}

void World::ContactCallback::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{

}

//destruction callbacks
void World::DestructionCallback::SayGoodbye(b2Joint* joint)
{
    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::JointDestroyed;
    msg->joint = joint;
}

void World::DestructionCallback::SayGoodbye(b2Fixture* fixture)
{
    auto msg = m_messageBus.post<Message::PhysicsEvent>(Message::Type::PhysicsMessage);
    msg->event = Message::PhysicsEvent::FixtureDestroyed;
    msg->fixture = fixture;
}