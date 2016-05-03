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

//resource manager

#ifndef XY_RESOURCES_HPP_
#define XY_RESOURCES_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <memory>
#include <array>
#include <unordered_map>
#include <cstring>

namespace xy
{
    /*!
    \brief Abstract base class for resource management

    The templated base class provides resource management for
    types given by derived classes. Resource management makes sure
    only a single instance of each resource, such as a texture or
    shader is loaded at a time.
    */
    template <class T>
    class XY_EXPORT_API BaseResource
    {
    public:
        BaseResource()
        {
        }
        virtual ~BaseResource() = default;
        BaseResource(const BaseResource&) = delete;
        const BaseResource& operator = (const BaseResource&) = delete;

        /*!
        \brief Attempts to load a resource from disk at the given path

        If the resource is not found or fails to load the resource manager
        supplies a fallback resource, as defined by the concrete implementation
        of the resource manager.
        \see errorHandle
        */
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
            std::unique_ptr<T> r = std::make_unique<T>();
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
        /*!
        \brief Function for handling resource acquisition errors

        Concrete resource manager types must implement this to provide 
        a fallback instance of the resource type, should loading of the
        requested resource fail for some reason.
        */
        virtual std::unique_ptr<T> errorHandle() = 0;
    private:
        std::unordered_map<std::string, std::unique_ptr<T>> m_resources;
    };

    /*!
    \brief Resource manager for textures
    */
    class XY_EXPORT_API TextureResource final : public BaseResource<sf::Texture>
    {
    public:
        void setFallbackColour(const sf::Color colour) { m_fallbackColour = colour; }
    private:
        /*!
        \see BaseResource
        */
        std::unique_ptr<sf::Texture> errorHandle() override
        {
            std::unique_ptr<sf::Texture> t = std::make_unique<sf::Texture>();
            sf::Image i;
            i.create(20u, 20u, m_fallbackColour);
            t->loadFromImage(i);
            return std::move(t);
        }

        sf::Color m_fallbackColour;
    };
    /*!
    \brief Resource manager for sf::Image types
    */
    class XY_EXPORT_API ImageResource final : public BaseResource<sf::Image>
    {
        /*!
        \see BaseResource
        */
        std::unique_ptr<sf::Image> errorHandle() override
        {
            std::unique_ptr<sf::Image> i = std::make_unique<sf::Image>();
            i->create(20u, 20u, sf::Color::Green);
            return std::move(i);
        }
    };

    /*!
    \brief Resource manager for Fonts
    */
    class XY_EXPORT_API FontResource final : public BaseResource<sf::Font>
    {
    public:
        FontResource();
    private:
        sf::Font m_font;
        std::unique_ptr<sf::Font> errorHandle() override;
    };
    /*!
    \brief Resource manager for sound files
    */
    class XY_EXPORT_API SoundResource final : public BaseResource<sf::SoundBuffer>
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