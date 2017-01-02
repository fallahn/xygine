/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <xygine/spriter/Model.hpp>
#include <xygine/spriter/Document.hpp>
#include <xygine/spriter/DocumentAttribute.hpp>
#include <xygine/spriter/DocumentElement.hpp>
#include <xygine/spriter/Helpers.hpp>

#include <xygine/Resource.hpp>
#include <xygine/FileSystem.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/Sprite.hpp>

using namespace xy;
using namespace xy::Spriter;

Model::Model(TextureResource& tr)
    : m_textureResource(tr) {}


//public
bool Model::loadFromFile(const std::string& file)
{
    if (FileSystem::getFileExtension(file) == ".scml")
    {
        Detail::Document document;
        if (document.loadFromFile(file))
        {
            auto firstElement = document.firstElement("spriter_data");
            if (firstElement)
            {
                Detail::DirectoryLister dirLister;
                return 
                ( 
                    loadImages(firstElement, file, dirLister)
                    && loadTags(firstElement)
                    && loadEntities(firstElement, dirLister)
                );
            }
            else
            {
                LOG("Invalid or corrupt xml in: " + file, Logger::Type::Error);
                return false;
            }
        }
        else
        {
            LOG("Failed to open scml file: " + file, Logger::Type::Error);
            return false;
        }
    }
    LOG("Invalid file extension: must be *.scml", Logger::Type::Error);
    return false;
}

//private
bool Model::loadImages(Detail::DocumentElement& element, const std::string& file, Detail::DirectoryLister& dirLister)
{
    auto path = FileSystem::getFilePath(file);
    auto folderElement = element.firstElement("folder");
    std::vector<std::pair<sf::Texture*, sf::Vector2f>> textures;
    while (folderElement)
    {
        //LOG(folderElement.getName(), Logger::Type::Info);

        dirLister.addDirectory();
        auto fileElement = folderElement.firstElement("file");
        while (fileElement)
        {
            //LOG(fileElement.getName(), Logger::Type::Info);

            dirLister.addFile();

            auto attrib = fileElement.firstAttribute("name");
            std::string fileName;
            if (attrib)
            {
                fileName = attrib.valueAsString();
            }
            else
            {
                LOG("Missing file name attribute", Logger::Type::Error);
                return false;
            }

            //check for images and load / map to origin
            //REMEMBER pivot values are normalised
            //TODO might have to invert Y coord here for SFML
            sf::Vector2f origin;
            attrib = fileElement.firstAttribute("pivot_x");
            if (attrib) origin.x = attrib.valueAsFloat();

            attrib = fileElement.firstAttribute("pivot_y");
            if (attrib) origin.y = attrib.valueAsFloat();

            origin.y = 1.f - origin.y;

            sf::Texture* t = &m_textureResource.get(path + fileName);
            sf::Vector2f size(t->getSize());
            origin.x *= size.x;
            origin.y *= size.y;

            textures.emplace_back(std::make_pair(t, origin));

            fileElement.advanceNextSameName();
        }

        folderElement.advanceNextSameName();
    }

    //pack textures into an atlas
    packTextures(textures);

    return true;
}

namespace
{
    const sf::Uint16 textureWidth = 1024u;
}

void Model::packTextures(const std::vector<std::pair<sf::Texture*, sf::Vector2f>>& textures)
{
    for (auto i = 0u; i < textures.size(); ++i)
    {
        m_imageData.emplace_back(i, sf::FloatRect(sf::Vector2f(), sf::Vector2f(textures[i].first->getSize())), textures[i].second);
    }

    //sort by height. rotating would optimise space but cause more work down the line for little gain
    std::sort(m_imageData.begin(), m_imageData.end(), [](const ImgData& imgDataA, const ImgData& imgDataB)
    {
        return imgDataA.textureRect.height > imgDataB.textureRect.height;
    });

    //do a prepass to find out how tall the texture atlas needs to be
    float currentRowOffset = 0.f;
    float nextRowOffset = m_imageData.front().textureRect.height;

    float currentColOffset = 0.f;
    float nextColOffset = m_imageData.front().textureRect.width;

    sf::Uint16 textureHeight = 0u;

    for (auto& id : m_imageData)
    {
        id.textureRect.left = currentColOffset;
        id.textureRect.top = currentRowOffset;
        if (id.textureRect.left + id.textureRect.width < textureWidth)
        {
            //we fit in this row
            currentColOffset = nextColOffset;
            nextColOffset += id.textureRect.width;
        }
        else
        {
            //start a new row
            currentColOffset = 0.f;
            nextColOffset = id.textureRect.width;
            currentRowOffset = nextRowOffset;
            nextRowOffset += id.textureRect.height;

            id.textureRect.left = currentColOffset;
            id.textureRect.top = currentRowOffset;
            textureHeight = static_cast<sf::Uint16>(nextRowOffset);
        }
    }

    //round height to nearest 16 px
    textureHeight += textureHeight % 16;

    //finally render all images to atlas
    m_texture.create(textureWidth, textureHeight);
    m_texture.clear(sf::Color::Transparent);
    sf::Sprite tempSprite;

    for (const auto& id : m_imageData)
    {
        tempSprite.setTexture(*textures[id.directoryIndex].first);
        tempSprite.setPosition(id.textureRect.left, id.textureRect.top);
        m_texture.draw(tempSprite);
    }

    m_texture.display();
}

bool Model::loadTags(Detail::DocumentElement& element)
{
    auto taglistElement = element.firstElement("tag_list");
    if (taglistElement)
    {
        auto tagElement = taglistElement.firstElement();
        while (tagElement)
        {
            auto attrib = tagElement.firstAttribute("name");
            if (attrib)
            {
                m_tags.emplace_back(attrib.valueAsString());
            }
            else
            {
                //TODO do we really want to quit out just because
                //one or more names are missing? Suppose we don't
                //want to load corrupted files I guess...
                return false;
            }
            tagElement.advanceNextSameName();
        }
        taglistElement.advanceNextSameName();
    }

    return true;
}

bool Model::loadEntities(const Detail::DocumentElement& element, Detail::DirectoryLister& lister)
{
    auto entityElement = element.firstElement("entity");
    while (entityElement)
    {
        

        entityElement.advanceNextSameName();
    }
    return false;
}