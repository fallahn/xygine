/*********************************************************************
� Matt Marchant 2014 - 2017
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

#include <xygine/physics/DebugDrawer.hpp>
#include <xygine/physics/World.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>

using namespace xy;
using namespace xy::Physics;

namespace
{
    const sf::Uint8 alpha = 140u;
}

DebugDraw::DebugDraw(sf::RenderTarget& rt)
    :m_renderTarget (rt)
{
    SetFlags(e_shapeBit | e_jointBit /*| e_aabbBit*/ | e_pairBit | e_centerOfMassBit);
}

//public
void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& colour)
{
    std::vector<sf::Vertex> verts;
    sf::Color newColour = World::boxToSfColour(colour);
    for (auto i = 0; i < vertexCount; ++i)
    {
        verts.emplace_back(sf::Vertex(World::boxToSfVec(vertices[i]), newColour));
    }
    m_renderTarget.draw(verts.data(), verts.size(), sf::PrimitiveType::LinesStrip);
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& colour)
{
    m_convexShape.setPointCount(vertexCount);
    for (auto i = 0; i < vertexCount; ++i)
    {
        m_convexShape.setPoint(i, World::boxToSfVec(vertices[i]));
    }
    auto newColour = World::boxToSfColour(colour);
    m_convexShape.setOutlineColor(newColour);
    m_convexShape.setOutlineThickness(-1.f);
    newColour.a = alpha;
    m_convexShape.setFillColor(newColour);
    m_renderTarget.draw(m_convexShape);
}

void DebugDraw::DrawCircle(const b2Vec2& centre, float32 radius, const b2Color& colour)
{
    float newRadius = World::boxToSfFloat(radius);
    m_circleShape.setRadius(newRadius);
    m_circleShape.setOrigin(newRadius, newRadius);
    m_circleShape.setPosition(World::boxToSfVec(centre));
    m_circleShape.setOutlineColor(World::boxToSfColour(colour));
    m_circleShape.setOutlineThickness(-1.f);
    m_circleShape.setFillColor(sf::Color::Transparent);
    m_renderTarget.draw(m_circleShape);
}

void DebugDraw::DrawSolidCircle(const b2Vec2& centre, float32 radius, const b2Vec2& axis, const b2Color& colour)
{
    float newRadius = World::boxToSfFloat(radius);
    m_circleShape.setRadius(newRadius);
    m_circleShape.setOrigin(newRadius, newRadius);
    m_circleShape.setPosition(World::boxToSfVec(centre));

    auto newColour = World::boxToSfColour(colour);
    m_circleShape.setOutlineColor(newColour);
    m_circleShape.setOutlineThickness(-1.f);
    newColour.a = alpha;
    m_circleShape.setFillColor(newColour);
    m_renderTarget.draw(m_circleShape);
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& colour)
{
    sf::Color newColour = World::boxToSfColour(colour);
    std::vector<sf::Vertex> verts = 
    {
        {World::boxToSfVec(p1), newColour},
        {World::boxToSfVec(p2), newColour}
    };

    m_renderTarget.draw(verts.data(), verts.size(), sf::PrimitiveType::LinesStrip);
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 p1 = xf.p;
    const float k_axisScale = 0.4f * (100.f / World::m_instance->getWorldScale());

    b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
    DrawSegment(p1, p2, b2Color(1, 0, 0));

    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    DrawSegment(p1, p2, b2Color(0, 1, 0));
}

//this was added in B2D 2.3.2 but I can't seem to find  a release for that...

//void DebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& colour)
//{
//    size *= 2.f;
//    sf::Color newColour = World::boxToSfColour(colour);
//    std::vector<sf::Vertex> verts =
//    {
//        { World::boxToSfVec(p) - sf::Vector2f(size, 0.f), newColour },
//        { World::boxToSfVec(p) + sf::Vector2f(size, 0.f), newColour },
//        { World::boxToSfVec(p) + sf::Vector2f(size, 0.f), sf::Color::Transparent },
//        { World::boxToSfVec(p) + sf::Vector2f(0.f, size), sf::Color::Transparent },
//        { World::boxToSfVec(p) + sf::Vector2f(0.f, size), newColour },
//        { World::boxToSfVec(p) - sf::Vector2f(0.f, size), newColour }
//    };
//
//    m_renderTarget.draw(verts.data(), verts.size(), sf::PrimitiveType::LinesStrip);
//}