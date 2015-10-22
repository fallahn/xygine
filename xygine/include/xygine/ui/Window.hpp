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

//window drawable which may house a UI Container

#ifndef UI_WINDOW_HPP_
#define UI_WINDOW_HPP_

#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>

namespace sf
{
    class Event;
}

namespace xy
{
    namespace ui
    {
        class Window final : public sf::Drawable, public sf::Transformable
        {
        public:
            using Ptr = std::unique_ptr<Window>;
            struct Palette
            {
                sf::Color borderNormal;
                sf::Color borderActive;
                sf::Color background;
            };

            Window(const sf::Font&, sf::Uint16 width, sf::Uint16 height, const Palette& = Palette());
            ~Window() = default;
            Window(const Window&) = delete;
            Window& operator = (const Window&) = delete;

            void update(float);
            void handleEvent(const sf::Event&, const sf::Vector2f&);
            void setPalette(const Palette&);
            void setTitle(const std::string&);

            void addControl(Control::Ptr);

        private:
            class BackgroundShape final : public sf::Shape
            {
            public:
                explicit BackgroundShape(const sf::Vector2f& size);
                ~BackgroundShape() = default;
                BackgroundShape(const BackgroundShape&) = delete;
                BackgroundShape& operator = (const BackgroundShape&) = delete;

                void setSize(const sf::Vector2f&);
                void setMinimumSize(const sf::Vector2f&);
                const sf::Vector2f& getSize() const;
                std::size_t getPointCount() const override;
                sf::Vector2f getPoint(std::size_t) const override;

            private:
                sf::Vector2f m_size;
                sf::Vector2f m_minimumSize;

                void clampSize();
            }m_backgroundShape;

            sf::RectangleShape m_titleBar;
            sf::Text m_titleText;
            sf::CircleShape m_resizeHandle;
            Palette m_palette;
            sf::Vector2f m_lastMousePos;

            Container m_container;

            enum Drag
            {
                All = 0x1,
                Right = 0x2,
                Bottom = 0x4,
                Corner = 0x8
            };
            sf::Int16 m_dragMask;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}
#endif //UI_WINDOW_HPP_