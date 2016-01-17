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

#include <CaveDemoDrawable.hpp>

#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace CaveDemo;

namespace
{
    const float cellSize = 25.f;
    const std::size_t width = static_cast<std::size_t>(std::ceil(1920.f / cellSize));
    const std::size_t height = static_cast<std::size_t>(std::ceil(1080.f / cellSize));

    const int randThreshold = 53; //TODO make this a property

    std::size_t indexFromCoord(const sf::Vector2i& coord)
    {
        return width * coord.y + coord.x;
    }

    sf::Vector2i coordFromIndex(std::size_t idx)
    {
        return sf::Vector2i(idx % width, idx / width );
    }
}

CaveDrawable::CaveDrawable(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_tileData      (width * height)
{
    fillRand();
    for (auto i = 0u; i < 5; ++i)
    {
        smooth();
    }

    buildVertexArray();


    //for now add quads to visualise output
    //TODO generate proper mesh
    for (auto i = 0u; i < m_tileData.size(); ++i)
    {
        if (m_tileData[i] > 0)
        {
            addQuad(i);
        }
    }
    //TODO check position is offset correctly
    m_globalBounds.width = static_cast<float>(width) * cellSize;
    m_globalBounds.height = static_cast<float>(height) * cellSize;
}

//public
void CaveDrawable::entityUpdate(xy::Entity&, float)
{


}

sf::Vector2f CaveDrawable::getSize() const
{
    return sf::Vector2f(static_cast<float>(width), static_cast<float>(height)) * cellSize;
}

sf::FloatRect CaveDrawable::globalBounds() const
{
    return m_globalBounds;
}

//private
void CaveDrawable::fillRand()
{
    for (auto& t : m_tileData)
    {
        t = (xy::Util::Random::value(0, 100) > randThreshold) ? 1u : 0u;
    }
}

void CaveDrawable::smooth()
{
    for (auto i = 0u; i < m_tileData.size(); ++i)
    {
        auto count = getNeighbourCount(i);

        if (count < 4) m_tileData[i] = 0u;
        else if (count > 4) m_tileData[i] = 1u;
    }
}

sf::Uint8 CaveDrawable::getNeighbourCount(std::size_t idx)
{
    sf::Uint8 retVal = 0u;
    auto coord = coordFromIndex(idx);

    for (auto x = coord.x - 1; x <= coord.x + 1; ++x)
    {
        for (auto y = coord.y - 1; y <= coord.y + 1; ++y)
        {
            if (x >= 0 && x < width && y >= 0 && y < height)
            {
                if (x != coord.x || y != coord.y) //&& or || ?
                {
                    retVal += m_tileData[indexFromCoord({ x, y })];
                }
            }
            else
            {
                retVal++;
            }
        }
    }

    return retVal;
}

void CaveDrawable::addQuad(std::size_t idx)
{
    static const sf::Color colour(20u, 35u, 30u);
    
    auto coord = sf::Vector2f(coordFromIndex(idx));
    coord *= cellSize;
    m_vertices.emplace_back(coord, colour);
    m_vertices.emplace_back(sf::Vector2f(coord.x + cellSize, coord.y), colour);
    m_vertices.emplace_back(sf::Vector2f(coord.x + cellSize, coord.y + cellSize), colour);
    m_vertices.emplace_back(sf::Vector2f(coord.x, coord.y + cellSize), colour);
}

void CaveDrawable::buildVertexArray()
{
    //TODO we could split the map into smaller sections which would help culling and allow
    //fast rebuilding of areas for destruction

    std::vector<ControlNode> controlNodes;
    for (auto i = 0u; i < m_tileData.size(); ++i)
    {
        auto coord = coordFromIndex(i);
        auto size = getSize();
        sf::Vector2f position(-(size.x / 2.f) + coord.x * cellSize + (cellSize / 2.f), -(size.y / 2.f) + coord.y * cellSize + (cellSize / 2.f));
        controlNodes.emplace_back((m_tileData[i] == 1), position, cellSize);
    }

    std::vector<Square> squares;
    for (auto x = 0u; x < width - 1; ++x)
    {
        for (auto y = 0u; y < height - 1; ++y)
        {
            squares.emplace_back(controlNodes[indexFromCoord(sf::Vector2i(x, y + 1))],
                                controlNodes[indexFromCoord(sf::Vector2i(x + 1, y + 1))],
                                controlNodes[indexFromCoord(sf::Vector2i(x + 1, y))],
                                controlNodes[indexFromCoord(sf::Vector2i(x, y))]);
        }
    }
    int buns = 0;
}

void CaveDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}