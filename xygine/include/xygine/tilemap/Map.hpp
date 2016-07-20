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

#ifndef XY_TMX_MAP_HPP_
#define XY_TMX_MAP_HPP_

#include <xygine/tilemap/Tileset.hpp>
#include <xygine/tilemap/Layer.hpp>
#include <xygine/tilemap/ObjectGroup.hpp>
#include <xygine/tilemap/ImageLayer.hpp>
#include <xygine/tilemap/Property.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>

#include <string>
#include <vector>

namespace xy
{
    namespace tmx
    {
        struct XY_EXPORT_API Version
        {
            sf::Uint16 major;
            sf::Uint16 minor;
            Version(sf::Uint16 maj = 0, sf::Uint16 min = 0)
             : major(maj), minor(min) {}
        };
        
        enum class XY_EXPORT_API Orientation
        {
            Orthogonal,
            Isometric,
            Staggered,
            Hexagonal,
            None
        };

        enum class XY_EXPORT_API RenderOrder
        {
            RightDown,
            RightUp,
            LeftDown,
            LeftUp,
            None
        };

        enum class XY_EXPORT_API StaggerAxis
        {
            X, Y, None
        };

        enum class XY_EXPORT_API StaggerIndex
        {
            Even, Odd, None
        };

        /*!
        \brief Parser for TMX format tile maps.
        This class can be used to parse the XML format tile maps created
        with the Tiled map editor, providing an interface to create xygine
        compatible Drawable and Physics components, which can be used in
        a scene graph. Typical usage would be to create an instance of this
        class before calling load() providing a path to the *.tmx file to
        load. Then layers or objects can be requested from the Map class
        to be returned as xygine components. Once the components are added
        to a Scene they can be interacted with in the same way as any other
        xygine Entity/Component, providing a flexible interface with tile
        maps.
        */
        class XY_EXPORT_API Map final
        {
        public:
            Map();
            ~Map() = default;

            /*!
            \brief Attempts to parse the tilemap at the given location.
            \param std::string Path to map file to try to parse
            \returns true if map was parsed successful else returns false.
            In debug mode this will attempt to log any errors to the console.
            */
            bool load(const std::string&);

            /*!
            \brief Returns the version of the tile map last parsed.
            If no tile map has yet been parsed the version will read 0, 0
            */
            const Version& getVersion() const { return m_version; }
            /*!
            \brief Returns the orientation of the map if one is loaded,
            else returns None
            */
            Orientation getOrientation() const { return m_orientation; }
            /*!
            \brief Returns the RenderOrder of the map if one is loaded,
            else returns None
            */
            RenderOrder getRenderOrder() const { return m_renderOrder; }
            /*!
            \brief Returns the tile count of the map in the X and Y directions
            */
            const sf::Vector2u& getTileCount() const { return m_tileCount; }
            /*!
            \brief Returns the size of the tile grid in this map.
            Actual tile sizes may vary and will be exteded / shrunk about
            the bottom left corner of the tile.
            */
            const sf::Vector2u& getTileSize() const { return m_tileSize; }
            /*!
            \brief Returns the length of an edge of a tile if a Hexagonal
            map is loaded.
            The length returned is in pixels of the straight edge running
            along the axis returned by getStaggerAxis(). If no map is loaded
            or the loaded map is not of Hexagonal orientation this function
            returns 0.f
            */
            float getHexSideLength() const { return m_hexSideLength; }
            /*!
            \brief Stagger axis of the map.
            If either a Staggered or Hexagonal tile map is loaded this returns
            which axis the map is staggered along, else returns None.
            */
            StaggerAxis getStaggerAxis() const { return m_staggerAxis; }
            /*!
            \brief Stagger Index of the loaded map.
            If a Staggered or Hexagonal map is loaded this returns whether
            the even or odd rows of tiles are staggered, otherwise it returns None.
            */
            StaggerIndex getStaggerIndex() const { return m_staggerIndex; }
            /*!
            \brief Returns the background colour of the map.
            */
            const sf::Color& getBackgroundColour() const { return m_backgroundColour; }

        private:
            Version m_version;
            Orientation m_orientation;
            RenderOrder m_renderOrder;

            sf::Vector2u m_tileCount;
            sf::Vector2u m_tileSize;

            float m_hexSideLength;
            StaggerAxis m_staggerAxis;
            StaggerIndex m_staggerIndex;

            sf::Color m_backgroundColour;

            std::string m_workingDirectory;

            std::vector<Tileset> m_tilesets;
            std::vector<Layer> m_layers;
            std::vector<ObjectGroup> m_objectGroups;
            std::vector<ImageLayer> m_imageLayers;
            std::vector<Property> m_properties;

            //always returns false so we can return this
            //on load failure
            bool reset();
        };
    }
}
#endif //XY_TMX_MAP_HPP_