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

#include <xygine/ParticleController.hpp>
#include <xygine/ParticleSystem.hpp>
#include <xygine/Entity.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Scene.hpp>

namespace
{
    std::vector<sf::Vector2f> explodeVelocities =
    {
        { -80.5f, 0.f },
        { -60.f, -8.9f },
        { -20.f, -24.f },
        { 0.f, -40.5f },
        { 28.5f, -24.6f },
        { 64.f, -8.5f },
        { 80.9f, 0.f },
        { 64.f, 9.5f },
        { 28.f, 27.5f },
        { 0.7f, 40.4f },
        { -20.f, 29.6f },
        { -60.f, 9.5f }
    };
}

using namespace xy;

ParticleController::ParticleController(MessageBus& mb)
    : Component (mb),
    m_entity    (nullptr)
{

}

//public
Component::Type ParticleController::type() const
{
    return Component::Type::Script;
}

Component::UniqueType ParticleController::uniqueType() const
{
    return Component::UniqueId::ParticleControllerId;
}

void ParticleController::entityUpdate(Entity&, float){}

void ParticleController::handleMessage(const Message& msg)
{
    switch (msg.type)
    {
    case Message::Type::Player:

        break;
    default: break;
    }
}

void ParticleController::onStart(Entity& entity)
{
    m_entity = &entity;
}

void ParticleController::explosion(const sf::Vector2f& position, const sf::Color& colour)
{
    auto& children = m_entity->getChildren();
    auto child = std::find_if(children.begin(), children.end(), [](const Entity::Ptr& p)
    {
        return !p->getComponent<ParticleSystem>(Component::UniqueId::ParticleSystemId)->active();
    });

    if (child != children.end())
    {
        (*child)->setWorldPosition(position);
        (*child)->getComponent<ParticleSystem>(Component::UniqueId::ParticleSystemId)->start(5, 0.f, 0.1f);
        (*child)->getComponent<ParticleSystem>(Component::UniqueId::ParticleSystemId)->setColour(colour);
        //LOG("CLIENT fired existing explosion particles", Logger::Type::Info);
    }
    else
    {
        auto ent = std::make_unique<Entity>(getMessageBus());
        ent->setPosition(position);
        auto system = std::make_unique<ParticleSystem>(getMessageBus());
        system->setRandomInitialVelocity(explodeVelocities);
        system->followParent(true);
        ForceAffector fa({ 0.f, 150.f });
        system->addAffector(fa);
        system->start(5, 0.f, 0.1f);
        system->setColour(colour);
        ent->addComponent<ParticleSystem>(system);
        m_entity->addChild(ent);
        //LOG("CLIENT fired new explosion particles", Logger::Type::Info);
    }
}