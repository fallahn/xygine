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

#ifndef XY_QUAD_TREE_ITEM_HPP_
#define XY_QUAD_TREE_ITEM_HPP_

#include "xyginext/Config.hpp"

#include <SFML/Graphics/Rect.hpp>

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
        QuadTreeItem();

        void setArea(sf::FloatRect);

    private:
        sf::FloatRect m_area;

        QuadTree* m_quadTree;
        QuadTreeNode* m_node;

        friend class QuadTree;
        friend class QuadTreeNode;
    };
}

#endif //XY_QUAD_TREE_ITEM_HPP_
