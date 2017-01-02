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

#include <CaveDemoDrawable.hpp>

#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <map>
#include <unordered_set>

using namespace CaveDemo;

namespace
{
    const float cellSize = 25.f;
    const std::size_t width = static_cast<std::size_t>(std::ceil(1970.f / cellSize));
    const std::size_t height = static_cast<std::size_t>(std::ceil(1130.f / cellSize));

    const int randThreshold = 54; //TODO make this a property

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
    m_tileData      (width * height),
    m_texture       (nullptr),
    m_normalMap     (nullptr),
    m_maskMap       (nullptr)
{
    fillRand();
    for (auto i = 0u; i < 5; ++i)
    {
        smooth();
    }

    buildVertexArray();


    //for now add quads to visualise output
    /*for (auto i = 0u; i < m_tileData.size(); ++i)
    {
        if (m_tileData[i] > 0)
        {
            addQuad(i);
        }
    }*/

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
    //xy::Util::Random::rndEngine.seed(12345);
    const auto border = 2;
    for (auto i = 0u; i < m_tileData.size(); ++i)
    {
        auto coord = coordFromIndex(i);
        if ((coord.x >= border && coord.x < width - border) &&
            (coord.y >= border && coord.y < height - border))
        {
            m_tileData[i] = (xy::Util::Random::value(0, 100) > randThreshold) ? 1u : 0u;
        }
        else
        {
            m_tileData[i] = 1u;
        }
    }
    //xy::Util::Random::rndEngine.seed(static_cast<unsigned long>(std::time(0)));
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
                if (x != coord.x || y != coord.y)
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
    static const sf::Color colour(120u, 135u, 130u);
    
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
        sf::Vector2f position(coord.x * cellSize + (cellSize / 2.f), coord.y * cellSize + (cellSize / 2.f));
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
    
    //functional programming ftw
    std::vector<sf::Vector2f> vertices;//vertex positions
    std::vector<std::size_t> indices; //indices into vertex array
    std::map<sf::Int32, std::vector<Triangle>> triangles; //cache triangles to search edges

    std::function<void(std::vector<Node*>& points)> meshFromPoints = [&vertices, &indices, &triangles](std::vector<Node*>& points)
    {
        std::function<void(sf::Int32, const Triangle&)> cacheTriangle = [&triangles](sf::Int32 idx, const Triangle& t)
        {
            auto result = triangles.find(idx);
            if (result != triangles.end())
            {
                result->second.push_back(t);
            }
            else
            {
                std::vector<Triangle> tris;
                tris.push_back(t);
                triangles.insert(std::make_pair(idx, tris));
            }
        };

        std::function<void(sf::Int32, sf::Int32, sf::Int32)> indexTriangle = [&](sf::Int32 a, sf::Int32 b, sf::Int32 c)
        {
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(c);

            Triangle t(a, b, c);
            cacheTriangle(a, t);
            cacheTriangle(b, t);
            cacheTriangle(c, t);
        };
        
        for (auto& p : points)
        {
            if (p->idx == -1)
            {
                p->idx = vertices.size();
                vertices.push_back(p->position);
            }
        }

        if (points.size() >= 3u)
        {
            indexTriangle(points[0]->idx, points[1]->idx, points[2]->idx);
        }
        if (points.size() >= 4u)
        {
            indexTriangle(points[0]->idx, points[2]->idx, points[3]->idx);
        }
        if (points.size() >= 5u)
        {
            indexTriangle(points[0]->idx, points[3]->idx, points[4]->idx);
        }
        if (points.size() >= 6u)
        {
            indexTriangle(points[0]->idx, points[4]->idx, points[5]->idx);
        }
    };

    std::unordered_set<sf::Int32> checkedVerts; //track indices we've already checked for edges
    std::function<void(Square&)> triangulate = [&meshFromPoints, &checkedVerts](Square& square)
    {
        std::vector<Node*> points;

        switch (square.mask)
        {
        case 0:
        default:
            break;
            //1 point
        case 1:
            points = { &square.centreLeft, &square.centreBottom, &square.bottomLeft };
            break;
        case 2:
            points = { &square.bottomRight, &square.centreBottom, &square.centreRight };
            break;
        case 4:
            points = { &square.topRight, &square.centreRight, &square.centreTop };
            break;
        case 8:
            points = { &square.topLeft, &square.centreTop, &square.centreLeft };
            break;
            //2 points
        case 3:
            points = { &square.centreRight, &square.bottomRight, &square.bottomLeft, &square.centreLeft };
            break;
        case 6:
            points = { &square.centreTop, &square.topRight, &square.bottomRight, &square.centreBottom };
            break;
        case 9:
            points = { &square.topLeft, &square.centreTop, &square.centreBottom, &square.bottomLeft };
            break;
        case 12:
            points = { &square.topLeft, &square.topRight, &square.centreRight, &square.centreLeft };
            break;
        case 5:
            points = { &square.centreTop, &square.topRight, &square.centreRight, &square.centreBottom, &square.bottomLeft, &square.centreLeft };
            break;
        case 10:
            points = { &square.topLeft, &square.centreTop, &square.centreRight, &square.bottomRight, &square.centreBottom, &square.centreLeft };
            break;
            //3 points
        case 7:
            points = { &square.centreTop, &square.topRight, &square.bottomRight, &square.bottomLeft, &square.centreLeft };
            break;
        case 11:
            points = { &square.topLeft, &square.centreTop, &square.centreRight, &square.bottomRight, &square.bottomLeft };
            break;
        case 13:
            points = { &square.topLeft, &square.topRight, &square.centreRight, &square.centreBottom, &square.bottomLeft };
            break;
        case 14:
            points = { &square.topLeft, &square.topRight, &square.bottomRight, &square.centreBottom, &square.centreLeft };
            break;
            //4 points
        case 15:
            points = { &square.topLeft, &square.topRight, &square.bottomRight, &square.bottomLeft };
            //we can makes sure to always skip these indices as we are guarenteed they are not on an edge
            //but we have to make sure mesh from points is called *first* so that correct indices are assigned
            meshFromPoints(points);
            checkedVerts.insert(points[0]->idx);
            checkedVerts.insert(points[1]->idx);
            checkedVerts.insert(points[2]->idx);
            checkedVerts.insert(points[3]->idx);
            return;
        }
        meshFromPoints(points);
    };

    for (auto& s : squares)
    {
        triangulate(s);
    }

    //build final vertex array - TODO would be nice if SFML supported drawing vertices from index arrays
    for (const auto& i : indices)
    {
        m_vertices.emplace_back(sf::Vertex(vertices[i], sf::Color::White/*(112u, 80u, 46u)*/, vertices[i]));
    }

    //---------search the triangle cache to find the edges----------
    std::function<bool(sf::Int32, sf::Int32)> isEdge = [&](sf::Int32 vertexA, sf::Int32 vertexB)
    {
        XY_ASSERT(vertexA < triangles.size(), "");
        const auto& triList = triangles[vertexA];
        auto sharedTriangleCount = 0;

        for (const auto& tri : triList)
        {
            if (tri.contains(vertexB))
            {
                sharedTriangleCount++;
                if (sharedTriangleCount > 1)
                {
                    break;
                }
            }
        }
        return (sharedTriangleCount == 1);
    };

    std::function<sf::Int32(sf::Int32)> getConnectedVert = [&](sf::Int32 idx)
    {
        XY_ASSERT(idx < triangles.size(), "");
        const auto& triList = triangles[idx];
        for (const auto& tri : triList)
        {
            for (auto i = 0; i < 3; ++i)
            {
                sf::Int32 vertexB = tri[i];
                if (vertexB != idx && (checkedVerts.find(vertexB) == checkedVerts.end()))
                {
                    if (isEdge(idx, vertexB))
                    {
                        //LOG("is edge", xy::Logger::Type::Info);
                        return vertexB;
                    }
                }
            }
        }

        return -1;
    };

    std::function<void(sf::Int32, sf::Int32)> followEdge = [&, this](sf::Int32 vertIdx, sf::Int32 edgeIdx)
    {
        m_edges[edgeIdx].push_back(vertices[vertIdx]);
        checkedVerts.insert(vertIdx);
        auto nextVertIndex = getConnectedVert(vertIdx);

        if (nextVertIndex != -1 /*&& m_edges.back().size() < 100*/)
        {
            followEdge(nextVertIndex, edgeIdx);
        }
    };

    for (auto vertIdx = 0u; vertIdx < vertices.size(); ++vertIdx)
    {
        if (checkedVerts.find(vertIdx) == checkedVerts.end())
        {
            auto newEdgeVert = getConnectedVert(vertIdx);
            if (newEdgeVert != -1)
            {
                checkedVerts.insert(vertIdx);

                std::vector<sf::Vector2f> newEdge;
                newEdge.push_back(vertices[vertIdx]);
                m_edges.push_back(newEdge);
                followEdge(newEdgeVert, m_edges.size() - 1);
                m_edges.back().push_back(vertices[vertIdx]);
            }
        }
    }
}

void CaveDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = m_texture;
    auto shader = getShader();
    
    if (shader)
    {
        shader->setUniform("u_diffuseMap", *m_texture);
        shader->setUniform("u_normalMap", *m_normalMap);
        shader->setUniform("u_maskMap", *m_maskMap);
        shader->setUniform("u_inverseWorldViewMatrix", sf::Glsl::Mat4(states.transform.getInverse()));
        states.shader = shader;
    }

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
}