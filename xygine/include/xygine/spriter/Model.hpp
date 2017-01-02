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

#ifndef XY_SPRITER_MODEL_HPP_
#define XY_SPRITER_MODEL_HPP_

#include <xygine/Config.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <string>
#include <vector>

namespace sf
{
    class Texture;
}

namespace xy
{
    class TextureResource;
    namespace Spriter
    {
        namespace Detail
        {
            class DocumentElement;
            class DirectoryLister;
        }

        /*!
        \brief Spriter Model.
        Spriter Models are used to parse scml files created with
        <a href="http://www.brashmonkey.com/spriter.htm">spriter</a>
        and load associated media such as textures.
        Models are analogous to Textures in SFML, where models are 
        related to Spriter::Actor in the same way sf::Texture is
        to sf::Sprite. that is, Models are resources which should be
        resource managed, and a single instance of each type created.
        Actors are the xygine components which reference a model and
        are used to interact with the scene. Multiple Actors can 
        reference a single Model.
        */
        class XY_EXPORT_API Model final
        {
        public:
            explicit Model(TextureResource&);
            ~Model() = default;
            Model(const Model&) = delete;
            const Model& operator = (const Model&) = delete;

            /*!
            \brief Attempts to load an scml file from disk and parse the contents
            \returns true if successful else false
            */
            bool loadFromFile(const std::string& file);
            //bool loadFromMemory();

            const sf::Texture& getTexture()const { return m_texture.getTexture(); }
        private:
            TextureResource& m_textureResource;

            std::vector<std::string> m_tags;

            struct ImgData final
            {
                ImgData(std::size_t idx, sf::FloatRect tr, sf::Vector2f o)
                    : directoryIndex(idx), textureRect(tr), origin(o) {}
                std::size_t directoryIndex;
                sf::FloatRect textureRect;
                sf::Vector2f origin;
            };
            sf::RenderTexture m_texture;
            std::vector<ImgData> m_imageData;

            bool loadImages(Spriter::Detail::DocumentElement&, const std::string&, Spriter::Detail::DirectoryLister&);
            void packTextures(const std::vector<std::pair<sf::Texture*, sf::Vector2f>>&);
            bool loadTags(Spriter::Detail::DocumentElement&);
            bool loadEntities(const Spriter::Detail::DocumentElement&, Spriter::Detail::DirectoryLister&);
        };
    }//ns Spriter
} //ns xy

#endif //XY_SPRITER_MODEL_HPP_
