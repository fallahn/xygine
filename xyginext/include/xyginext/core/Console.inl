/*********************************************************************
(c) Matt Marchant 2017 - 2020
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


template <>
inline std::string Console::getConvarValue(const std::string& name)
{
    if (auto* obj = Console::convars.findObjectWithName(name))
    {
        if (auto* value = obj->findProperty("value"))
        {
            return value->getValue<std::string>();
        }
    }
    return {};
}

template <>
inline sf::Int32 Console::getConvarValue(const std::string& name)
{
    if (auto* obj = Console::convars.findObjectWithName(name))
    {
        if (auto* value = obj->findProperty("value"))
        {
            return value->getValue<int32>();
        }
    }
    return 0;
}

template <>
inline float Console::getConvarValue(const std::string& name)
{
    if (auto* obj = Console::convars.findObjectWithName(name))
    {
        if (auto* value = obj->findProperty("value"))
        {
            return value->getValue<float>();
        }
    }
    return 0.f;
}

template <>
inline bool Console::getConvarValue(const std::string& name)
{
    if (auto* obj = Console::convars.findObjectWithName(name))
    {
        if (auto* value = obj->findProperty("value"))
        {
            return value->getValue<bool>();
        }
    }
    return false;
}
