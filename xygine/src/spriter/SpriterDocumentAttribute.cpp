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

#include <xygine/spriter/DocumentAttribute.hpp>

using namespace xy::Spriter::Detail;

DocumentAttribute::DocumentAttribute(const pugi::xml_attribute& atb)
    :m_attribute(atb) {}

//public
std::string DocumentAttribute::getName() const
{
    return m_attribute.name();
}

float DocumentAttribute::valueAsFloat() const
{
    return m_attribute.as_float();
}

sf::Int32 DocumentAttribute::valueAsInt() const
{
    return m_attribute.as_int();
}

std::string DocumentAttribute::valueAsString() const
{
    return m_attribute.as_string();
}

void DocumentAttribute::advanceNext()
{
    m_attribute = m_attribute.next_attribute();
}