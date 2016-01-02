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

#include <xygine/PolyBatch.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace xy;

PolyBatch::PolyBatch(std::size_t count, sf::PrimitiveType type)
    : m_primitiveType   (type),
    m_texture           (nullptr),
    m_vertices          (count),
    m_nextIndex         (0)
{}

xy::Polygon PolyBatch::addPolygon(std::size_t count)
{
    XY_ASSERT(m_nextIndex + count < m_vertices.size(), "Not enough vertices left in batch");

    xy::Polygon poly(*this, count);
    return std::move(poly);
}

void PolyBatch::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = m_texture;
    rt.draw(m_vertices.data(), m_nextIndex, m_primitiveType, states);
}

//-------Polygon-------//
xy::Polygon::Polygon(PolyBatch& pb, std::size_t count)
    : m_batch       (pb),
    m_vertexCount   (count),
    m_vertexOffset  (pb.m_nextIndex)
{
    pb.m_nextIndex += count;
}

xy::Polygon::~Polygon()
{
    setVertexColour(sf::Color::Transparent);
}

//public
std::size_t xy::Polygon::getVertexCount() const
{
    return m_vertexCount;
}

void xy::Polygon::setVertexPosition(const sf::Vector2f& position, std::size_t index)
{
    XY_ASSERT(index < m_vertexCount, "Index out of range");
    m_batch.m_vertices[m_vertexOffset + index].position = position;
}

void xy::Polygon::setVertexPositions(const std::vector<sf::Vector2f>& positions)
{
    XY_ASSERT(positions.size() <= m_vertexCount, "too many vertex positions");
    for (auto i = m_vertexOffset, j = 0u; j < positions.size(); ++i, ++j)
    {
        m_batch.m_vertices[i].position = positions[j];
    }
}

void xy::Polygon::setVertexColour(const sf::Color& colour)
{
    auto end = m_vertexOffset + m_vertexCount;
    for (auto i = m_vertexOffset; i < end; ++i)
    {
        m_batch.m_vertices[i].color = colour;
    }
}

void xy::Polygon::setVertexColours(const std::vector<sf::Color>& colours)
{
    XY_ASSERT(colours.size() <= m_vertexCount, "too many vertex colours");
    for (auto i = m_vertexOffset, j = 0u; j < colours.size(); ++i, ++j)
    {
        m_batch.m_vertices[i].color = colours[j];
    }
}

void xy::Polygon::setTextureCoord(const sf::Vector2f& coord, std::size_t index)
{
    XY_ASSERT(index < m_vertexCount, "index out of range");
    m_batch.m_vertices[m_vertexOffset + index].texCoords = coord;
}

void xy::Polygon::setTextureCoords(const std::vector<sf::Vector2f>& coords)
{
    XY_ASSERT(coords.size() <= m_vertexCount, "too many vertex coordinates");
    for (auto i = m_vertexOffset, j = 0u; j < coords.size(); ++i, ++j)
    {
        m_batch.m_vertices[i].texCoords = coords[j];
    }
}

void xy::Polygon::setVertices(const std::vector<sf::Vertex>& verts)
{
    XY_ASSERT(verts.size() <= m_vertexCount, "too many vertices");
    for (auto i = m_vertexOffset, j = 0u; j < verts.size(); ++i, ++j)
    {
        m_batch.m_vertices[i] = verts[i];
    }
}