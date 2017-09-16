/*********************************************************************
(c) Matt Marchant 2017
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

#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/systems/SpriteRenderer.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;

SpriteRenderer::SpriteRenderer(MessageBus& mb)
    : System        (mb, typeid(SpriteRenderer)),
    m_wantsSorting  (false)
{
    requireComponent<xy::Transform>();
    requireComponent<xy::Sprite>();
}

//public
void SpriteRenderer::process(float dt)
{
    //TODO cull to viewport
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        if (sprite.m_dirty)
        {
            //update vert positions
            const auto subRect = sprite.m_textureRect;
            auto& verts = sprite.m_vertices;
            verts[0].position = { 0.f, 0.f };
            verts[1].position = { subRect.width, 0.f };
            verts[2].position = { subRect.width, subRect.height };
            verts[3].position = { 0.f, subRect.height };

            //update vert coords
            verts[0].texCoords = { subRect.left, subRect.top };
            verts[1].texCoords = { subRect.left + subRect.width, subRect.top };
            verts[2].texCoords = { subRect.left + subRect.width, subRect.top + subRect.height };
            verts[3].texCoords = { subRect.left, subRect.top + subRect.height };

            sprite.m_dirty = false;
        }

        if (sprite.m_wantsSorting)
        {
            m_wantsSorting = true;
            sprite.m_wantsSorting = false;
        }
    }

    //do Z sorting
    if (m_wantsSorting)
    {
        m_wantsSorting = false;

        std::sort(entities.begin(), entities.end(),
            [](const Entity& entA, const Entity& entB) 
        {
            return entA.getComponent<Sprite>().getDepth() < entB.getComponent<Sprite>().getDepth();
        });
    }
}

//private
void SpriteRenderer::onEntityAdded(Entity)
{
    m_wantsSorting = true;
}

void SpriteRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    const auto& entities = getEntities();
    for (const auto& entity : entities)
    {
        const auto& sprite = entity.getComponent<Sprite>();
        
        states = sprite.m_states;
        states.transform = entity.getComponent<Transform>().getWorldTransform();

        rt.draw(sprite.m_vertices.data(), 4, sf::Quads, states);
    }
}
