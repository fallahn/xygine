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

#include <xygine/spriter/DocumentElement.hpp>
#include <xygine/spriter/DocumentAttribute.hpp>

using namespace xy::Spriter::Detail;

DocumentElement::DocumentElement(const pugi::xml_node& node)
    : m_node(node) {}

//public
std::string DocumentElement::getName() const
{
    return m_node.name();
}

void DocumentElement::advanceNext()
{
    m_node = m_node.next_sibling();
}

void DocumentElement::advanceNextSameName()
{
    m_node = m_node.next_sibling(m_node.name());
}

DocumentAttribute DocumentElement::firstAttribute() const
{
    return std::move(DocumentAttribute(m_node.first_attribute()));
}

DocumentAttribute DocumentElement::firstAttribute(const std::string& name) const
{
    return std::move(DocumentAttribute(m_node.attribute(name.c_str())));
}

DocumentElement DocumentElement::firstElement() const
{
    return std::move(DocumentElement(m_node.first_child()));
}

DocumentElement DocumentElement::firstElement(const std::string& name) const
{
    return std::move(DocumentElement(m_node.child(name.c_str())));
}

DocumentElement DocumentElement::nextSibling() const
{
    return std::move(DocumentElement(m_node.next_sibling()));
}

//private
pugi::xml_attribute DocumentElement::findAttribute(const std::string& name) const
{
    return m_node.attribute(name.c_str());
}