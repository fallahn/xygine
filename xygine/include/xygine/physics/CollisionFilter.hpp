/*********************************************************************
© Matt Marchant 2014 - 2017
http://trederia.blogspot.com

xygine - Zlib license.

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

#ifndef XY_COLLISION_FILTER_HPP_
#define XY_COLLISION_FILTER_HPP_

#include <xygine/Config.hpp>
#include <SFML/Config.hpp>

namespace xy
{
    namespace Physics
    {
        /*!
        \brief Struct containing a collision filter mask

        See the Box2D manual on collision filtering
        */
        struct XY_EXPORT_API CollisionFilter
        {
            sf::Uint16 categoryFlags = 0x0001;
            sf::Uint16 maskFlags = 0xFFFF;
            sf::Int16 groupIndex = 0;

            /*!
            \brief Compare another filter to this
            \param other Another CollisionFilter to compare to this one
            \returns true if the given filter passes testing
            */
            bool passes(const CollisionFilter& other) const
            {
                return ((categoryFlags & other.maskFlags) != 0 && (other.categoryFlags & maskFlags) != 0);
            }
        };
    }
}

#endif //XY_COLLISION_FILTER_HPP_