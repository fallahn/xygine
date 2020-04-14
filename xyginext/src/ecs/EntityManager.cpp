/*********************************************************************
(c) Matt Marchant 2017 - 2020
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#include "xyginext/ecs/Entity.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/core/MessageBus.hpp"

using namespace xy;

namespace
{
    const std::size_t MinComponentMasks = 50;
}

EntityManager::EntityManager(MessageBus& mb, ComponentManager& cm, std::size_t poolSize)
    : m_messageBus      (mb),
    m_componentManager  (cm),
    m_componentPools    (Detail::MaxComponents),
    m_initialPoolSize   (poolSize),
    m_entityCount       (0)
{}

//public
Entity EntityManager::createEntity()
{
    Entity::ID idx = 0;
    if (m_generations.size() == Detail::MaxIDsPerGeneration)
    {
        idx = m_freeIDs.front();
        m_freeIDs.pop_front();
    }
    else
    {
        m_generations.push_back(0);
        idx = static_cast<Entity::ID>(m_generations.size() - 1);
        
        XY_ASSERT(idx < (1 << Detail::IndexBits), "Index out of range");
        if (idx >= m_componentMasks.size())
        {
            m_componentMasks.resize(m_componentMasks.size() + MinComponentMasks);
            m_labels.resize(m_componentMasks.size());
        }
    }

    XY_ASSERT(idx < m_generations.size(), "Index out of range");
    Entity e(idx, m_generations[idx]);
    e.m_entityManager = this;

    m_entityCount++;

#ifdef XY_DEBUG
    if (m_entityCount == 1000)
    {
        xy::Logger::log("Entity count has reached 1000!", xy::Logger::Type::Warning);
    }
#endif //XY_DEBUG

    return e;
}

void EntityManager::destroyEntity(Entity entity)
{
    const auto index = entity.getIndex();
    XY_ASSERT(index < m_generations.size(), "Index out of range");
    XY_ASSERT(m_generations[index] < 255, "Max generations reached!");

    //make sure we don't destroy this entity more than once
    //otherwise the ID will get marked as free multiple times
    if (entity.getGeneration() == m_generations[index])
    {
        ++m_generations[index];
        m_freeIDs.push_back(index);
        m_componentMasks[index].reset();
        m_labels[index].clear();

        m_entityCount--;

        //clears up any moveable components
        //or drawables with vertex arrays
        //which might otherwise get left behind
        for (auto& pool : m_componentPools)
        {
            if (pool)
            {
                pool->reset(index);
            }
        }

        //let the world know the entity was destroyed
        auto msg = m_messageBus.post<Message::SceneEvent>(Message::SceneMessage);
        msg->entityID = index;
        msg->event = Message::SceneEvent::EntityDestroyed;
    }
}

bool EntityManager::entityDestroyed(Entity entity) const
{
    const auto id = entity.getIndex();
    XY_ASSERT(id < m_generations.size(), "Generation index out of range");
    
    return (m_generations[id] != entity.getGeneration());
}

Entity EntityManager::getEntity(Entity::ID id) const
{
    XY_ASSERT(id < m_generations.size(), "Invalid Entity ID");
    Entity ent(id, m_generations[id]);
    ent.m_entityManager = const_cast<EntityManager*>(this);
    return ent;
}

const ComponentMask& EntityManager::getComponentMask(Entity entity) const
{
    const auto index = entity.getIndex();
    XY_ASSERT(index < m_componentMasks.size(), "Invalid mask index (out of range)");
    return m_componentMasks[index];
}

bool EntityManager::owns(Entity entity) const
{
    return (entity.m_entityManager == this);
}

void EntityManager::setLabel(Entity entity, const std::string& label)
{
    const auto index = entity.getIndex();
    XY_ASSERT(index < m_labels.size(), "Invalid label index (out of range)");
    m_labels[index] = label;
}

const std::string& EntityManager::getLabel(Entity entity) const
{
    const auto index = entity.getIndex();
    XY_ASSERT(index < m_componentMasks.size(), "Invalid label index (out of range)");
    return m_labels[index];
}