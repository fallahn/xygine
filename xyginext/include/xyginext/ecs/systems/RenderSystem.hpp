/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

        /*!
        \brief Adds a border around the current view when culling.
        This is used to increase the effectively culled area when the system is drawn.
        By default drawables outside the view of the currently active camera are
        culled from rendering, this value adds a border around the viewble area,
        increasing or decreasing the size of the area from which drawables are culled.
        \param size The size of the border. This is a positive or negative value added
        to every side of the active view. Negative values will decrease the size of the
        culling area making drawables visibly culled from the output.
        */
        void setCullingBorder(float size);

        /*!
        \brief Sets the filter flags which decide which drawables are rendered.
        Setting this value to anything other than 0 will render all drawables
        which have their matching flag set. As drawables by default are set to
        uint64::max they will ALWAYS be drawn when this is non-zero value. Flags
        must be set on both drawable components and the RenderSystem for the filter
        to work. For example light emitting enitities can be rendered to a separate
        buffer by setting the component flag to 0x1 (first bit) then setting this
        filter to the same value. Returning it to uint64::max (all flags set) will
        once again render all drawable with a non-zero bitmask.
        Generally this is not used unless you have a specific multipass effect in mind.
        */
        void setFilterFlags(std::uint64_t flags) { m_filterFlags = flags; }

    private:
        bool m_wantsSorting;
        sf::Vector2f m_cullingBorder;
        std::uint64_t m_filterFlags;

        void onEntityAdded(xy::Entity) override;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}
