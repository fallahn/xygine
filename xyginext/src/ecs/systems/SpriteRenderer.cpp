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
#include <xyginext/core/App.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace xy;

namespace
{
    const std::size_t MaxSprites = 256;
    const std::size_t MinSprites = 16;
}

SpriteRenderer::SpriteRenderer(MessageBus& mb)
    : System        (mb, typeid(SpriteRenderer)),
    m_wantsSorting  (false),
    m_drawlist      (MinSprites),
    m_drawCount     (0)
{
    requireComponent<xy::Transform>();
    requireComponent<xy::Sprite>();
}

//public
void SpriteRenderer::process(float dt)
{
    //update geometry
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

    //cull to viewport
    auto view = App::getRenderWindow().getView();
    sf::FloatRect viewableArea(view.getCenter() - (view.getSize() / 2.f), view.getSize());

    m_drawCount = 0;

    //add visible to draw list
    for (const auto& entity : entities)
    {
        const auto& sprite = entity.getComponent<xy::Sprite>();
        const auto& tx = entity.getComponent<xy::Transform>().getWorldTransform();
        if (tx.transformRect(sprite.getLocalBounds()).intersects(viewableArea)
            && m_drawCount < m_drawlist.size())
        {
            m_drawlist[m_drawCount].verts = sprite.m_vertices;
            m_drawlist[m_drawCount].states = sprite.m_states;
            m_drawlist[m_drawCount++].states.transform = tx;
        }
    }
}

//private
void SpriteRenderer::onEntityAdded(Entity)
{
    m_wantsSorting = true;

    if (getEntities().size() > m_drawlist.size()
        && getEntities().size() < MaxSprites)
    {
        m_drawlist.resize(m_drawlist.size() + MinSprites);
    }
}

void SpriteRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for(auto i = 0u; i < m_drawCount; ++i)
    {
        const auto& sprite = m_drawlist[i];
        rt.draw(sprite.verts.data(), 4, sf::Quads, sprite.states);
    }
}
