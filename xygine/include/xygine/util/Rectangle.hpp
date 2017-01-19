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

#ifndef XY_UTIL_RECTANGLE_HPP_
#define XY_UTIL_RECTANGLE_HPP_

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

namespace xy
{
    namespace Util
    {
        /*!
        \brief Functions for manipulating rectangle types
        */
        namespace Rectangle
        {
            /*!
            \brief Returns true if the second given FloatRect is fully contained within the first
            */
            static inline bool contains(const sf::FloatRect& first, const sf::FloatRect& second)
            {
                if (second.left < first.left) return false;
                if (second.top < first.top) return false;
                if (second.left + second.width > first.left + first.width) return false;
                if (second.top + second.height > first.top + first.height) return false;

                return true;
            }
            /*!
            \brief Returns the centre point of a given FloatRect
            */
            static inline sf::Vector2f centre(const sf::FloatRect& rect)
            {
                return sf::Vector2f(rect.left + (rect.width / 2.f), rect.top + (rect.height / 2.f));
            }
            /*!
            \brief Returns a new FloatRect from the given upper and lower bounds.
            */
            static inline sf::FloatRect fromBounds(const sf::Vector2f& lower, const sf::Vector2f& upper)
            {
                return sf::FloatRect(lower.x, lower.y, upper.x - lower.x, upper.y - lower.y);
            }
        }
    }
}
#endif //XY_UTIL_RECTANGLE_HPP_