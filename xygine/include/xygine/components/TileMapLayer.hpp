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

#ifndef XY_TILEMAP_LAYER_HPP_
#define XY_TILEMAP_LAYER_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/tilemap/Map.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <functional>

namespace xy
{
    namespace tmx
    {
        class ImageLayer;
        class TileLayer;
    }

    class TextureResource;
    class ShaderResource;

    /*
    \brief Drawable tile map layer.
    This component represents a single layer loaded via xy::tmx::Map.
    As the map class itself is not drawable, these components can be
    requested from the map object, passing in the layer which this
    component should represent. The component can be used to draw either
    TileLayer or ImageLayer type layers loaded by the map.
    \see tmx::Map::getDrawable()
    */
    class TileMapLayer final : public xy::Component, public sf::Drawable
    {
    public:

        TileMapLayer(xy::MessageBus&, const tmx::Map::Key&, const sf::Vector2u&);
        ~TileMapLayer() = default;

        xy::Component::Type type() const override { return Component::Type::Drawable; }
        void entityUpdate(Entity&, float) override;
        sf::FloatRect globalBounds() const override { return m_globalBounds; }

        void setTileData(const tmx::TileLayer*, const std::vector<tmx::Tileset>&, const tmx::Map&, TextureResource&, ShaderResource&);
        void setImageData(const tmx::ImageLayer*, const tmx::Map&, TextureResource&);

    private:

        struct TileData final
        {
            std::unique_ptr<sf::Texture> lookupTexture;
            sf::Texture* tileTexture = nullptr;
            sf::Vector2u tileCount;
            sf::Vector2f tileScale; //this tile set's scale relative to map
        };
        
        struct Chunk final : public sf::Drawable
        {
            std::vector<TileData> tileData;
            sf::FloatRect bounds;
            std::array<sf::Vertex, 4u> vertices;
        private:
            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };

        std::vector<Chunk> m_chunks;
        std::vector<const Chunk*> m_drawList;
        sf::Vector2u m_chunkResolution;

        float m_opacity;
        sf::FloatRect m_globalBounds;
        sf::Vector2u m_tileSize;
        sf::Shader* m_shader;
        
        sf::Sprite m_imageSprite;

        std::function<void(sf::RenderTarget&, sf::RenderStates)> m_renderFunc;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //XY_TILEMAP_LAYER_HPP_