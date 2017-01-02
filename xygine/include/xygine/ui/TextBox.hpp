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

//ui control for text input

#ifndef XY_UI_TEXTBOX_HPP_
#define XY_UI_TEXTBOX_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace xy
{
    namespace UI
    {
        /*!
        \brief A text input box
        */
        class XY_EXPORT_API TextBox final : public Control
        {
        public:
            using Ptr = std::shared_ptr<TextBox>;

            /*!
            \brief Constructor
            \param font Font to use with the displayed text
            \param background Background colour of the input box
            \param border Colour for the input box border and cursor icon
            */
            TextBox(const sf::Font& font, const sf::Color& background = sf::Color::Black, const sf::Color& border = sf::Color::White);
            ~TextBox() = default;
            /*!
            \see Control
            */
            bool selectable() const override;
            /*!
            \see Control
            */
            void select() override;
            /*!
            \see Control
            */
            void deselect() override;
            /*!
            \see Control
            */
            void activate() override;
            /*!
            \see Control
            */
            void deactivate() override;
            /*!
            \see Control
            */
            void handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) override;
            /*!
            \see Control
            */
            void update(float dt) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment a) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f& mousePos) const override;
            /*!
            \brief Get the contained text string
            */
            const std::string& getText() const;
            /*!
            \brief Set a texture  to be used in the background
            */
            void setTexture(const sf::Texture& text);
            /*!
            \brief Show or hide the border
            \param show True to show the border, false to hide it
            */
            void showBorder(bool show = true);
            /*!
            \brief Set the width and height of the textbox
            */
            void setSize(const sf::Vector2f& size);
            /*!
            \brief Set the string of the textbox text.
            */
            void setString(const std::string& text);
            /*!
            \brief Set the string for the textbox label text
            */
            void setLabelText(const std::string&);
            /*!
            \brief Set the maximum number of characters allowed
            in the textbox string
            */
            void setMaxLength(sf::Uint16);

        private:
            std::string m_string;
            sf::Text m_text;
            sf::Text m_label;
            sf::RectangleShape m_backShape;
            sf::RectangleShape m_cursorShape;
            sf::Clock m_cursorClock;
            bool m_showCursor;
            sf::Keyboard::Key m_lastKey;

            sf::Color m_borderColour;
            sf::Color m_selectedColour;

            sf::Uint16 m_maxLength;

            enum State
            {
                Selected = 0,
                Normal,
                Size
            };
            std::vector<sf::IntRect> m_subRects;
            sf::FloatRect m_bounds;
            Alignment m_currentAlignment;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_TEXTBOX_H_