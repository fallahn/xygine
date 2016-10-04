#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>

#include <xygine/Config.hpp>

#include <string>
#include <vector>

namespace sf
{
    class Event;
    class RenderTarget;
    class RenderWindow;
    class Sprite;
    class Texture;
    class Window;
}

//#include "imgui.h"
namespace ImGui
{
    //XY_EXPORT_API bool Combo(const char* label, int* currentIndex, const std::vector<std::string>& items, int itemHeight = -1)
    //{
    //    return Combo(label, currentIndex,
    //        [](void* data, int idx, const char** out_text)
    //    {
    //        *out_text = (*(const std::vector<std::string>*)data)[idx].c_str();
    //        return true;
    //    }, (void*)&items, items.size(), itemHeight);
    //}

namespace SFML
{
    void Init(sf::Window& window, sf::RenderTarget& target);
    void Init(sf::RenderWindow& window); // for convenience
    bool ProcessEvent(const sf::Event& event);
    void Update(bool cursor);
    void Shutdown();

    void SetRenderTarget(sf::RenderTarget& target);
    void SetWindow(sf::Window& window);
}

// custom ImGui widgets for SFML stuff
XY_EXPORT_API void Image(const sf::Texture& texture);
XY_EXPORT_API void Image(const sf::Texture& texture, const sf::Vector2f& size);

XY_EXPORT_API void Image(const sf::Sprite& sprite);
XY_EXPORT_API void Image(const sf::Sprite& sprite, const sf::Vector2f& size);

XY_EXPORT_API bool ImageButton(const sf::Texture& texture, const int framePadding = -1,
                     const sf::Color& bgColor = sf::Color::Transparent,
                     const sf::Color& tintColor = sf::Color::White);
XY_EXPORT_API bool ImageButton(const sf::Texture& texture, const sf::Vector2f& size, const int framePadding = -1,
                     const sf::Color& bgColor = sf::Color::Transparent, const sf::Color& tintColor = sf::Color::White);

XY_EXPORT_API bool ImageButton(const sf::Sprite& sprite, const int framePadding = -1,
                     const sf::Color& bgColor = sf::Color::Transparent,
                     const sf::Color& tintColor = sf::Color::White);
XY_EXPORT_API bool ImageButton(const sf::Sprite& sprite, const sf::Vector2f& size, const int framePadding = -1,
                     const sf::Color& bgColor = sf::Color::Transparent,
                     const sf::Color& tintColor = sf::Color::White);
}
