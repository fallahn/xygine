#include "xyginext/resources/ResourceHandler.hpp"

using namespace xy;
ResourceHandler::ResourceHandler()
{
    // Add a texture loader
    ResourceLoader texLoader;
    texLoader.loader = [](const std::string& path)
    {
        sf::Texture tex;
        return tex.loadFromFile(path) ? tex : stdx::any();
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
        return font.loadFromFile(path) ? font : stdx::any();
    };

    fontLoader.fallback = []()
    {
        sf::Font font;
        font.loadFromMemory(DejaVuSans_ttf.data(), DejaVuSans_ttf.size());
        return font;
    };

    getLoader<sf::Texture>() = texLoader;
    getLoader<sf::Font>() = fontLoader;
}