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

//drawable component for rendering the track in the racing demo

#ifndef RACING_DEMO_TRACK_HPP_
#define RACING_DEMO_TRACK_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector3.hpp>

class Track final : public sf::Drawable, public xy::Component
{
public:
    explicit Track(xy::MessageBus&);
    ~Track() = default;

    xy::Component::Type type() const override { return xy::Component::Type::Drawable; }

    void entityUpdate(xy::Entity&, float) override;
    void setDistance(float distance) { m_distance = distance; }
    void setTexture(const sf::Texture&);

private:

    struct Point final
    {
        struct Screen final
        {
            float x = 0.f;
            float y = 0.f;
            float w = 0.f;
            float scale = 1.f;
        }screen;

        sf::Vector3f world;
        sf::Vector3f camera;

        void project(float camX, float camY, float camZ, float camDepth, float width, float height, float trackWidth)
        {
            camera.x = world.x - camX;
            camera.y = world.y - camY;
            camera.z = world.z - camZ;
           
            screen.x = width / 2u + screen.scale * camera.x  * width / 2u;
            screen.y = height / 2u - screen.scale * camera.y  * height / 2u;
            screen.w = screen.scale * trackWidth * width / 2u;
            screen.scale = camDepth / camera.z;
        }
    };

    //TODO batch these
    class Polygon final : public sf::Drawable
    {
    public:
        Polygon() : m_vertices(4) {}
        ~Polygon() = default;
        Polygon(const Polygon&) = delete;
        Polygon& operator = (const Polygon&) = delete;

        //when batching prefer a list of positions for arbitrary sizes
        void setVerts(const sf::Vector2f& p0, const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Color& colour = sf::Color::White)
        {
            m_vertices[0].position = p0;
            m_vertices[1].position = p1;
            m_vertices[2].position = p2;
            m_vertices[3].position = p3;

            m_vertices[0].color = m_vertices[1].color = m_vertices[2].color = m_vertices[3].color = colour;

            //TODO set tex coords
        }
    private:
        std::vector<sf::Vertex> m_vertices;
        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
        {
            rt.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::Quads);
        }
    };

    class Palette final
    {
    public:
        enum Category
        {
            Light, Dark,
            Start, Finish,
            Count
        };

        explicit Palette(Category cat = Category::Light)
            : m_category    (cat),
            m_data          (Category::Count)
        {
            m_data[Palette::Light].track = { 160, 160, 160 };
            m_data[Palette::Light].lane = { 204, 204, 204 };
            m_data[Palette::Light].rumble = sf::Color::White;
            m_data[Palette::Light].grass = { 160, 230, 160 };
            
            m_data[Palette::Dark].track = { 160, 160, 160 };
            m_data[Palette::Dark].lane = { 160, 160, 160 };
            m_data[Palette::Dark].rumble = { 227,107, 107 };
            m_data[Palette::Dark].grass = { 100, 194, 100 };

            m_data[Palette::Start].track = sf::Color::White;
            m_data[Palette::Start].lane = sf::Color::White;
            m_data[Palette::Start].rumble = sf::Color::White;
            m_data[Palette::Start].grass = { 16, 170, 16 };

            m_data[Palette::Finish].track = {};
            m_data[Palette::Finish].lane = {};
            m_data[Palette::Finish].rumble = {};
            m_data[Palette::Finish].grass = { 16, 170, 16 };           
        }
        void setCategory(Category cat)
        {
            m_category = cat;
        }
        sf::Color getTrack() const
        {
            return m_data[m_category].track;
        }

        sf::Color getLane() const
        {
            return m_data[m_category].lane;
        }

        sf::Color getRumble() const
        {
            return m_data[m_category].rumble;
        }

        sf::Color getGrass() const
        {
            return m_data[m_category].grass;
        }

    private:
        struct Data
        {
            sf::Color track;
            sf::Color lane;
            sf::Color rumble;
            sf::Color grass;
        };
        std::vector<Data> m_data;
        Category m_category;
    };

    class Segment final : public sf::Drawable
    {
    public:
        using Ptr = std::unique_ptr<Segment>;

        Segment();
        ~Segment() = default;
        Segment(const Segment&) = delete;
        Segment& operator = (const Segment&) = delete;

        Point& pointA();
        Point& pointB();
        const Point& pointA() const;
        const Point& pointB() const;

        void setPalette(Palette::Category);

        void setCurve(float);
        float getCurve() const;

        void setIndex(std::size_t);
        std::size_t getIndex() const;

        void setClip(float);
        float getClip() const;

        void updateVerts(float width, float fog);

    private:
        Point m_pointA;
        Point m_pointB;
        Polygon m_rumbleA;
        Polygon m_rumbleB;
        Polygon m_laneA;
        Polygon m_laneB;
        Polygon m_mainLane;
        Polygon m_landscape;
        Polygon m_fog;
        float m_curve;
        std::size_t m_index;
        float m_clip;
        Palette m_palette;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    float m_distance;
    float m_trackLength;
    std::vector<Segment::Ptr> m_segments;

    void addTrackSegment(float curve, float y);
    void addTrackSection(float enter, float hold, float exit, float curve, float y);
    float prevY();
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
};

#endif //RACING_DEMO_TRACK_HPP_
