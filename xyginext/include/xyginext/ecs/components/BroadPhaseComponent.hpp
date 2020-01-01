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

#include "xyginext/Config.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <cstdint>
#include <limits>

namespace xy
{
    /*!
    \brief Entities with broadphase components are returned
    from dynamic tree queries, which can then be used in collision
    testing. Narrow phase collision should be performed independently
    \see DynamicTreeSystem
    */
    struct XY_EXPORT_API BroadphaseComponent final
    {
    public:
        /*!
        \brief Default constructor
        */
        BroadphaseComponent() = default;

        /*!
        \brief Construct a broadphase component from given FloatRect and flags
        */
        BroadphaseComponent(sf::FloatRect rect, std::uint64_t flags = std::numeric_limits<std::uint64_t>::max())
            : m_bounds(rect), m_filterFlags(flags) {}

        /*!
        \brief Set the area within the world which
        this component's entity covers, in local coordinates.
        */
        void setArea(sf::FloatRect area) { m_bounds = area; }

        /*!
        \brief Returns the local bounds of the component as applied in the
        constructor oe with setArea()
        */
        sf::FloatRect getArea() const { return m_bounds; }

        /*!
        \brief Allows filtering BroadphaseComponents during DynamicTree queries.
        The filter consists of up to 64 flags allowing items
        to be categorised. When the dynamic tree is queried only the items
        matching the query flags are returned. For example setting the
        flag to 4 will set the 3rd bit, and items matching a dynamic tree
        query which includes the 3rd bit will be included in the results.
        All flags are set by default, so all items are returned in a query
        */
        void setFilterFlags(std::uint64_t flags) { m_filterFlags = flags; }

        /*!
        \brief Returns the current flags for this item. Defaults to
        the max size of uint64 (all flags set)
        */
        std::uint64_t getFilterFlags() const { return m_filterFlags; }

    private:
        sf::FloatRect m_bounds; //<! AABB of the entity
        std::int32_t m_treeID = -1;
        std::uint64_t m_filterFlags = std::numeric_limits<std::uint64_t>::max();
        sf::Vector2f m_lastWorldPosition;

        friend class DynamicTreeSystem;
    };

}