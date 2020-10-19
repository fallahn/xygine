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

#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/graphics/SpriteSheet.hpp"
#include "xyginext/resources/Resource.hpp"
#include "xyginext/resources/ResourceHandler.hpp"

using namespace xy;

SpriteSheet::SpriteSheet()
    : m_smooth(false)
{

}

//public
bool SpriteSheet::loadFromFile(const std::string& path, TextureResource& textures)
{
    std::function<sf::Texture*(const std::string&)> getTexture = [&](const std::string& texPath)->sf::Texture*
    {
        return &textures.get(texPath);
    };

    return loadFromFile(path, getTexture);
}

bool SpriteSheet::loadFromFile(const std::string& path, ResourceHandler& textures)
{
    std::function<sf::Texture*(const std::string&)> getTexture =  [&](const std::string& texPath)->sf::Texture*
    {
        auto handle = textures.load<sf::Texture>(texPath);
        return &textures.get<sf::Texture>(handle); 
    };

    return loadFromFile(path, getTexture);
}

bool SpriteSheet::saveToFile(const std::string &path)
{
    ConfigFile sheetFile;
    sheetFile.addProperty("src", "\"" + m_texturePath + "\"");
    sheetFile.addProperty("smooth").setValue( m_smooth ? true : false);
    
    for (const auto& [name, sprite] : m_sprites)
    {
        auto sprObj = sheetFile.addObject("sprite", name);
        sprObj->addProperty("bounds").setValue(sprite.getTextureRect());
        sprObj->addProperty("colour").setValue(sprite.getColour());
        
        auto& anims = sprite.getAnimations();
        for (auto i(0u); i < sprite.getAnimations().size(); i++)
        {
            auto animObj = sprObj->addObject("animation", m_animations[name][i]);
            animObj->addProperty("framerate").setValue(anims[i].framerate);
            animObj->addProperty("loop").setValue(anims[i].looped);
            animObj->addProperty("loop_start").setValue(static_cast<std::int32_t>(anims[i].loopStart));
            
            auto& frames = anims[i].frames;
            for (auto j(0u); j < anims[i].frames.size(); j++)
            {
                animObj->addProperty("frame").setValue(frames[j]);
            }
        }
    }
    
    return sheetFile.save(path);
}

const std::string& SpriteSheet::getTexturePath() const
{
    return m_texturePath;
}

void SpriteSheet::setTexturePath(const std::string& path)
{
    m_texturePath = path;
}

Sprite SpriteSheet::getSprite(const std::string& name) const
{
    if (m_sprites.count(name) != 0)
    {
        return m_sprites[name];
    }

    LogW << name << " not found in sprite sheet" << std::endl;
    return {};
}

void SpriteSheet::setSprite(const std::string& name, const xy::Sprite& data)
{
    m_sprites[name] = data;
}

void SpriteSheet::removeSprite(const std::string& name)
{
    m_sprites.erase(name);
}

const std::unordered_map<std::string, Sprite>& SpriteSheet::getSprites() const
{
    return m_sprites;
}

std::size_t SpriteSheet::getAnimationIndex(const std::string& name, const std::string& spriteName) const
{
    if (m_animations.count(spriteName) != 0)
    {
        const auto& anims = m_animations[spriteName];
        const auto& result = std::find(anims.cbegin(), anims.cend(), name);
        if (result == anims.cend()) return 0;

        return std::distance(anims.cbegin(), result);
    }
    return 0;
}


//private
bool SpriteSheet::loadFromFile(const std::string& path, std::function<sf::Texture*(const std::string&)>& getTexture)
{
    ConfigFile sheetFile;
    if (!sheetFile.loadFromFile(xy::FileSystem::getResourcePath() + path))
    {
        return false;
    }

    m_sprites.clear();
    m_animations.clear();

    std::size_t count = 0;

    sf::Texture* texture = nullptr;
    sf::BlendMode blendMode = sf::BlendAlpha;

    //validate sprites, increase count
    if (auto* p = sheetFile.findProperty("src"))
    {
        m_texturePath = p->getValue<std::string>();
        texture = getTexture(m_texturePath);
    }
    else
    {
        LOG(sheetFile.getId() + " missing texture property", Logger::Type::Error);
        return false;
    }

    if (auto* p = sheetFile.findProperty("blendmode"))
    {
        std::string mode = p->getValue<std::string>();
        if (mode == "add") blendMode = sf::BlendAdd;
        else if (mode == "multiply") blendMode = sf::BlendMultiply;
        else if (mode == "none") blendMode = sf::BlendNone;
    }

    if (auto* p = sheetFile.findProperty("smooth"))
    {
        texture->setSmooth(p->getValue<bool>());
    }

    const auto& sheetObjs = sheetFile.getObjects();
    for (const auto& spr : sheetObjs)
    {
        if (spr.getName() == "sprite")
        {
            std::string spriteName = spr.getId();
            if (m_sprites.count(spriteName) > 0)
            {
                Logger::log(spriteName + " already exists in sprite sheet", Logger::Type::Error);
                continue;
            }

            Sprite spriteComponent;
            if (texture != nullptr)
            {
                spriteComponent.setTexture(*texture);
            }

            if (auto* p = spr.findProperty("blendmode"))
            {
                //override sheet mode
                std::string mode = p->getValue<std::string>();
                if (mode == "add") spriteComponent.m_blendMode = sf::BlendAdd;
                else if (mode == "multiply") spriteComponent.m_blendMode = sf::BlendMultiply;
                else if (mode == "none") spriteComponent.m_blendMode = sf::BlendNone;
                spriteComponent.m_blendOverride = true;
            }
            else
            {
                spriteComponent.m_blendMode = blendMode;
                spriteComponent.m_blendOverride = true;
            }

            if (auto* p = spr.findProperty("bounds"))
            {
                spriteComponent.setTextureRect(p->getValue<sf::FloatRect>());
            }

            if (auto* p = spr.findProperty("colour"))
            {
                spriteComponent.setColour(p->getValue<sf::Color>());
            }

            const auto& spriteObjs = spr.getObjects();
            for (const auto& sprOb : spriteObjs)
            {
                if (sprOb.getName() == "animation"
                    && spriteComponent.m_animations.size() < Sprite::MaxAnimations)
                {
                    auto& anim = spriteComponent.m_animations.emplace_back();

                    const auto& properties = sprOb.getProperties();
                    for (const auto& p : properties)
                    {
                        std::string name = p.getName();
                        if (name == "frame"
                            && anim.frames.size() < Sprite::MaxFrames)
                        {
                            anim.frames.emplace_back(p.getValue<sf::FloatRect>());
                        }
                        else if (name == "framerate")
                        {
                            anim.framerate = p.getValue<float>();
                        }
                        else if (name == "loop")
                        {
                            anim.looped = p.getValue<bool>();
                        }
                        else if (name == "loop_start")
                        {
                            anim.loopStart = p.getValue<std::int32_t>();
                        }
                    }

                    auto animId = sprOb.getId();
                    m_animations[spriteName].push_back(animId);
                    anim.id.resize(animId.length());
                    animId.copy(anim.id.data(), animId.length());
                }
            }

            m_sprites.insert(std::make_pair(spriteName, spriteComponent));
            count++;
        }
    }

    //LOG("Found " + std::to_string(count) + " sprites in " + path, Logger::Type::Info);
    return count > 0;
}