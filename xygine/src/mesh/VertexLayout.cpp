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

#include <xygine/mesh/VertexLayout.hpp>
#include <xygine/Assert.hpp>

using namespace xy;

VertexLayout::Element::Element()
    : type  (VertexLayout::Element::Type::NONE),
    size    (0u){}

VertexLayout::Element::Element(VertexLayout::Element::Type t, std::size_t s)
    : type  (t),
    size    (s){}

bool VertexLayout::Element::operator== (const VertexLayout::Element& e) const
{
    return (e.type == type && e.size == size);
}

bool VertexLayout::Element::operator!= (const VertexLayout::Element& e) const
{
    return !(*this == e);
}

//---------------------------------------------//

VertexLayout::VertexLayout(const std::vector<VertexLayout::Element>& elements)
    : m_vertexSize(0u)
{
    for (const auto& e : elements)
    {
        m_vertexSize += sizeof(float) * e.size;
        m_elements.push_back(e);
    }
}

//public
const VertexLayout::Element& VertexLayout::getElement(std::size_t idx) const
{
    XY_ASSERT(idx < m_elements.size(), "VertexLayout element: index out of range");
    return m_elements[idx];
}

std::size_t VertexLayout::getElementCount() const
{
    return m_elements.size();
}

std::size_t VertexLayout::getVertexSize() const
{
    return m_vertexSize;
}

sf::Int32 VertexLayout::getElementIndex(VertexLayout::Element::Type type) const
{
    for (auto i = 0u; i < m_elements.size(); ++i)
    {
        if (m_elements[i].type == type) return i;
    }
    return -1;
}

sf::Int32 VertexLayout::getElementOffset(VertexLayout::Element::Type type) const
{
    sf::Int32 offset = 0;
    for (auto i = 0u; i < m_elements.size(); ++i)
    {
        if (m_elements[i].type == type) return offset;
        offset += m_elements[i].size;
    }
    return -1;
}

bool VertexLayout::operator== (const VertexLayout& vl) const
{
    if (vl.m_elements.size() != m_elements.size()) return false;

    for (auto i = 0u; i < m_elements.size(); ++i)
    {
        if (vl.m_elements[i] != m_elements[i]) return false;
    }
    return true;
}

bool VertexLayout::operator!= (const VertexLayout& vl) const
{
    return !(*this == vl);
}

//private