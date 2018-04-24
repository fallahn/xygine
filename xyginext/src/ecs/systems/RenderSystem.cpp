/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/ecs/systems/RenderSystem.hpp"

#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/Drawable.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>

xy::RenderSystem::RenderSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(xy::RenderSystem)),
    m_wantsSorting  (true)
{
    requireComponent<xy::Drawable>();
    requireComponent<xy::Transform>();
}

//public
void xy::RenderSystem::process(float)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& drawable = entity.getComponent<xy::Drawable>();
        if (drawable.m_wantsSorting)
        {
            drawable.m_wantsSorting = false;
            m_wantsSorting = true;
        }
    }

    //do Z sorting
    if (m_wantsSorting)
    {
        m_wantsSorting = false;

        std::sort(entities.begin(), entities.end(),
            [](const Entity& entA, const Entity& entB)
        {
            return entA.getComponent<xy::Drawable>().getDepth() < entB.getComponent<xy::Drawable>().getDepth();
        });
    }
}

void xy::RenderSystem::setCullingBorder(float size)
{
    m_cullingBorder = { size, size };
}

//private
void xy::RenderSystem::onEntityAdded(xy::Entity)
{
    m_wantsSorting = true;
}

void xy::RenderSystem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto view = rt.getView();
    sf::FloatRect viewableArea((view.getCenter() - (view.getSize() / 2.f)) - m_cullingBorder, view.getSize() + m_cullingBorder);

    for (auto entity : getEntities())
    {
        const auto& drawable = entity.getComponent<xy::Drawable>();
        const auto& tx = entity.getComponent<xy::Transform>().getWorldTransform();
        const auto bounds = tx.transformRect(drawable.getLocalBounds());
        if (!drawable.m_cull || bounds.intersects(viewableArea))
        {
            states = drawable.m_states;
            states.transform = tx;

            if (states.shader)
            {
                sf::Shader* shader = const_cast<sf::Shader*>(states.shader);
                for (auto i = 0u; i < drawable.m_textureCount; ++i)
                {
                    const auto& pair = drawable.m_textureBindings[i];
                    shader->setUniform(pair.first, *pair.second);
                }
                for (auto i = 0u; i < drawable.m_floatCount; ++i)
                {
                    const auto& pair = drawable.m_floatBindings[i];
                    shader->setUniform(pair.first, pair.second);
                }
                for (auto i = 0u; i < drawable.m_vec2Count; ++i)
                {
                    const auto& pair = drawable.m_vec2Bindings[i];
                    shader->setUniform(pair.first, pair.second);
                }
                for (auto i = 0u; i < drawable.m_vec3Count; ++i)
                {
                    const auto& pair = drawable.m_vec3Bindings[i];
                    shader->setUniform(pair.first, pair.second);
                }
                for (auto i = 0u; i < drawable.m_boolCount; ++i)
                {
                    const auto& pair = drawable.m_boolBindings[i];
                    shader->setUniform(pair.first, pair.second);
                }
                for (auto i = 0u; i < drawable.m_matCount; ++i)
                {
                    const auto& pair = drawable.m_matBindings[i];
                    shader->setUniform(pair.first, sf::Glsl::Mat4(pair.second));
                }
            }

            rt.draw(drawable.m_vertices.data(), drawable.m_vertices.size(), drawable.m_primitiveType, states);
        }
    }
}
