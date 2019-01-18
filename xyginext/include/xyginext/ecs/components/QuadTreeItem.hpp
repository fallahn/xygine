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

#include "xyginext/Config.hpp"

#include <SFML/Graphics/Rect.hpp>

#include <cstdint>
#include <limits>

namespace xy
{
    class QuadTree;
    class QuadTreeNode;

    /*!
    \brief Entities with a QuadTreeItem and Transform
    component will be actively partitioned an in QuadTree
    system which is added to the entities scene.
    */
    class XY_EXPORT_API QuadTreeItem final
    {
    public:
        /*!
        \brief Default constructor
        */
        QuadTreeItem();

        /*!
        \brief Construct a QuadTreeItem from a given area and filter flags
        */
        QuadTreeItem(sf::FloatRect area, std::uint64_t flags = std::numeric_limits<std::uint64_t>::max());

        /*!
        \brief Sets the area of the quad tree item in relative coordinates
        */
        void setArea(sf::FloatRect);

        /*!
        \brief Allows filtering QuadTreeItems during QuadTree queries.
        The filter consists of up to 64 flags allowing quad tree items
        to be categorised. When the quad tree is queried only the items
        matching the query flags are returned. For example setting the
        flag to 4 will set the 3rd bit, and items matching a quad tree
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
        sf::FloatRect m_area;

        QuadTree* m_quadTree;
        QuadTreeNode* m_node;

        std::uint64_t m_filterFlags;

        friend class QuadTree;
        friend class QuadTreeNode;
    };
}
