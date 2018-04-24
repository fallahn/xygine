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

#include "xyginext/ecs/Component.hpp"
#include "xyginext/ecs/Entity.hpp"

using namespace xy;

ComponentManager::ID ComponentManager::getFromTypeID(std::type_index id)
{
    auto result = std::find(std::begin(m_IDs), std::end(m_IDs), id);
    if (result == m_IDs.end())
    {
        XY_ASSERT(m_IDs.size() < Detail::MaxComponents, "Max components have been allocated");
        m_IDs.push_back(id);
        return static_cast<ID>(m_IDs.size() - 1);
    }
    return static_cast<ID>(std::distance(m_IDs.begin(), result));
}
