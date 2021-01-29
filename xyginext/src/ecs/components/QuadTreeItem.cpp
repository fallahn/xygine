/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/ecs/components/QuadTreeItem.hpp"

using namespace xy;

QuadTreeItem::QuadTreeItem()
    : m_area        (0.f, 0.f, 1.f, 1.f),
    m_quadTree      (nullptr),
    m_node          (nullptr),
    m_filterFlags   (std::numeric_limits<std::uint64_t>::max())
{

}

QuadTreeItem::QuadTreeItem(sf::FloatRect area, std::uint64_t flags)
    : m_area        (area),
    m_quadTree      (nullptr),
    m_node          (nullptr),
    m_filterFlags   (flags)
{

}

//public
void QuadTreeItem::setArea(sf::FloatRect rect)
{
    m_area = rect; //TODO flag update
}
