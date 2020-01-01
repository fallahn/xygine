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
inline std::string ConfigProperty::getValue<std::string>() const
{
    return m_value;
}

template <>
inline sf::Int32 ConfigProperty::getValue<sf::Int32>() const
{
    sf::Int32 retVal;
    std::istringstream is(m_value);
    if (is >> retVal) return retVal;
    return 0;
}

template <>
inline float ConfigProperty::getValue<float>() const
{
    float retVal;
    std::istringstream is(m_value);
    if (is >> retVal) return retVal;
    return 0.f;
}

template <>
inline bool ConfigProperty::getValue<bool>() const
{
    return (m_value == "true");
}

template <>
inline sf::Vector2f ConfigProperty::getValue<sf::Vector2f>() const
{
    auto values = valueAsArray();

    return { values[0], values[1] };
}

template <>
inline sf::Vector2i ConfigProperty::getValue<sf::Vector2i>() const
{
    auto values = valueAsArray();
    
    return { static_cast<sf::Int32>(values[0]), static_cast<sf::Int32>(values[1]) };
}

template <>
inline sf::Vector2u ConfigProperty::getValue<sf::Vector2u>() const
{
    auto values = valueAsArray();
    
    return { static_cast<sf::Uint32>(values[0]), static_cast<sf::Uint32>(values[1]) };
}


template <>
inline sf::Vector3f ConfigProperty::getValue<sf::Vector3f>() const
{
    auto values = valueAsArray();

    return { values[0], values[1], values[2] };
}

template <>
inline sf::FloatRect ConfigProperty::getValue<sf::FloatRect>() const
{
    auto values = valueAsArray();
    return { values[0], values[1], values[2], values[3] };
}

template <>
inline sf::Color ConfigProperty::getValue<sf::Color>() const
{
    auto values = valueAsArray();
    return { static_cast<sf::Uint8>(values[0]), static_cast<sf::Uint8>(values[1]), static_cast<sf::Uint8>(values[2]), static_cast<sf::Uint8>(values[3]) };
}

template <>
inline xy::Vector4f ConfigProperty::getValue<xy::Vector4f>() const
{
    auto values = valueAsArray();
    return { values[0], values[1], values[2], values[3] };
}
