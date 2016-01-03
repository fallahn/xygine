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

#include <RacingDemoTrack.hpp>

#include <xygine/Util.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>

namespace
{
    const float SEGMENT_LENGTH = 200.f;
    const float CAMERA_DEPTH = (1.f / std::tan(50.f * degToRad));
    const float CAMERA_HEIGHT = 1000.f;
    const std::size_t RUMBLE_LENGTH = 3u;
    const std::size_t DRAW_DISTANCE = 500u;

    const float VIEW_WIDTH = 1920.f;
    const float VIEW_HEIGHT = 1080.f;
    const float TRACK_WIDTH = 2000.f;
    const float FOG_DENSITY = 6.8f;

    float easeIn(float a, float b, float percent)
    {
        return a + (b - a) * std::pow(percent, 2.f);
    }

    float easeOut(float a, float b, float percent)
    {
        return a + (b - a) * (1 - std::pow(1.f - percent, 2.f));
    }

    float easeInOut(float a, float b, float percent)
    {
        return a + (b - a) * (static_cast<float>(-std::cos(percent * PI) / 2.f) + 0.5f);
    }

    float percentRemaining(float n, float total)
    {
        return (static_cast<int>(n) % static_cast<int>(total)) / total;
    }

    float interpolate(float a, float b, float percent)
    {
        return a + (b - a) * percent;
    }

    float exponentialFog(float distance, float density)
    {
        return static_cast<float>(1 / std::pow(E, (distance * distance * density)));
    }
}

Track::Track(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_distance      (0.f),
    m_trackLength   (0.f),
    m_texture       (nullptr)
{
    auto num = 50.f;
    addTrackSection(num, num, num, 0.f, 0.f);
   
    //auto height = 40.f;
    //addTrackSection(num, num, num, 0, height / 2.f);
    //addTrackSection(num, num, num, 0, -height);
    //addTrackSection(num, num, num, 0, height);
    //addTrackSection(num, num, num, 0, 0);
    //addTrackSection(num, num, num, 0, height / 2.f);
    //addTrackSection(num, num, num, 0, 0);


    m_trackLength = SEGMENT_LENGTH * m_segments.size();
}

//public
void Track::entityUpdate(xy::Entity&, float dt)
{
    m_distance += 12400.f * dt;

    while (m_distance >= m_trackLength)
        m_distance -= m_trackLength;

    while (m_distance < 0)
        m_distance += m_trackLength;
}

void Track::setTexture(const sf::Texture* t)
{
    m_texture = t;

    for (auto& seg : m_segments) seg->setTexture(t);
}

//private
void Track::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    const auto& baseSegment = *m_segments[static_cast<std::size_t>(std::floor(m_distance / SEGMENT_LENGTH)) % m_segments.size()];
    auto basePercent = percentRemaining(m_distance, SEGMENT_LENGTH);;

    float x = 0.f;
    float dx = -(baseSegment.getCurve() * basePercent);
    float maxy = VIEW_HEIGHT;

    for (auto n = 0u; n < DRAW_DISTANCE; ++n)
    {
        auto& segment = *m_segments[(baseSegment.getIndex() + n) % m_segments.size()];
        segment.setClip(maxy);

        bool looped = segment.getIndex() < baseSegment.getIndex();
        auto fog = exponentialFog(n / static_cast<float>(DRAW_DISTANCE), FOG_DENSITY);

        auto camX = 0.f;// mPlayer->getOffset() * roadWidth;
        auto camY = CAMERA_HEIGHT;// playerY + mCameraHeight;
        auto camZ = m_distance - (looped ? m_trackLength : 0.f);

        auto& pointA = segment.pointA();
        auto& pointB = segment.pointB();

        pointA.project(camX - x, camY, camZ, CAMERA_DEPTH, VIEW_WIDTH, VIEW_HEIGHT, TRACK_WIDTH);
        pointB.project(camX - x - dx, camY, camZ, CAMERA_DEPTH, VIEW_WIDTH, VIEW_HEIGHT, TRACK_WIDTH);

        x += dx;
        dx += segment.getCurve();

        if ((pointA.camera.z <= CAMERA_DEPTH) || (pointB.screen.y >= maxy || pointB.screen.y >= pointA.screen.y))
            continue;

        segment.updateVerts(TRACK_WIDTH, fog);

        rt.draw(segment, states);

        maxy = pointB.screen.y;
    }
}

void Track::addTrackSegment(float curve, float y)
{
    std::size_t segCount = m_segments.size();
    const float oldY = prevY();

    m_segments.emplace_back(std::make_unique<Segment>());

    auto& newSeg = m_segments.back();
    newSeg->setIndex(segCount);
    newSeg->setCurve(curve);
    
    newSeg->pointA().world.y = oldY;
    newSeg->pointB().world.y = y;

    newSeg->pointA().world.z = segCount * SEGMENT_LENGTH;
    newSeg->pointB().world.z = (segCount + 1u) * SEGMENT_LENGTH;

    (static_cast<std::size_t>(std::floor(segCount / RUMBLE_LENGTH)) % 2) ?
        newSeg->setPalette(Palette::Light) 
        :
        newSeg->setPalette(Palette::Dark);

    newSeg->setTexture(m_texture);
}

void Track::addTrackSection(float enter, float hold, float exit, float curve, float y)
{
    auto startY = prevY();
    auto endY = startY + (y * SEGMENT_LENGTH);
    auto total = enter + hold + exit;

    for (auto i = 0.f; i < enter; ++i)
        addTrackSegment(easeIn(0, curve, i / enter), easeInOut(startY, endY, i / total));

    for (auto i = 0.f; i < hold; ++i)
        addTrackSegment(curve, easeInOut(startY, endY, (enter + i) / total));

    for (auto i = 0.f; i < exit; ++i)
        addTrackSegment(easeInOut(curve, 0, i / exit), easeInOut(startY, endY, (enter + hold + i) / total));
}

float Track::prevY()
{
    return (m_segments.empty()) ? 0.f : m_segments.back()->pointB().world.y;
}

//-------segment------//
Track::Segment::Segment()
    : m_batch   (28u),
    m_landscape (m_batch, 4u),
    m_rumbleA   (m_batch, 4u),
    m_rumbleB   (m_batch, 4u),
    m_mainLane  (m_batch, 4u),
    m_laneA     (m_batch, 4u),
    m_laneB     (m_batch, 4u),
    m_fog       (m_batch, 4u),
    m_curve     (0.f),
    m_index     (0u),
    m_clip      (0.f)
{
    m_landscape.setVertexColour(m_palette.getGrass());
    m_rumbleA.setVertexColour(m_palette.getRumble());
    m_rumbleB.setVertexColour(m_palette.getRumble());
    m_mainLane.setVertexColour(m_palette.getTrack());
    m_laneA.setVertexColour(m_palette.getLane());
    m_laneB.setVertexColour(m_palette.getLane());
}

Track::Point& Track::Segment::pointA()
{
    return m_pointA;
}

Track::Point& Track::Segment::pointB()
{
    return m_pointB;
}

const Track::Point& Track::Segment::pointA() const
{
    return m_pointA;
}

const Track::Point& Track::Segment::pointB() const
{
    return m_pointB;
}

void Track::Segment::setPalette(Palette::Category cat)
{
    m_palette.setCategory(cat);

    m_landscape.setVertexColour(m_palette.getGrass());
    m_rumbleA.setVertexColour(m_palette.getRumble());
    m_rumbleB.setVertexColour(m_palette.getRumble());
    m_mainLane.setVertexColour(m_palette.getTrack());
    m_laneA.setVertexColour(m_palette.getLane());
    m_laneB.setVertexColour(m_palette.getLane());
}

void Track::Segment::setCurve(float curve)
{
    m_curve = curve;
}

float Track::Segment::getCurve() const
{
    return m_curve;
}

void Track::Segment::setIndex(std::size_t index)
{
    m_index = index;
}

std::size_t Track::Segment::getIndex() const
{
    return m_index;
}

void Track::Segment::setClip(float clip)
{
    m_clip = clip;
}

float Track::Segment::getClip() const
{
    return m_clip;
}

void Track::Segment::setTexture(const sf::Texture* t)
{
    m_batch.setTexture(t);
}

namespace
{
    float rumbleWidth(float projectedWidth, std::size_t laneCount)
    {
        return projectedWidth / std::max(6u, 2u * laneCount);
    }

    float laneMarkerWidth(float projectedWidth, std::size_t laneCount)
    {
        return projectedWidth / std::max(32u, 8u * laneCount);
    }

    const auto laneCount = 3u;
}

void Track::Segment::updateVerts(float width, float fog)
{
    XY_ASSERT(fog >= 0 && fog <= 1, "fog value must be normalised");

    //landscape
    std::vector<sf::Vector2f> positions = 
    {
        { 0.f, m_pointB.screen.y },
        { width, m_pointB.screen.y },
        { width, m_pointA.screen.y },
        { 0.f, m_pointA.screen.y }
    };
    m_landscape.setVertexPositions(positions);
    m_landscape.setTextureCoords(positions);
    
    //fog
    m_fog.setVertexPositions(positions);
    m_fog.setVertexColour(sf::Color(100u, 180u, 100u, 255u - static_cast<sf::Uint8>(fog * 255.f)));

    //rumble strip
    auto rumbleWidth1 = rumbleWidth(m_pointA.screen.w, laneCount);
    auto rumbleWidth2 = rumbleWidth(m_pointB.screen.w, laneCount);

    positions = 
    {
        { m_pointA.screen.x - m_pointA.screen.w - rumbleWidth1, m_pointA.screen.y },
        { m_pointA.screen.x - m_pointA.screen.w, m_pointA.screen.y },
        { m_pointB.screen.x - m_pointB.screen.w, m_pointB.screen.y },
        { m_pointB.screen.x - m_pointB.screen.w - rumbleWidth2, m_pointB.screen.y } 
    };
    m_rumbleA.setVertexPositions(positions);
    m_rumbleA.setTextureCoords(positions);

    positions = 
    {
        { m_pointA.screen.x + m_pointA.screen.w + rumbleWidth1, m_pointA.screen.y },
        { m_pointA.screen.x + m_pointA.screen.w, m_pointA.screen.y },
        { m_pointB.screen.x + m_pointB.screen.w, m_pointB.screen.y },
        { m_pointB.screen.x + m_pointB.screen.w + rumbleWidth2, m_pointB.screen.y } 
    };
    m_rumbleB.setVertexPositions(positions);
    m_rumbleB.setTextureCoords(positions);

    //main lane
    positions = 
    {
        { m_pointA.screen.x - m_pointA.screen.w, m_pointA.screen.y },
        { m_pointA.screen.x + m_pointA.screen.w, m_pointA.screen.y },
        { m_pointB.screen.x + m_pointB.screen.w, m_pointB.screen.y },
        { m_pointB.screen.x - m_pointB.screen.w, m_pointB.screen.y } 
    };
    m_mainLane.setVertexPositions(positions);
    m_mainLane.setTextureCoords(positions);

    //lanes
    auto laneMarkerWidth1 = laneMarkerWidth(m_pointA.screen.w, laneCount) / 2.f;
    auto laneMarkerWidth2 = laneMarkerWidth(m_pointB.screen.w, laneCount) / 2.f;
    auto lanew1 = m_pointA.screen.w * 2.f / laneCount;
    auto lanew2 = m_pointB.screen.w * 2.f / laneCount;
    auto lanex1 = m_pointA.screen.x - m_pointA.screen.w + lanew1;
    auto lanex2 = m_pointB.screen.x - m_pointB.screen.w + lanew2;

    for (auto lane = 1u; lane < laneCount; lanex1 += lanew1 + 1, lanex2 += lanew2 + 1, lane++)
    {
        positions = 
        {
            { lanex1 - laneMarkerWidth1, m_pointA.screen.y },
            { lanex1 + laneMarkerWidth1, m_pointA.screen.y },
            { lanex2 + laneMarkerWidth2, m_pointB.screen.y },
            { lanex2 - laneMarkerWidth2, m_pointB.screen.y }
        };
        if (lane == 1)
        {
            m_laneA.setVertexPositions(positions);
            m_laneA.setTextureCoords(positions);
        }
        else
        {
            m_laneB.setVertexPositions(positions);
            m_laneB.setTextureCoords(positions);
        }
    }
}

void Track::Segment::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_batch);
}