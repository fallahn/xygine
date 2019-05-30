/*********************************************************************
(c) Matt Marchant 2017 - 2019
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/Config.hpp"
#include "xyginext/resources/ResourceHandler.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace sf
{
    class Texture;
}

namespace xy
{
    /*!
    \brief Sprite component optimised to work with the ECS.
    Sprite components require their entity to also have a Drawable component
    and a Transform component.
    */
    class XY_EXPORT_API Sprite final
    {
    public:
        /*!
        \brief Default constructor
        */
        Sprite();

        /*!
        \brief Construct a sprite with a texture
        */
        explicit Sprite(const sf::Texture&);
        //explicit Sprite(const xy::ResourceHandle&);

        /*!
        \brief Sets the texture with which to draw this sprite.
        By default sprites are resized to the size of this texture
        */
        void setTexture(const sf::Texture&);
        //void setTexture(const xy::ResourceHandle&);

        /*!
        \brief Sets a sub rectangle of the current texture
        to be used when drawing.
        When a new texture is set on the sprite this is reset to
        the size of the texture,
        */
        void setTextureRect(sf::FloatRect);

        /*!
        \brief Sets the colour to be multiplied with this
        sprite when drawing.
        */
        void setColour(sf::Color);

        /*!
        \brief Returns a pointer to the sprite's active texture
        */
        const sf::Texture* getTexture() const;

        /*!
        \brief Returns the current sub-rectangle used to crop the sprite
        */
        sf::FloatRect getTextureRect() const { return m_textureRect; }

        /*!
        \brief Returns the current sprite colour
        */
        sf::Color getColour() const;

        /*!
        \brief Returns the bounds created by the current texture rect
        */
        sf::FloatRect getTextureBounds() const { return { {}, getSize() }; }

        /*!
        \brief Returns the size of the sprite based on the current texture rect
        */
        sf::Vector2f getSize() const { return { m_textureRect.width, m_textureRect.height }; }

        /*!
        \brief Maximum number of frames in an animation
        */
        static constexpr std::size_t MaxFrames = 64;

        /*!
        \brief Maximum number of animations per sprite
        */
        static constexpr std::size_t MaxAnimations = 32;

        /*!
        \brief Represents a single animation
        */
        struct Animation final
        {
            /*!
            \brief Maximum  length of animation id
            */
            static constexpr std::size_t MaxAnimationIdLength = 32;
            
            std::array<char,MaxAnimationIdLength> id = {{0}};
            std::array<sf::FloatRect, MaxFrames> frames;
            std::size_t frameCount = 0;
            std::uint32_t loopStart = 0; //!< looped animations can jump to somewhere other than the beginning
            bool looped = false;
            float framerate = 12.f;
        };


        /*!
        \brief Returns the number of animations for this sprite when loaded
        from a sprite sheet definition file.
        */
        std::size_t getAnimationCount() const { return m_animationCount; }

        /*!
        /brief Returns a reference to the sprites animation array.
        Use getAnimationCount() to check how many of the animations are valid
        */
        std::array<Animation, MaxAnimations>& getAnimations() { return m_animations; }

        /*!
        /brief Returns a const reference to the sprites animation array.
        Use getAnimationCount() to check how many of the animations are valid
        */
        const std::array<Animation, MaxAnimations>& getAnimations() const { return m_animations; }

    private:

        sf::FloatRect m_textureRect;
        const sf::Texture* m_texture;
        ResourceHandle  m_textureHandle;
        sf::Color m_colour;
        sf::BlendMode m_blendMode;
        bool m_blendOverride;
        bool m_dirty;

        std::size_t m_animationCount;
        std::array<Animation, MaxAnimations> m_animations;

        friend class SpriteSystem;
        friend class SpriteSheet;
        friend class SpriteAnimator;
    };
}
