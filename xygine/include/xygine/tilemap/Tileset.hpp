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

#ifndef XY_TMX_TILESET_HPP_
#define XY_TMX_TILESET_HPP_

#include <xygine/Config.hpp>
#include <xygine/tilemap/Property.hpp>
#include <xygine/tilemap/ObjectGroup.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <vector>
#include <array>

namespace pugi
{
    class xml_node;
}

namespace xy
{
    class TextureResource;

    namespace tmx
    {
        /*!
        \brief Represents a Tileset node as loaded
        from a *.tmx format tile map via the tmx::Map
        class.
        */
        class XY_EXPORT_API Tileset final
        {
        public:
            explicit Tileset(const std::string& workingDir);
            ~Tileset() = default;

            /*!
            \brief Any tiles within a tile set which have special
            data associated with them such as animation or terrain
            information will have one of these stored in the tile set.
            */
            struct Tile final
            {
                sf::Uint32 ID = 0;
                std::array<std::int32_t, 4u> terrainIndices;
                sf::Uint32 probability = 100;

                struct Animation final
                {
                    struct Frame final
                    {
                        sf::Uint32 tileID = 0;
                        sf::Uint32 duration = 0;
                    };
                    std::vector<Frame> frames;
                }animation;
                std::vector<Property> properties;
                ObjectGroup objectGroup;
                std::string imagePath;
            };
            
            /*!
            \brief Terrain information with which one
            or more tiles may be associated.
            */
            struct Terrain final
            {
                std::string name;
                sf::Int32 tileID = -1;
                std::vector<Property> properties;
            };

            /*!
            \brief Attempts to parse the given xml node.
            If node parsing fails an error is printed in the console
            and the Tileset remains in an uninitialised state.
            */
            void parse(pugi::xml_node);
            /*!
            \brief Returns the first GID of this tile set.
            This the ID of the first tile in the tile set, so that
            each tile set guarentees a unique set of IDs
            */
            sf::Uint32 getFirstGID() const { return m_firstGID; }
            /*!
            \brief Returns the name of this tile set.
            */
            const std::string& getName() const { return m_name; }
            /*!
            \brief Returns the width and height of a tile in the
            tile set, in pixels.
            */
            const sf::Vector2u& getTileSize() const { return m_tileSize; }
            /*!
            \brief Returns the spacing, in pixels, between each tile in the set
            */
            sf::Uint32 getSpacing() const { return m_spacing; }
            /*!
            \brief Returns the margin, in pixels, around each tile in the set
            */
            sf::Uint32 getMargin() const { return m_margin; }
            /*!
            \brief Returns the number of tiles in the tile set
            */
            sf::Uint32 getTileCount() const { return m_tileCount; }
            /*!
            \brief Returns the number of columns which make up the tile set.
            This is used when rendering collection of images sets
            */
            sf::Uint32 getColumnCount() const { return m_columnCount; }
            /*!
            \brief Returns the tile offset in pixels.
            Tile will draw tiles offset from the top left using this value.
            */
            const sf::Vector2u& getTileOffset() const { return m_tileOffset; }
            /*!
            \brief Returns a reference to the list of Property objects for this
            tile set
            */
            const std::vector<Property>& getProperties() const { return m_properties; }
            /*!
            \brief Returns the file path to the tile set image, relative to the
            working directory. Use this to load the texture required by whichever
            method you choose to render the map.
            */
            const std::string getImagePath() const { return m_imagePath; }
            /*!
            \brief Returns the colour used by the tile map image to represent transparency.
            By default this is a transparent colour (0, 0, 0, 0)
            */
            const sf::Color getTransparencyColour() const { return m_transparencyColour; }
            /*!
            \brief Returns a vector of Terrain types associated with one
            or more tiles within this tile set
            */
            const std::vector<Terrain>& getTerrainTypes() const { return m_terrainTypes; }
            /*!
            \brief Returns a reference to the vector of tile data used by
            tiles which make up this tile set, if it is a collection of images.
            */
            const std::vector<Tile>& getTiles() const { return m_tiles; }

        private:

            std::string m_workingDir;

            sf::Uint32 m_firstGID;
            std::string m_source;
            std::string m_name;
            sf::Vector2u m_tileSize;
            sf::Uint32 m_spacing;
            sf::Uint32 m_margin;
            sf::Uint32 m_tileCount;
            sf::Uint32 m_columnCount;
            sf::Vector2u m_tileOffset;

            std::vector<Property> m_properties;
            std::string m_imagePath;
            sf::Color m_transparencyColour;

            std::vector<Terrain> m_terrainTypes;
            std::vector<Tile> m_tiles;

            void reset();

            void parseOffsetNode(const pugi::xml_node&);
            void parsePropertyNode(const pugi::xml_node&);
            void parseTerrainNode(const pugi::xml_node&);
            void parseTileNode(const pugi::xml_node&);
        };
    }
}

#endif //XY_TMX_TILESET_HPP_