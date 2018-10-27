#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>

#include "xyginext/resources/ResourceHandler.hpp"
#include "xyginext/resources/SystemFont.hpp"

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
        auto& tr = *std::any_cast<sf::Texture>(&tex);
        tr.loadFromFile(xy::FileSystem::getResourcePath() + path);
        return tex;
    };

    texLoader.fallback = []()
    {
        return sf::Texture();
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
        //font.loadFromMemory(DejaVuSans_ttf.data(), DejaVuSans_ttf.size());
        font.loadFromFile(getFontPath());
        return font;
    };

    getLoader<sf::Texture>() = texLoader;
    getLoader<sf::Font>() = fontLoader;
}
