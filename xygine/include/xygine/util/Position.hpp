/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_UTIL_POSITION_HPP_
#define XY_UTIL_POSITION_HPP_

#include <SFML/Graphics/Text.hpp>

#include <cmath>

namespace xy
{
    namespace Util
    {
        namespace Position
        {
            /*!
            \brief Centres the origin of sf::Transformable types
            */
            template <typename T>
            static inline void centreOrigin(T& transformable)
            {
                static_assert(std::is_base_of<sf::Transformable, T>::value, "only transformable type allowed");
                sf::FloatRect bounds = transformable.getLocalBounds();
                transformable.setOrigin(std::floor(bounds.width / 2.f), std::floor(bounds.height / 2.f));

                //sf::text is, unfortunately, a special case
                if (typeid(T) == typeid(sf::Text))
                {
                    auto origin = transformable.getOrigin();
                    origin.y += bounds.top;
                    transformable.setOrigin(origin);
                }
            }
        }
    }
}

#endif //XY_UTIL_POSITION_HPP_