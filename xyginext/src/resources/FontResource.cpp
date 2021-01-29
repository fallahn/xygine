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

//creates a default font in memory to return when requested font unavailable//
#include "xyginext/resources/Resource.hpp"
#include "xyginext/resources/SystemFont.hpp"

using namespace xy;

FontResource::FontResource()
{
    static const std::array<std::string, 4u> fontNames =
    {
        "arial.ttf",
        "Arial.ttf",
        "tahoma.ttf",
        "VeraMono.ttf"
    };

    std::string fullPath = getFontPath();
    for (const auto& font : fontNames)
    {
        if (m_font.loadFromFile(fullPath + font))
        {

            xy::Logger::log("Loaded default font " + fullPath + font, xy::Logger::Type::Info);
            break;
        }
        else
        {
            xy::Logger::log("Failed to load fallback font at " + fullPath + font, xy::Logger::Type::Warning);
        }
    }
}

std::unique_ptr<sf::Font> FontResource::errorHandle()
{
	return std::make_unique<sf::Font>(m_font);
}
