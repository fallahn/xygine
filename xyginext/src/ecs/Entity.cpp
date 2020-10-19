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

#include "xyginext/ecs/Component.hpp"
#include "xyginext/ecs/Entity.hpp"

using namespace xy;

namespace
{
    const std::uint32_t IndexMask = (1 << Detail::IndexBits) - 1;
    const std::uint32_t GenerationMask = (1 << Detail::GenerationBits) - 1;
}

Entity::Entity()
    : m_id          ((0 << Detail::IndexBits) | std::numeric_limits<ID>::max()),
    m_entityManager (nullptr),
    m_destroyed     (false)
{

}

Entity::Entity(Entity::ID index, Entity::Generation generation)
    : m_id          ((generation << Detail::IndexBits) | index),
    m_entityManager (nullptr),
    m_destroyed     (false)
{

}

//public
Entity::ID Entity::getIndex() const
{
    return m_id & IndexMask;
}

Entity::Generation Entity::getGeneration() const
{
    return (m_id >> Detail::IndexBits) & GenerationMask;
}

//TODO fix this so that it goes through its parent scene.
//destroying here is not enough as it will not unregister
//from all the active scene systems
//void Entity::destroy() 
//{
//    XY_ASSERT(m_entityManager, "Invalid Entity instance");
//    m_entityManager->destroyEntity(*this);
//}

bool Entity::destroyed() const
{
    XY_ASSERT(m_entityManager, "Invalid Entity instance");
    return m_destroyed || m_entityManager->entityDestroyed(*this);
}

const ComponentMask& Entity::getComponentMask() const
{
    XY_ASSERT(isValid(), "Invalid Entity instance");
    return m_entityManager->getComponentMask(*this);
}

bool Entity::isValid() const
{
    return ((m_entityManager != nullptr) && (!m_entityManager->entityDestroyed(*this)));
}

void Entity::setLabel(const std::string& label)
{
    XY_ASSERT(isValid(), "Invalid Entity instance");
    m_entityManager->setLabel(*this, label);
}

const std::string& Entity::getLabel() const
{
    XY_ASSERT(isValid(), "Invalid Entity instance");
    return m_entityManager->getLabel(*this);
}

bool operator < (const Entity& l, const Entity& r) { return (l.getIndex() < r.getIndex()); }
