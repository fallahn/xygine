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

#include <xygine/components/ParticleController.hpp>
#include <xygine/MessageBus.hpp>
#include <xygine/Scene.hpp>

namespace
{

}

using namespace xy;

ParticleController::ParticleController(MessageBus& mb)
    : Component (mb, this),
    m_entity    (nullptr)
{

}

//public
Component::Type ParticleController::type() const
{
    return Component::Type::Script;
}

void ParticleController::entityUpdate(Entity&, float){}

void ParticleController::onStart(Entity& entity)
{
    m_entity = &entity;
    for (auto i = 0u; i < m_pendingDefinitions.size(); ++i)
    {
	Entity::Ptr e = std::move(m_pendingDefinitions[i]);      
	m_entity->addChild(e);
    }
    m_pendingDefinitions.clear();
}

void ParticleController::addDefinition(SystemID id, const ParticleSystem::Definition& d)
{
    auto ent = Entity::create(getMessageBus());
    m_activeSystems[id] = std::make_pair(ent.get(), d);
    
    if (m_entity)
    {
        m_entity->addChild(ent);
    }
    else
    {
        m_pendingDefinitions.push_back(std::move(ent));
    }
}

void ParticleController::fire(SystemID id, const sf::Vector2f& position)
{
    auto& pair = m_activeSystems[id];
    auto ent = pair.first;
    XY_ASSERT(ent, "particle entity is null");

    //search children for inactive system
    auto& children = ent->getChildren();
    auto child = std::find_if(children.begin(), children.end(), [](const Entity::Ptr& p)
    {
        return !p->getComponent<ParticleSystem>()->active();
    });

    const auto& definition = pair.second;
    if (child != children.end())
    {
        (*child)->setWorldPosition(position);
        (*child)->getComponent<ParticleSystem>()->start(definition.releaseCount, definition.delay, definition.duration);
        //LOG("CLIENT fired existing particle system", Logger::Type::Info);
    }
    else
    {
        //if no inactive systems add a new one from the definition
        auto entity = Entity::create(getMessageBus());
        auto ps = definition.createSystem(getMessageBus());
        entity->addComponent<ParticleSystem>(ps);
        entity->setWorldPosition(position);
        entity->getComponent<ParticleSystem>()->start(definition.releaseCount, definition.delay, definition.duration);
        ent->addChild(entity);
        //LOG("CLIENT fired new particle system", Logger::Type::Info);
    }
}
