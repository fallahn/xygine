/*********************************************************************
Matt Marchant 2014 - 2015
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

//resource manager

#ifndef XY_RESOURCES_HPP_
#define XY_RESOURCES_HPP_

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <memory>
#include <array>

namespace xy
{
    template <class T>
    class BaseResource
    {
    public:
        BaseResource()
        {
        }
        virtual ~BaseResource() = default;
        BaseResource(const BaseResource&) = delete;
        const BaseResource& operator = (const BaseResource&) = delete;

        T& get(const std::string& path = "default")
        {
            //if we have a valid path check current resources and return if found
            if (!path.empty())
            {
                auto r = m_resources.find(path);
                if (r != m_resources.end())
                {
                    return *r->second;
                }
            }
            //else attempt to load from file
            std::unique_ptr<T> r = std::unique_ptr<T>(new T());
            if (path.empty() || !r->loadFromFile(path))
            {
                m_resources[path] = errorHandle();
            }
            else
            {
                m_resources[path] = std::move(r);
            }

            return *m_resources[path];
        }
    protected:
        virtual std::unique_ptr<T> errorHandle() = 0;
    private:
        std::map<std::string, std::unique_ptr<T>> m_resources;
    };

    class TextureResource final : public BaseResource<sf::Texture>
    {
    private:
        std::unique_ptr<sf::Texture> errorHandle() override
        {
            std::unique_ptr<sf::Texture> t(new sf::Texture());
            sf::Image i;
            i.create(20u, 20u, sf::Color(127u, 127u, 255u));
            t->loadFromImage(i);
            return std::move(t);
        }
    };
    class ImageResource final : public BaseResource<sf::Image>
    {
        std::unique_ptr<sf::Image> errorHandle() override
        {
            std::unique_ptr<sf::Image> i(new sf::Image);
            i->create(20u, 20u, sf::Color::Green);
            return std::move(i);
        }
    };

    class FontResource final : public BaseResource<sf::Font>
    {
    public:
        FontResource();
    private:
        sf::Font m_font;
        std::unique_ptr<sf::Font> errorHandle() override;
    };

    class SoundResource final : public BaseResource<sf::SoundBuffer>
    {
        std::unique_ptr<sf::SoundBuffer> errorHandle() override
        {
            std::array<sf::Int16, 20u> buffer;
            std::memset(buffer.data(), 0, buffer.size());
            auto sb = std::make_unique<sf::SoundBuffer>();
            sb->loadFromSamples(buffer.data(), buffer.size(), 1, 48000);
            return std::move(sb);
        }
    };
}
#endif //XY_RESOURCES_HPP_