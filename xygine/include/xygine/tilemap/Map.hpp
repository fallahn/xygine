/*********************************************************************
© Matt Marchant 2014 - 2017
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
#include <xygine/tilemap/Property.hpp>

#include <xygine/physics/RigidBody.hpp>
#include <xygine/physics/CollisionShape.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>

#include <string>
#include <vector>

namespace xy
{
    class TileMapLayer;
    class ShaderResource;
    class TextureResource;
    namespace tmx
    {
        struct XY_EXPORT_API Version
        {
            //major/minor are apparently reserved by gcc            
            sf::Uint16 upper;
            sf::Uint16 lower;
            Version(sf::Uint16 maj = 0, sf::Uint16 min = 0)
             : upper(maj), lower(min) {}
        };
        
        enum class Orientation
        {
            Orthogonal,
            Isometric,
            Staggered,
            Hexagonal,
            None
        };

        enum class RenderOrder
        {
            RightDown,
            RightUp,
            LeftDown,
            LeftUp,
            None
        };

        enum class StaggerAxis
        {
            X, Y, None
        };

        enum class StaggerIndex
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

        The class also provides direct read-only access to map data so that 
        it may be used for any custom interpretation.
        */
        class XY_EXPORT_API Map final
        {
        public:
            friend class xy::TileMapLayer;
            
            Map();
            ~Map() = default;
            Map(const Map&) = delete;
            Map& operator  = (const Map&) = delete;

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
            \brief Returns the bounds of the map in SFML units
            */
            sf::FloatRect getBounds() const { return sf::FloatRect(0.f, 0.f, static_cast<float>(m_tileCount.x * m_tileSize.x), static_cast<float>(m_tileCount.y * m_tileSize.y)); }
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
            /*!
            \brief Returns a reference to the vector of tile sets used by the map
            */
            const std::vector<Tileset>& getTilesets() const { return m_tilesets; }
            /*!
            \brief Returns a reference to the vector containing the layer data.
            Layers are pointer-to-baseclass, the concrete type of which can be
            found via Layer::getType()
            \see Layer
            */
            const std::vector<Layer::Ptr>& getLayers() const { return m_layers; }
            /*!
            \brief Returns a vector of Property objects loaded by the map
            */
            const std::vector<Property>& getProperties() const { return m_properties; }

            /*!
            \brief Creates a drawable component from the given layer which can
            be attached to a Scene Entity.
            Only TileLayer and ImageLayer types are valid, trying to create a 
            drawable component from an object layer will return a nullptr. Passing
            a reference to a layer which doesn't belong to this map will also cause
            a nullptr to be returned.
            \returns Drawable component if successful, else nullptr
            */
            std::unique_ptr<TileMapLayer> getDrawable(xy::MessageBus&, const Layer&, TextureResource&, ShaderResource&);

            /*!
            \brief Attempts to create a RigidBody component from a given Layer.
            This function will attempt to parse all objects in a given ObjectGroup
            and attach them to a single RigidBody component. This is useful for map
            layers designed to contain, for example, solid collision objects.
            If the given layer is not an ObjectGroup then this function returns a nullptr.
            */
            std::unique_ptr<Physics::RigidBody> createRigidBody(xy::MessageBus&, const Layer&, Physics::BodyType = Physics::BodyType::Static);

            /*!
            \brief Creates a RigidBody component from a given ObjectGroup.
            This function will attempt to parse all objects in a given ObjectGroup
            and attach them to a single RigidBody component. This is useful for map
            layers designed to contain, for example, solid collision objects.
            If the given ObjectGroup is invalid then this function returns a nullptr.
            */
            std::unique_ptr<Physics::RigidBody> createRigidBody(xy::MessageBus&, const ObjectGroup&, Physics::BodyType = Physics::BodyType::Static);

            /*!
            \brief Creates a RigidBody component from a given object.
            This function attempts to create a RigidBody component from a given object,
            useful for dynamic items which each require their own physics body. If the
            function fails then it will return nullptr.
            */
            std::unique_ptr<Physics::RigidBody> createRigidBody(xy::MessageBus&, const Object&, Physics::BodyType = Physics::BodyType::Dynamic);

            /*!
            \brief Attempts to create a CollisionShape from a given object.
            Collision shapes created from the given object can then be attached to
            arbitrary RigidBody components, and so is useful for creating template
            type shapes from an object on a map, for example when dynamically spawning
            new Entities at run time.
            */
            std::unique_ptr<Physics::CollisionShape> createCollisionShape(const Object&);

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
            std::vector<Layer::Ptr> m_layers;
            std::vector<Property> m_properties;

            //always returns false so we can return this
            //on load failure
            bool reset();

            void processObject(Physics::RigidBody*, const Object&);
            using PointList = std::vector<sf::Vector2f>;
            void subDivide(Physics::RigidBody*, const PointList&, bool);
            std::vector<PointList> splitConvex(const PointList&);
            std::vector<PointList> splitConcave(const PointList&);

            struct Key {};
        };
    }
}
#endif //XY_TMX_MAP_HPP_