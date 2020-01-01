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

#pragma once

#include "xyginext/ecs/System.hpp"

#include <SFML/Graphics/Drawable.hpp>

namespace xy
{
    /*!
    \brief System for rendering text objects.
    Usually this would be added last to the Scene after
    existing drawable systems, as the text ought to be rendered on top of everything else
    DEPRECATED: use the TextSystem and RenderSystem with Text components and Drawable components.
    */
    class XY_EXPORT_API TextRenderer final : public xy::System, public sf::Drawable
    {
    public:
        explicit TextRenderer(MessageBus&);

        void process(float) override;

    private:

        std::vector<Entity> m_texts;
        std::vector<Entity> m_croppedTexts;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}
