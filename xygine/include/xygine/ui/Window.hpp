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

//window drawable which may house a UI Container

#ifndef XY_UI_WINDOW_HPP_
#define XY_UI_WINDOW_HPP_

#include <xygine/ui/Container.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <memory>

namespace sf
{
    class Event;
    class RenderWindow;
}

namespace xy
{
    namespace UI
    {
        /*!
        \brief Creates a window-like control

        Windows are not strictly UI Controls, but have their own Container
        to which Controls can be added. Unlike containers windows can be moved
        and resized by the user

        WARNING this is extremely experimental and its use is not recommended
        If windowing is required currently the best choice is to use the imGUI controls
        found within the nim namespace
        */
        class XY_EXPORT_API Window final : public sf::Drawable, public sf::Transformable
        {
        public:
            using Ptr = std::unique_ptr<Window>;
            /*!
            \brief A Collection of colours used to create a window palette
            */
            struct Palette
            {
                sf::Color borderNormal; //! Colour of the window border in its normal state
                sf::Color borderActive; //! Colour of the window border in its active state
                sf::Color background; //! Colour of the window background
                sf::Color font; //! Colour of the window's font
                Palette()
                : borderNormal  (120u, 120u, 120u),
                borderActive    (190u, 190u, 190u),
                background      (120u, 120u, 120u, 190u){}
            };
            /*!
            \brief Constructor
            \param sf::RenderWindow The current render window
            \param MessageBus Reference to the active message bus
            \param sf::Font Font to use with window text
            \param width Width of the window
            \param height Height of the window
            \param palette Instance of the Palette struct which defines the window colours
            */
            Window(sf::RenderWindow&, xy::MessageBus&, const sf::Font&, sf::Uint16 width, sf::Uint16 height, const Palette& = Palette());
            ~Window() = default;
            Window(const Window&) = delete;
            Window& operator = (const Window&) = delete;
            /*!
            \brief Forwards the frame time to contained controls
            */
            void update(float);
            /*!
            \brief Forwards events for contained controls
            */
            void handleEvent(const sf::Event&, const sf::Vector2f&);
            /*!
            \brief Set the palette of colours to be used by the window
            */
            void setPalette(const Palette&);
            /*!
            \brief Set the title string of the window
            */
            void setTitle(const std::string&);
            /*!
            \brief Add a control to the window's control collection
            */
            void addControl(Control::Ptr);
            /*!
            \brief Set if the widow can be resized by dragging the corner.
            True by default
            */
            void canResize(bool);
            /*!
            \brief Returns true if the window is resizable
            */
            bool canResize() const;
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
            bool m_resizable;

            sf::Shader m_cropShader;
            sf::RenderWindow& m_renderWindow;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}
#endif //XY_UI_WINDOW_HPP_