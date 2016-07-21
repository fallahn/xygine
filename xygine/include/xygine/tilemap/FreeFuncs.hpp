/*********************************************************************
Matt Marchant 2014 - 2016
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

#ifndef XY_TILE_FUNCS_HPP_
#define XY_TILE_FUNCS_HPP_

#include <xygine/Log.hpp>

#include <SFML/Graphics/Color.hpp>

#include <string>

namespace xy
{
    namespace tmx
    {
        static inline sf::Color colourFromString(std::string str)
        {
            std::remove(str.begin(), str.end(), '#');
            if (str.size() == 6 || str.size() == 8)
            {
                unsigned int value, r, g, b;
                unsigned int a = 255;
                std::stringstream input(str);
                input >> std::hex >> value;

                r = (value >> 16) & 0xff;
                g = (value >> 8) & 0xff;
                b = value & 0xff;

                if (str.size() == 8)
                {
                    a = (value >> 24) & 0xff;
                }

                return sf::Color(r, g, b, a);
            }
            Logger::log(str + ": not a valid colour string", Logger::Type::Error);
            return sf::Color();
        }
    }
}

#endif //XY_TILE_FUNCS_HPP_