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

#ifndef XY_RENDER_SYSTEM_HPP_
#define XY_RENDER_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <SFML/Graphics/Drawable.hpp>

namespace xy
{
    /*!
    \brief Used to draw all entities which have a Drawable and Transform component.
    The RenderSystem is used to depth sort and draw all entities which have a 
    Drawable and Transform component attached, and optionally a Sprite component.
    NOTE multiple components which rely on a Drawable component cannot exist on the same entity,
    as only one set of vertices will be available.
    */
    class XY_EXPORT_API RenderSystem final : public xy::System, public sf::Drawable 
    {
    public:
        explicit RenderSystem(xy::MessageBus&);

        void process(float) override;

    private:
        bool m_wantsSorting;

        void onEntityAdded(xy::Entity) override;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //XY_RENDER_SYSTEM_HPP_