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

#include <xygine/tilemap/Map.hpp>
#include <xygine/tilemap/FreeFuncs.hpp>
#include <xygine/tilemap/ObjectGroup.hpp>
#include <xygine/tilemap/ImageLayer.hpp>
#include <xygine/tilemap/TileLayer.hpp>

#include <xygine/components/TileMapLayer.hpp>

#include <xygine/Assert.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/Resource.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/util/Vector.hpp>

#include <xygine/physics/CollisionCircleShape.hpp>
#include <xygine/physics/CollisionEdgeShape.hpp>
#include <xygine/physics/CollisionPolygonShape.hpp>
#include <xygine/physics/CollisionRectangleShape.hpp>
#include <xygine/physics/RigidBody.hpp>

#include <queue>

using namespace xy;
using namespace xy::tmx;

namespace
{
    float cross(const sf::Vector2f& lv, const sf::Vector2f& rv)
    {
        return lv.x * rv.y - lv.y * rv.x;
    }
    
    float cross(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c)
    {
        sf::Vector2f BA = a - b;
        sf::Vector2f BC = c - b;
        return cross(BA, BC);
    }
    const float pointTolerance = 0.1f;
    
    bool concave(const std::vector<sf::Vector2f>& points)
    {
        bool negative = false;
        bool positive = false;

        std::size_t a, b, c, n = points.size();
        for (a = 0u; a < n; ++a)
        {
            b = (a + 1) % n;
            c = (b + 1) % n;

            float cr = cross(points[a], points[b], points[c]);

            if (cr < 0.f)
            {
                negative = true;
            }
            else if (cr > 0.f)
            {
                positive = true;
            }
            if (positive && negative)
            {
                return true;
            }
        }
        return false;
    }

    void enforceWinding(std::vector<sf::Vector2f>& points)
    {
        float result = 0.f;
        for (auto i = 0u; i < points.size() - 1u; ++i)
        {
            result += ((points[i + 1].x - points[i].x) * (points[i + 1].y - points[i].y));
        }
        //reverse winding if counter clockwise
        if (result > 0)
        {
            std::reverse(points.begin(), points.end());
        }
    }
}


Map::Map()
    : m_orientation (Orientation::None),
    m_renderOrder   (RenderOrder::None),
    m_hexSideLength (0.f),
    m_staggerAxis   (StaggerAxis::None),
    m_staggerIndex  (StaggerIndex::None)
{

}

//public
bool Map::load(const std::string& path)
{
    reset();

    //open the doc
    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());
    if (!result)
    {
        Logger::log("Failed opening " + path, Logger::Type::Error);
        Logger::log("Reason: " + std::string(result.description()), Logger::Type::Error);
        return false;
    }

    m_workingDirectory = FileSystem::getFilePath(path);
    std::replace(m_workingDirectory.begin(), m_workingDirectory.end(), '\\', '/');
    if (m_workingDirectory.find_last_of('/') == m_workingDirectory.size() - 1)
    {
        m_workingDirectory.pop_back();
    }

    //find the map node and bail if it doesn't exist
    auto mapNode = doc.child("map");
    if(!mapNode)
    {
        Logger::log("Failed opening map: " + path + ", no map data found", Logger::Type::Error);
        return reset();
    }

    //parse map attributes
    std::size_t pointPos = 0;
    std::string attribString = mapNode.attribute("version").as_string();
    if (attribString.empty() || (pointPos = attribString.find('.')) == std::string::npos)
    {
        Logger::log("Invalid map version value, map not loaded.", Logger::Type::Error);
        return reset();
    }

    m_version.upper = std::stoi(attribString.substr(0, pointPos));
    m_version.lower = std::stoi(attribString.substr(pointPos + 1));

    attribString = mapNode.attribute("orientation").as_string();
    if (attribString.empty())
    {
        Logger::log("Missing map orientation attribute, map not loaded.", Logger::Type::Error);
        return reset();
    }

    if (attribString == "orthogonal")
    {
        m_orientation = Orientation::Orthogonal;
    }
    else if (attribString == "isometric")
    {
        m_orientation = Orientation::Isometric;
    }
    else if (attribString == "staggered")
    {
        m_orientation = Orientation::Staggered;
    }
    else if (attribString == "hexagonal")
    {
        m_orientation = Orientation::Hexagonal;
    }
    else
    {
        Logger::log(attribString + " format maps aren't supported yet, sorry! Map not loaded", Logger::Type::Error);
        return reset();
    }

    attribString = mapNode.attribute("renderorder").as_string();
    if (attribString.empty())
    {
        Logger::log("missing render-order attribute, map not loaded.", Logger::Type::Error);
        return reset();
    }

    if (attribString == "right-down")
    {
        m_renderOrder = RenderOrder::RightDown;
    }
    else if (attribString == "right-up")
    {
        m_renderOrder = RenderOrder::RightUp;
    }
    else if (attribString == "left-down")
    {
        m_renderOrder = RenderOrder::LeftDown;
    }
    else if (attribString == "left-up")
    {
        m_renderOrder = RenderOrder::LeftUp;
    }
    else
    {
        Logger::log(attribString + ": invalid render order. Map not loaded.", Logger::Type::Error);
        return reset();
    }

    unsigned width = mapNode.attribute("width").as_int();
    unsigned height = mapNode.attribute("height").as_int();
    if (width && height)
    {
        m_tileCount = { width, height };
    }
    else
    {
        Logger::log("Invalid map tile count, map not loaded", Logger::Type::Error);
        return reset();
    }

    width = mapNode.attribute("tilewidth").as_int();
    height = mapNode.attribute("tileheight").as_int();
    if (width && height)
    {
        m_tileSize = { width, height };
    }
    else
    {
        Logger::log("Invalid tile size, map not loaded", Logger::Type::Error);
        return reset();
    }

    m_hexSideLength = mapNode.attribute("hexsidelength").as_float();
    if (m_orientation == Orientation::Hexagonal && m_hexSideLength <= 0)
    {
        Logger::log("Invalid he side length found, map not loaded", Logger::Type::Error);
        return reset();
    }

    attribString = mapNode.attribute("staggeraxis").as_string();
    if (attribString == "x")
    {
        m_staggerAxis = StaggerAxis::X;
    }
    else if (attribString == "y")
    {
        m_staggerAxis = StaggerAxis::Y;
    }
    if ((m_orientation == Orientation::Staggered || m_orientation == Orientation::Hexagonal)
        && m_staggerAxis == StaggerAxis::None)
    {
        Logger::log("Map missing stagger axis property. Map not loaded.", Logger::Type::Error);
        return reset();
    }

    attribString = mapNode.attribute("staggerindex").as_string();
    if (attribString == "odd")
    {
        m_staggerIndex = StaggerIndex::Odd;
    }
    else if (attribString == "even")
    {
        m_staggerIndex = StaggerIndex::Even;
    }
    if ((m_orientation == Orientation::Staggered || m_orientation == Orientation::Hexagonal)
        && m_staggerIndex == StaggerIndex::None)
    {
        Logger::log("Map missing stagger index property. Map not loaded.", Logger::Type::Error);
        return reset();
    }


    //colour property is optional
    attribString = mapNode.attribute("backgroundcolor").as_string();
    if (!attribString.empty())
    {
        m_backgroundColour = colourFromString(attribString);
    }

    //TODO do we need next object ID? technically we won't be creating
    //new objects outside of the scene in xygine.

    //parse all child nodes
    for (const auto& node : mapNode.children())
    {
        std::string name = node.name();
        if (name == "tileset")
        {
            m_tilesets.emplace_back(m_workingDirectory);
            m_tilesets.back().parse(node);
        }
        else if (name == "layer")
        {
            m_layers.emplace_back(std::make_unique<TileLayer>(m_tileCount.x * m_tileCount.y));
            m_layers.back()->parse(node);
        }
        else if (name == "objectgroup")
        {
            m_layers.emplace_back(std::make_unique<ObjectGroup>());
            m_layers.back()->parse(node);
        }
        else if (name == "imagelayer")
        {
            m_layers.emplace_back(std::make_unique<ImageLayer>(m_workingDirectory));
            m_layers.back()->parse(node);
        }
        else if (name == "properties")
        {
            const auto& children = node.children();
            for (const auto& child : children)
            {
                m_properties.emplace_back();
                m_properties.back().parse(child);
            }
        }
        else
        {
            LOG("Unidentified name " + name + ": node skipped", Logger::Type::Warning);
        }
    }

    return true;
}

std::unique_ptr<TileMapLayer> Map::getDrawable(xy::MessageBus& mb, const Layer& layer, TextureResource&, ShaderResource&)
{
    if (layer.getType() == Layer::Type::Object)
    {
        Logger::log("Layer is not a drawawble type", Logger::Type::Error);
        return nullptr;
    }
    //TODO check layer belongs to this map


    return nullptr;
}

std::unique_ptr<Physics::RigidBody> Map::createRigidBody(xy::MessageBus& mb, const ObjectGroup& og, Physics::BodyType bodyType)
{
    auto rb = xy::Component::create<xy::Physics::RigidBody>(mb, bodyType);
    for (const auto& o : og.getObjects())
    {
        processObject(rb.get(), o);
    }
    
    return std::move(rb);
}

std::unique_ptr<Physics::RigidBody> Map::createRigidBody(xy::MessageBus& mb, const Object& object, Physics::BodyType bodyType)
{
    auto rb = xy::Component::create<xy::Physics::RigidBody>(mb, bodyType);    
    processObject(rb.get(), object);
    
    return std::move(rb);
}

std::unique_ptr<Physics::CollisionShape> Map::createCollisionShape(const Object& object)
{   
    switch (object.getShape())
    {
    default: return nullptr;
    case Object::Shape::Ellipse:
    {
        auto size = object.getAABB();
        if (size.width == size.height)
        {
            //we have a circle
            const float radius = size.width / 2.f;

            std::unique_ptr<Physics::CollisionShape> cs =
                std::make_unique<Physics::CollisionCircleShape>(radius);
            dynamic_cast<Physics::CollisionCircleShape*>(cs.get())->setPosition({size.left + radius, size.top + radius});
            return std::move(cs);
        }
        else
        {
            //elliptical
            const float x = size.width / 2.f;
            const float y = size.height / 2.f;
            const float tau = 6.283185f;
            const float step = tau / 16.f; //number of points to make up ellipse
            std::vector<sf::Vector2f> points;
            for (float angle = 0.f; angle < tau; angle += step)
            {
                points.emplace_back((x + x * std::cos(angle)), (y + y * std::sin(angle)));               
            }

            sf::Transform tx;
            tx.rotate(object.getRotation());
            for (auto i = 0u; i < points.size(); ++i)
            {
                points[i] = tx.transformPoint(points[i]);
                points[i].x += size.left;
                points[i].x += size.top;
            }

            std::unique_ptr<Physics::CollisionShape> cs =
                std::make_unique<Physics::CollisionEdgeShape>(points, Physics::CollisionEdgeShape::Option::Loop);
            return std::move(cs);
        }
    }
    case Object::Shape::Polygon:
    {
        const auto& points = object.getPoints();
        if (points.size() > Physics::CollisionPolygonShape::maxPoints())
        {
            Logger::log("Polygon " + object.getName() + " has more than maximum allowable points (8), consider breaking into smaller parts.", Logger::Type::Warning);
            Logger::log("Polygon " + object.getName() + " will be returned as an edge shape", Logger::Type::Info);
            goto polyline;
        }
        else if (concave(points))
        {
            Logger::log("Polygon " + object.getName() + " is concave, consider breaking into smaller parts.", Logger::Type::Warning);
            Logger::log("Polygon " + object.getName() + " will be returned as an edge shape", Logger::Type::Info);
            goto polyline;
        }

        sf::Transform tx;
        tx.rotate(object.getRotation());
        std::vector<sf::Vector2f> worldPoints;
        for (const auto& p : points)
        {
            auto point = tx.transformPoint(p);
            worldPoints.push_back(point + object.getPosition());
        }
        enforceWinding(worldPoints);
        std::unique_ptr<Physics::CollisionShape> cs =
            std::make_unique<Physics::CollisionPolygonShape>(worldPoints);
        return std::move(cs);
    }
        break;
    case Object::Shape::Polyline:
        polyline:
    {
        const auto& points = object.getPoints();
        sf::Transform tx;
        tx.rotate(object.getRotation());
        std::vector<sf::Vector2f> worldPoints;
        for (const auto& p : points)
        {
            auto point = tx.transformPoint(p);
            worldPoints.push_back(point + object.getPosition());
        }

        std::unique_ptr<Physics::CollisionShape> cs =
            std::make_unique<Physics::CollisionEdgeShape>(worldPoints);
        return std::move(cs);
    }
    case Object::Shape::Rectangle:
    {
        float rotation = object.getRotation();
        if (rotation == 0)
        {
            std::unique_ptr<Physics::CollisionShape> cs =
                std::make_unique<Physics::CollisionRectangleShape>(sf::Vector2f(object.getAABB().width, object.getAABB().height), sf::Vector2f(object.getPosition()));
            return std::move(cs);
        }
        else
        {
            //create a rotated polygon instead
            auto size = object.getAABB();
            std::vector<sf::Vector2f> points =
            {
                sf::Vector2f(),
                sf::Vector2f(size.width, 0.f),
                sf::Vector2f(size.width, size.height),
                sf::Vector2f(0.f, size.height)
            };

            sf::Transform tx;
            tx.rotate(rotation);
            for (auto& p : points)
            {
                p = tx.transformPoint(p);
                p += {size.left, size.top};
            }

            std::unique_ptr<Physics::CollisionShape> cs =
                std::make_unique<Physics::CollisionPolygonShape>(points);
            return std::move(cs);
        }
    }
    }
    
    return nullptr;
}

//private
bool Map::reset()
{
    m_orientation = Orientation::None;
    m_renderOrder = RenderOrder::None;
    m_tileCount = { 0u, 0u };
    m_tileSize = { 0u, 0u };
    m_hexSideLength = 0.f;
    m_staggerAxis = StaggerAxis::None;
    m_staggerIndex = StaggerIndex::None;
    m_backgroundColour = sf::Color::Black;
    m_workingDirectory = "";

    m_tilesets.clear();
    m_layers.clear();
    m_properties.clear();

    return false;
}

void Map::processObject(Physics::RigidBody* rb, const Object& object)
{
    //check if polygon and sub-divide if necessary
    //if (object.getShape() == Object::Shape::Polygon)
    //{
        const auto& points = object.getPoints();
        PointList pointlist;
        if (!points.empty())
        {
            sf::Transform tx;
            tx.rotate(object.getRotation());            
            for (const auto& p : points)
            {
                auto point = tx.transformPoint(p);
                pointlist.push_back(point + object.getPosition());
            }
            enforceWinding(pointlist);
        }

        bool conc = false;
        if (object.getShape() == Object::Shape::Polygon &&
            (object.getPoints().size() > Physics::CollisionPolygonShape::maxPoints() || (conc = concave(object.getPoints()))))
        {
            //subdivide
            LOG("subdividing polygon " + object.getName(), Logger::Type::Info);
            subDivide(rb, pointlist, conc);
        }
    //}
    else
    {
        auto cs = createCollisionShape(object);
        XY_ASSERT(cs, "failed creating collision shape");
        switch (cs->type())
        {
        default: break;
        case Physics::CollisionShape::Type::Box:
            rb->addCollisionShape(*dynamic_cast<Physics::CollisionRectangleShape*>(cs.get()));
            break;
        case Physics::CollisionShape::Type::Circle:
            rb->addCollisionShape(*dynamic_cast<Physics::CollisionCircleShape*>(cs.get()));
            break;
        case Physics::CollisionShape::Type::Edge:
            rb->addCollisionShape(*dynamic_cast<Physics::CollisionEdgeShape*>(cs.get()));
            break;
        case Physics::CollisionShape::Type::Polygon:
            rb->addCollisionShape(*dynamic_cast<Physics::CollisionPolygonShape*>(cs.get()));
            break;
        }
    }
}

void Map::subDivide(Physics::RigidBody* rb, const PointList& points, bool conc)
{
    std::vector<PointList> pointLists;
    if (conc)
    {
        pointLists = splitConcave(points);
    }
    else
    {
        pointLists = splitConvex(points);
    }

    for (const auto& list : pointLists)
    {
        Physics::CollisionPolygonShape cs(list);
        rb->addCollisionShape(cs);
    }
}

std::vector<Map::PointList> Map::splitConvex(const PointList& points)
{
    XY_ASSERT(points.size() > Physics::CollisionPolygonShape::maxPoints(), "shouldn't be trying to split this");
    std::vector<PointList> retVal;

    const std::size_t maxVerts = Physics::CollisionPolygonShape::maxPoints() - 2u;
    const std::size_t shapeCount = static_cast<std::size_t>(std::floor(points.size() / maxVerts));
    const std::size_t size = points.size();

    for (auto i = 0u; i <= shapeCount; ++i)
    {
        retVal.emplace_back();
        PointList& newPoints = retVal.back();
        for (auto j = 0u; j < maxVerts; ++j)
        {
            auto index = i * maxVerts + j;
            if (i) index--;
            if (index < size)
            {
                newPoints.push_back(points[index]);
            }
            else
            {
                break;
            }
        }
    }

    //close loop
    const PointList& endList = retVal[retVal.size() - 2];
    retVal.back().push_back(retVal.front().front());
    retVal.back().push_back(endList.back());

    return retVal;
}

std::vector<Map::PointList> Map::splitConcave(const PointList& points)
{
    std::function<float(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&)> getWinding = 
        [](const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3) ->float
    {
        return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.y * p2.x - p2.y * p3.x - p3.y * p1.x;
    };
    
    std::function<sf::Vector2f(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&)> hitsPoint = 
        [](const sf::Vector2f& p1, const sf::Vector2f& p2, const sf::Vector2f& p3, const sf::Vector2f& p4)->sf::Vector2f
    {
        sf::Vector2f g1 = p3 - p1;
        sf::Vector2f g2 = p2 - p1;
        sf::Vector2f g3 = p4 - p3;

        float t = cross(g3, g2);
        float a = cross(g3, g1) / t;

        return sf::Vector2f(p1.x + a * g2.x, p1.y + a * g2.y);
    };

    std::function<bool(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&)> onLine =
        [](const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end)->bool
    {
        if (end.x - start.x > pointTolerance || start.x - end.x > pointTolerance)
        {
            float a = (end.y - start.y) / (end.x - start.x);
            float newY = a * (p.x - start.x) + start.y;
            float diff = std::abs(newY - p.y);
            return (diff < pointTolerance);
        }

        return (p.x - start.x < pointTolerance || start.x - p.x < pointTolerance);
    };

    std::function<bool(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&)> onSeg = 
        [&onLine](const sf::Vector2f& p, const sf::Vector2f& start, const sf::Vector2f& end)->bool
    {
        bool a = (start.x + pointTolerance >= p.x && p.x >= end.x - pointTolerance)
            || (start.x - pointTolerance <= p.x && p.x <= end.x + pointTolerance);
        bool b = (start.y + pointTolerance >= p.y && p.y >= end.y - pointTolerance)
            || (start.y - pointTolerance <= p.y && p.y <= end.y + pointTolerance);
        return ((a && b) && onLine(p, start, end));
    };

    std::function<bool(const sf::Vector2f&, const sf::Vector2f&)> pointsMatch = 
        [](const sf::Vector2f& p1, const sf::Vector2f& p2)->bool
    {
        float x = std::abs(p2.x - p1.x);
        float y = std::abs(p2.y - p1.y);
        return (x < pointTolerance && y < pointTolerance);
    };

    //--------------------------------------------------------------------------------

    std::queue<PointList> queue;
    queue.push(points);

    std::vector<PointList> retVal;

    while (!queue.empty())
    {
        const auto& list = queue.front();
        std::size_t size = list.size();
        bool convex = true;

        for (auto i = 0u; i < size; ++i)
        {
            std::size_t i1 = i;
            std::size_t i2 = (i < size - 1u) ? i + 1u : i + 1u - size;
            std::size_t i3 = (i < size - 2u) ? i + 2u : i + 2u - size;

            sf::Vector2f p1 = list[i1];
            sf::Vector2f p2 = list[i2];
            sf::Vector2f p3 = list[i3];

            float direction = getWinding(p1, p2, p3);
            if (direction < 0.f)
            {
                convex = false;
                float minLen = FLT_MAX;
                int j1 = 0;
                int j2 = 0;
                sf::Vector2f v1, v2;
                int h = 0;
                int k = 0;
                sf::Vector2f hitPoint;

                for (auto j = 0u; j < size; ++j)
                {
                    if (j != i1 && j != i2)
                    {
                        j1 = j;
                        j2 = (j < size - 1u) ? j + 1u : 0;

                        v1 = list[j1];
                        v2 = list[j2];

                        sf::Vector2f hp = hitsPoint(p1, p2, v1, v2);
                        bool b1 = onSeg(p2, p1, hp);
                        bool b2 = onSeg(hp, v1, v2);

                        if (b1 && b2)
                        {
                            sf::Vector2f dist = p2 - hp;
                            float t = Util::Vector::lengthSquared(dist);

                            if (t < minLen)
                            {
                                h = j1;
                                k = j2;
                                hitPoint = hp;
                                minLen = t;
                            }
                        }
                    }
                }

                PointList list1;
                PointList list2;

                j1 = h;
                j2 = k;
                v1 = list[j1];
                v2 = list[j2];

                if (!pointsMatch(hitPoint, v2))
                {
                    list1.push_back(hitPoint);
                }
                if (!pointsMatch(hitPoint, v1))
                {
                    list2.push_back(hitPoint);
                }

                h = -1;
                k = i1;
                while (true)
                {
                    if (k != j2)
                    {
                        list1.push_back(list[k]);
                    }
                    else
                    {
                        XY_ASSERT(h >= 0 && h < size, "");
                        if (!onSeg(v2, list[h], p1))
                        {
                            list1.push_back(list[k]);
                        }
                        break;
                    }

                    h = k;
                    if ((k - 1) < 0)
                    {
                        k = size - 1u;
                    }
                    else
                    {
                        k--;
                    }
                }

                std::reverse(list1.begin(), list1.end());

                h = -1;
                k = i2;
                while (true)
                {
                    if (k != j1)
                    {
                        list2.push_back(list[k]);
                    }
                    else
                    {
                        XY_ASSERT(h >= 0 && h < size, "");
                        if ((k == j1) && !onSeg(v1, list[h], p2))
                        {
                           list2.push_back(list[k]);
                        }
                        break;
                    }

                    h = k;
                    if (k + 1 > size - 1u)
                    {
                        k = 0;
                    }
                    else
                    {
                        k++;
                    }
                }
                queue.push(list1);
                queue.push(list2);
                queue.pop();

                break;
            }
        }

        if (convex)
        {
            retVal.push_back(queue.front());
            queue.pop();
        }
    }

    return retVal;
}