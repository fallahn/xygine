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
#include "xyginext/ecs/System.hpp"
#include "xyginext/gui/Gui.hpp"

using namespace xy;

SystemManager::SystemManager(Scene& scene, ComponentManager& cm) 
    : m_scene           (scene),
    m_componentManager  (cm),
    m_showSystemInfo    (false)
{
    m_systems.reserve(128);

    registerWindow([&]()
        {
            if (m_showSystemInfo)
            {
                ImGui::SetNextWindowSize({ 220.f, 300.f });
                if (ImGui::Begin("System Data", &m_showSystemInfo, ImGuiWindowFlags_AlwaysVerticalScrollbar))
                {
                    for (const auto* s : m_activeSystems)
                    {
                        ImGui::Text("%s\nEntities: %d", s->getType().name(), s->getEntities().size());
                    }
                }
                ImGui::End();
            }
        });
}

void SystemManager::addToSystems(Entity entity)
{
    const auto& entMask = entity.getComponentMask();
    for (auto& sys : m_systems)
    {
        const auto& sysMask = sys->getComponentMask();
        if ((entMask & sysMask) == sysMask)
        {
            sys->addEntity(entity);
        }
    }
}

void SystemManager::removeFromSystems(Entity entity)
{
    for (auto& sys : m_systems)
    {
        sys->removeEntity(entity);
    }
}

void SystemManager::forwardMessage(const Message& msg)
{
    for (auto& sys : m_systems)
    {
        sys->handleMessage(msg);
    }
}

void SystemManager::process(float dt)
{
    for (auto& system : m_activeSystems)
    {
        system->process(dt);
    }
}
