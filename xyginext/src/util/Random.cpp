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

/*
Poisson Disc sampling based on article at http://devmag.org.za/2009/05/03/poisson-disk-sampling/
*/

#include "xyginext/util/Random.hpp"
#include "xyginext/util/Vector.hpp"

using namespace xy::Util::Random;

namespace
{
    const std::size_t maxGridPoints = 3;

    //it's not desirable but the only way I can think to hide this class
    class Grid
    {
    public:
        Grid(const sf::FloatRect& area, std::size_t maxPoints)
            : m_area(area), m_maxPoints(maxPoints)
        {
            XY_ASSERT(maxPoints < sizeof(std::size_t), "max points must be less than " + std::to_string(sizeof(std::size_t)));
            resize(area, maxPoints);
        }
        ~Grid() = default;

        void addPoint(const sf::Vector2f& point)
        {
            auto x = static_cast<std::size_t>(point.x + m_cellOffset.x) >> m_maxPoints;
            auto y = static_cast<std::size_t>(point.y + m_cellOffset.y) >> m_maxPoints;
            auto idx = y * m_cellCount.x + x;

            if (idx < m_cells.size()) m_cells[idx].push_back(point);
        }

        void resize(std::size_t maxPoints)
        {
            m_maxPoints = maxPoints;
            m_cellSize = 1ULL << m_maxPoints;
            m_cellOffset = { static_cast<int>(std::abs(m_area.left)), static_cast<int>(std::abs(m_area.top)) };
            m_cellCount = { static_cast<int>(std::ceil(m_area.width / static_cast<float>(m_cellSize))), static_cast<int>(std::ceil(m_area.height / static_cast<float>(m_cellSize))) };
            m_cells.clear();
            m_cells.resize(m_cellCount.x * m_cellCount.y);
        }

        void resize(const sf::FloatRect& area, std::size_t maxPoints)
        {
            m_area = area;
            resize(maxPoints);
        }

        bool hasNeighbour(const sf::Vector2f& point, float radius) const
        {
            const float radSqr = radius * radius;

            sf::Vector2i minDist
            (
                static_cast<int>(std::max(std::min(point.x - radius, (m_area.left + m_area.width) - 1.f), m_area.left)),
                static_cast<int>(std::max(std::min(point.y - radius, (m_area.top + m_area.height) - 1.f), m_area.top))
            );

            sf::Vector2i maxDist
            (
                static_cast<int>(std::max(std::min(point.x + radius, (m_area.left + m_area.width) - 1.f), m_area.left)),
                static_cast<int>(std::max(std::min(point.y + radius, (m_area.top + m_area.height) - 1.f), m_area.top))
            );

            sf::Vector2i minCell
            (
                (minDist.x + m_cellOffset.x) >> m_maxPoints,
                (minDist.y + m_cellOffset.y) >> m_maxPoints
            );

            sf::Vector2i maxCell
            (
                std::min(1 + ((maxDist.x + m_cellOffset.x) >> m_maxPoints), m_cellCount.x),
                std::min(1 + ((maxDist.y + m_cellOffset.y) >> m_maxPoints), m_cellCount.y)
            );

            for (auto y = minCell.y; y < maxCell.y; ++y)
            {
                for (auto x = minCell.x; x < maxCell.x; ++x)
                {
                    for (const auto& cell : m_cells[y * m_cellCount.x + x])
                    {
                        if (xy::Util::Vector::lengthSquared(point - cell) < radSqr)
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

    private:
        using Cell = std::vector<sf::Vector2f>;
        std::vector<Cell> m_cells;
        sf::Vector2i m_cellCount;
        sf::Vector2i m_cellOffset;
        sf::FloatRect m_area;
        std::size_t m_maxPoints;
        std::size_t m_cellSize;
    };
}

std::vector<sf::Vector2f> xy::Util::Random::poissonDiscDistribution(const sf::FloatRect& area, float minDist, std::size_t maxPoints)
{
    std::vector<sf::Vector2f> workingPoints;
    std::vector<sf::Vector2f> retVal;

    Grid grid(area, maxGridPoints);

    auto centre = (sf::Vector2f(area.width, area.height) / 2.f) + sf::Vector2f(area.left, area.top);
    workingPoints.push_back(centre);
    retVal.push_back(centre);
    grid.addPoint(centre);

    while (!workingPoints.empty())
    {
        auto idx = (workingPoints.size() == 1) ? 0 : value(0, workingPoints.size() - 1);
        centre = workingPoints[idx];

        workingPoints.erase(std::begin(workingPoints) + idx);

        for (auto i = 0u; i < maxPoints; ++i)
        {
            float radius = minDist * (1.f + value(0.f, 1.f));
            float angle = value(-1.f, 1.f) * xy::Util::Const::PI;
            sf::Vector2f newPoint = centre + sf::Vector2f(std::sin(angle), std::cos(angle)) * radius;

            if (area.contains(newPoint) && !grid.hasNeighbour(newPoint, minDist))
            {
                workingPoints.push_back(newPoint);
                retVal.push_back(newPoint);
                grid.addPoint(newPoint);
            }
        }
    }

    return std::move(retVal);
}
