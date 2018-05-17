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

#include "xyginext/ecs/components/Sprite.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"

using namespace xy;

namespace
{
    //const std::size_t MaxSprites = 256;
    //const std::size_t MinSprites = 16;
}

SpriteSystem::SpriteSystem(MessageBus& mb)
    : System        (mb, typeid(SpriteSystem))
{
    //requireComponent<xy::Transform>();
    requireComponent<xy::Sprite>();
    requireComponent<xy::Drawable>();
}

//public
void SpriteSystem::process(float)
{
    //update geometry
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& sprite = entity.getComponent<xy::Sprite>();
        if (sprite.m_dirty)
        {
            auto& drawable = entity.getComponent<xy::Drawable>();
            //drawable.setPrimitiveType(sf::TriangleStrip);
            
            //update vert positions
            const auto subRect = sprite.m_textureRect;
            auto& verts = drawable.getVertices();
            verts.resize(4);

            verts[0].position = { 0.f, 0.f };
            verts[1].position = { subRect.width, 0.f };
            verts[2].position = { subRect.width, subRect.height };
            verts[3].position = { 0.f, subRect.height };

            //update vert coords
            verts[0].texCoords = { subRect.left, subRect.top };
            verts[1].texCoords = { subRect.left + subRect.width, subRect.top };
            verts[2].texCoords = { subRect.left + subRect.width, subRect.top + subRect.height };
            verts[3].texCoords = { subRect.left, subRect.top + subRect.height };

            //update colour
            verts[0].color = sprite.m_colour;
            verts[1].color = sprite.m_colour;
            verts[2].color = sprite.m_colour;
            verts[3].color = sprite.m_colour;

            drawable.setTexture(sprite.getTexture());
            drawable.updateLocalBounds();

            sprite.m_dirty = false;
        }
    }
}

void SpriteSystem::handleMessage(const xy::Message &msg)
{
    if (msg.id == Message::ResourceMessage)
    {
        auto data = msg.getData<Message::ResourceEvent>();
        for (auto& e : getEntities())
        {
            auto& spr = e.getComponent<xy::Sprite>();
            if (spr.getTextureResourceID() == data.id)
            {
                // Update texture but not rect
                auto rect = spr.getTextureRect();
                spr.setTexture(*static_cast<sf::Texture*>(data.resource));
                spr.setTextureRect(rect);
            }
        }
    }
}

