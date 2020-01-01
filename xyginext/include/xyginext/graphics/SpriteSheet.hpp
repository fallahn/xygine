/*********************************************************************
(c) Matt Marchant 2017 - 2020
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
#include "xyginext/ecs/components/Sprite.hpp"

#include <unordered_map>
#include <string>

namespace xy
{
    class TextureResource;
    class ResourceHandler;

    /*!
    \brief Supports loading multiple sprites from a single
    texture atlas via the ConfigFile format.
    */
    class XY_EXPORT_API SpriteSheet final
    {
    public:
        SpriteSheet();

        /*!
        \brief Attempts to load a ConfigFile from the given path.
        A reference to a valid texture resource is required to load
        the sprite sheet texture.
        \returns true if successful, else false
        */
        bool loadFromFile(const std::string& path, TextureResource& rx);
        bool loadFromFile(const std::string& path, ResourceHandler& rx);

        /*!
        \brief Attempts to save a ConfigFile at the given path.
        \returns true if successful, else false
        */
        bool saveToFile(const std::string& path);

        /*!
        \brief Returns a sprite component with the given name as it
        appears in the sprite sheet. If the sprite does not exist an
        empty sprite is returned.
        */
        Sprite getSprite(const std::string& name) const;
        
        /*!
        \brief Set the data of a sprite in the spritesheet
         
        \param name The name of the sprite to set
        \param data The sprite component data to assign
         
        \warning if sprite already exists it will be overwritten
        */
        void setSprite(const std::string& name, const Sprite& data);
        
        /*!
        \brief Remove a sprite from the sheet
        */
        void removeSprite(const std::string& name);
        
        /*!
        \brief Returns all sprites in this spritesheet
        */
        const std::unordered_map<std::string, Sprite>& getSprites() const;

        /*!
        \brief Returns the index of the animation with the given name
        on the given sprite if it exists, else returns 0
        */
        std::size_t getAnimationIndex(const std::string& name, const std::string& sprite) const;
        
        /*!
        \brief Returns the path of the texture used by this spritesheet
        */
        const std::string& getTexturePath() const;
        
        /*!
        \brief Set the path of the texture used by this spritesheet
        */
        void setTexturePath(const std::string& path);

    private:
        mutable std::unordered_map<std::string, Sprite> m_sprites;
        mutable std::unordered_map<std::string, std::vector<std::string>> m_animations;

        std::string m_texturePath;
        bool m_smooth;

        bool loadFromFile(const std::string& path, std::function<sf::Texture*(const std::string&)>&);
    };
}
