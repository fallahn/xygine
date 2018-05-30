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

#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/String.hpp>

#include "cereal/types/string.hpp"

// because cereal
namespace sf
{
    template<class Archive>
    void serialize(Archive& ar, Color& col, const std::uint32_t version)
    {
        ar(col.r, col.g, col.b, col.a);
    }
    
    template<class Archive>
    void serialize(Archive& ar, Vector2f& vec, const std::uint32_t version)
    {
        ar(vec.x, vec.y);
    }
    
    template<class Archive>
    void serialize(Archive& ar, FloatRect& rect, const std::uint32_t version)
    {
        ar(rect.left, rect.top, rect.width, rect.height);
    }
    
    template<class Archive>
    void save(Archive& ar, const String& str, const std::uint32_t version)
    {
        ar(str.toAnsiString());
    }
    
    template<class Archive>
    void load(Archive& ar, String& str, const std::uint32_t version)
    {
        std::string string;
        ar(string);
        str = string;
    }
}
