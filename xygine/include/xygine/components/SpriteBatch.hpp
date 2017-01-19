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

#ifndef XY_SPRITE_BATCH_HPP_
#define XY_SPRITE_BATCH_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <array>

namespace xy
{
    class Sprite;
    /*!
    \brief SpriteBatch component.
    The sprite batch component allow batches of multiple sprites which
    share the same texture or atlas to be drawn in a single call. For
    a single batch multiple Sprites can be created and attached to
    entities in a scene. Only the batch itself is drawable, so the batch
    must have its own entity. Multiple batches using different textures
    can exist in a single scene, so potentially hundreds of sprites can
    be drawn with only a few draw calls.
    */
    class XY_EXPORT_API SpriteBatch final : public sf::Drawable, public xy::Component
    {
    public:
        /*!
        \brief Constructor
        */
        explicit SpriteBatch(xy::MessageBus&);
        ~SpriteBatch() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float);
        sf::FloatRect globalBounds() const override { return m_globalBounds; }

        /*!
        \brief Adds a new sprite to the batch.
        \returns A new sprite component.
        Using the addSprite function new sprites are created and
        added to this batch, which can then be attached to an existing entity.
        \see xy::Sprite
        */
        std::unique_ptr<Sprite> addSprite(xy::MessageBus&);
        /*!
        \brief Sets the texture used by this batch.
        All sprites drawn via this batch will use this texture.
        Usually a texture atlas would be used and a sub-rectangle of the
        atlas mapped to the sprite, so that many different sprites can
        belong to a single texture. If this texture is set to nullptr
        then all sprites belonging to this batch will use their default colour.
        */
        void setTexture(const sf::Texture*);

    private:
        
        const sf::Texture* m_texture;
        std::vector<sf::Vertex> m_vertices;
        std::vector<Sprite*> m_sprites;

        sf::FloatRect m_globalBounds;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    /*!
    \brief Batched Sprite.
    While these sprites are slightly less flexible than SFML sprites
    (they do not carry their own transform and are bound entirely to
    that of their parent entity's) xy::Sprites belong to a parent batch
    which allows efficient drawing of multiple sprites in a single call
    potentially offering a large performance boost. A sprite component
    on its own will not work - it must be created via the addSprite()
    function of a SpriteBatch component.
    */
    class XY_EXPORT_API Sprite final : public xy::Component
    {
    public:
        explicit Sprite(xy::MessageBus&);
        ~Sprite() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        /*!
        \brief Sets the sub-rectangle of the SpriteBatch texture this sprite should
        display.
        By default he rectangle covers the entire texture.
        */
        void setTextureRect(const sf::FloatRect&);
        /*!
        \brief Sets the Sprite's colour.
        When no texture is set on the parent SpriteBatch the sprite is drawn as a
        quad using this colour. If a texture is set then this colour is multiplied by
        the colour of the current texture.
        */
        void setColour(const sf::Color&);

    private:
        friend class SpriteBatch;
        sf::Transform m_transform;
        std::array<sf::Vertex, 4u> m_vertices;
    };
}

#endif //XY_SPRITE_BATCH_HPP_