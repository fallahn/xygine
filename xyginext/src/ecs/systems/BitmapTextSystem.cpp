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

#include "xyginext/ecs/systems/BitmapTextSystem.hpp"

#include "xyginext/ecs/components/BitmapText.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Transform.hpp"

#include "xyginext/graphics/BitmapFont.hpp"

using namespace xy;

BitmapTextSystem::BitmapTextSystem(xy::MessageBus& mb)
    : xy::System(mb, typeid(BitmapTextSystem))
{
    requireComponent<Transform>();
    requireComponent<Drawable>();
    requireComponent<BitmapText>();
}

//public
void BitmapTextSystem::process(float)
{
    auto entities = getEntities();
    for (auto entity : entities)
    {
        auto& drawable = entity.getComponent<Drawable>();
        auto& text = entity.getComponent<BitmapText>();

        if (text.m_dirty)
        {
            text.updateVertices(drawable);

            drawable.setTexture(text.getFont()->getTexture());
            drawable.setPrimitiveType(sf::PrimitiveType::Quads);
        }
    }
}