/*********************************************************************
(c) Jonny Paton 2018
(c) Matt Marchant 2019

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

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include "xyginext/resources/ResourceHandler.hpp"
#include "xyginext/resources/SystemFont.hpp"
#include "xyginext/graphics/BitmapFont.hpp"

#include <cstring>

using namespace xy;
ResourceHandler::ResourceHandler()
{
    // Add a texture loader
    ResourceLoader texLoader;
    texLoader.loader = [](const std::string& path)
    {
        //a bit convoluted but prevents a copy operation
        //which can throw occasional opengl errors
        auto tex = std::make_any<sf::Texture>();
        auto& tr = std::any_cast<sf::Texture&>(tex);
		
		if (tr.loadFromFile(xy::FileSystem::getResourcePath() + path))
		{
			return tex;
		}
		return std::any();
    };

    texLoader.fallback = []()
    {
		sf::Image img;
		img.create(16, 16, sf::Color::Magenta);

		auto tex = std::make_any<sf::Texture>();
		auto& tr = std::any_cast<sf::Texture&>(tex);
		tr.loadFromImage(img);

        return tex;
    };

    // And a font loader
    ResourceLoader fontLoader;
    fontLoader.loader = [](const std::string& path)
    {
        sf::Font font;
        return font.loadFromFile(xy::FileSystem::getResourcePath() + path) ? font : std::any();
    };

    fontLoader.fallback = []()
    {
        sf::Font font;
        static const std::array<std::string, 4u> fontNames =
        {
            "arial.ttf",
            "Arial.ttf",
            "tahoma.ttf",
            "VeraMono.ttf"
        };

        std::string fullPath = getFontPath();
        for (const auto& fn : fontNames)
        {
            if (font.loadFromFile(fullPath + fn))
            {

                xy::Logger::log("Loaded default font " + fullPath + fn, xy::Logger::Type::Info);
                break;
            }
            else
            {
                xy::Logger::log("Failed to load fallback font at " + fullPath + fn, xy::Logger::Type::Warning);
            }
        }
        return font;
    };

    //sound buffer loader
    ResourceLoader soundLoader;
    soundLoader.loader = [](const std::string& path)
    {
        sf::SoundBuffer buffer;
        return buffer.loadFromFile(xy::FileSystem::getResourcePath() + path) ? buffer : std::any();
    };

    soundLoader.fallback = []()
    {
        std::array<sf::Int16, 20u> buffer;
        std::memset(buffer.data(), 0, buffer.size());
        
        sf::SoundBuffer sb;
        sb.loadFromSamples(buffer.data(), buffer.size(), 1, 48000);
        return sb;
    };

    //bitmap font loader
    ResourceLoader bmfLoader;
    bmfLoader.loader = [](const std::string& path)
    {
        //a bit convoluted but prevents a copy operation
        //which can throw occasional opengl errors
        auto bmf = std::make_any<xy::BitmapFont>();
        auto& br = std::any_cast<xy::BitmapFont&>(bmf);

        if (br.loadTextureFromFile(xy::FileSystem::getResourcePath() + path))
        {
            return bmf;
        }
        return std::any();
    };

    bmfLoader.fallback = []() {return std::any(); };

    getLoader<sf::Texture>() = texLoader;
    getLoader<sf::Font>() = fontLoader;
    getLoader<sf::SoundBuffer>() = soundLoader;
    getLoader<xy::BitmapFont>() = bmfLoader;
}
