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

#include <xygine/Log.hpp>
#include <xygine/Assert.hpp>
#include <xygine/parsers/pugixml.hpp>
#include <xygine/FileSystem.hpp>

using namespace xy;
using namespace xy::tmx;


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
        LOG("Failed opening " + path, Logger::Type::Error);
        LOG("Reason: " + std::string(result.description()), Logger::Type::Error);
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
        LOG("Failed opening map: " + path + ", no map data found", Logger::Type::Error);
        return reset();
    }

    //parse map attributes
    std::size_t pointPos = 0;
    std::string attribString = mapNode.attribute("version").as_string();
    if (attribString.empty() || (pointPos = attribString.find('.')) == std::string::npos)
    {
        LOG("Invalid map version value, map not loaded.", Logger::Type::Error);
        return reset();
    }

    m_version.major = std::stoi(attribString.substr(0, pointPos));
    m_version.minor = std::stoi(attribString.substr(pointPos + 1));

    attribString = mapNode.attribute("orientation").as_string();
    if (attribString.empty())
    {
        LOG("Missing map orientation attribute, map not loaded.", Logger::Type::Error);
        return reset();
    }

    if (attribString == "orthogonal")
    {
        m_orientation = Orientation::Orthogonal;
    }
    else
    {
        LOG(attribString + " format maps aren't supported yet, sorry! Map not laoded", Logger::Type::Error);
        return reset();
    }

    attribString = mapNode.attribute("renderorder").as_string();
    if (attribString.empty())
    {
        LOG("missing render-order attribute, map not loaded.", Logger::Type::Error);
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
        LOG(attribString + ": invalid render order. Map not loaded.", Logger::Type::Error);
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
        LOG("Invalid map tile count, map not loaded", Logger::Type::Error);
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
        LOG("Invalid tile size, map not loaded", Logger::Type::Error);
        return reset();
    }

    //TODO check for stagger properties when hex and staggered maps are implemented

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
        //TODO we need to track the order of these so that they are drawn correctly
        std::string name = node.name();
        if (name == "tileset")
        {
            m_tilesets.emplace_back(m_workingDirectory);
            m_tilesets.back().parse(node, m_textureResource);
        }
        else if (name == "layer")
        {
            m_layers.emplace_back();
            m_layers.back().parse(node);
        }
        else if (name == "objectgroup")
        {
            m_objectGroups.emplace_back();
            m_objectGroups.back().parse(node);
        }
        else if (name == "imagelayer")
        {
            m_imageLayers.emplace_back();
            m_imageLayers.back().parse(node);
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
    m_objectGroups.clear();
    m_imageLayers.clear();
    m_properties.clear();

    return false;
}