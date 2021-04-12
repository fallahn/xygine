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

#pragma once

#include "xyginext/Config.hpp"

#include <SFML/Config.hpp>

#include <cstdlib>
#include <vector>
#include <typeindex>
#include <algorithm>
#include <cstdint>

namespace xy
{
    class XY_API ComponentManager final
    {
    public:

        using ID = std::uint32_t;

        /*!
        \brief Returns a unique ID based on the component type
        */
        template <typename T>
        ID getID()
        {
            auto id = std::type_index(typeid(T));
            return getFromTypeID(id);
        }

        ID getFromTypeID(std::type_index);

    private:

        std::vector<std::type_index> m_IDs;     
    };
}
