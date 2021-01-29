/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/ecs/System.hpp"

using namespace xy;

System::System(MessageBus& mb, UniqueType t)
    : m_messageBus  (mb),
    m_type          (t),
    m_scene         (nullptr),
    m_updateIndex   (0),
    m_active        (false)
{}

std::vector<Entity>& System::getEntities()
{
    return m_entities;
}

const std::vector<Entity>& System::getEntities() const
{
    return m_entities;
}

//public
void System::addEntity(Entity entity)
{
    m_entities.push_back(entity);
    onEntityAdded(entity);
}

void System::removeEntity(Entity entity)
{
    m_entities.erase(std::remove_if(std::begin(m_entities), std::end(m_entities),
        [&entity, this](const Entity& e)
    {
        if (entity == e)
        {
            onEntityRemoved(e);
            return true;
        }
        return false;
    }), std::end(m_entities));
}

const ComponentMask& System::getComponentMask() const
{
    return m_componentMask;
}

void System::handleMessage(const Message&) {}

void System::process(float) {}

//protected
void System::setScene(Scene& scene)
{
    m_scene = &scene;
}

Scene* System::getScene()
{
    XY_ASSERT(m_scene, "Scene is nullptr - something went wrong!");
    return m_scene;
}


//private
void System::processTypes(ComponentManager& cm)
{
    for (const auto& componentType : m_pendingTypes)
    {
        m_componentMask.set(cm.getFromTypeID(componentType));
    }
    m_pendingTypes.clear();
}