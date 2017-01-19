/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_UI_BUTTON_HPP_
#define XY_UI_BUTTON_HPP_

#include <xygine/ui/Control.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace xy
{
    namespace UI
    {
        /*!
        \brief Clickable and optionally togglable button
        */
        class XY_EXPORT_API Button final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Button>;
            using Callback = std::function<void()>;

            /*!
            \brief Constructor
            \param font Font to use for this button's text
            \param texture Texture containing an atlas of the 3 button states:
            Normal, Selected, Active.
            */
            Button(const sf::Font& font, const sf::Texture& texture);
            ~Button() = default;
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
            \brief Activates the control

            When the button is activated its click callback is performed, if it exists
            \see Control
            */
            void activate() override;
            /*!
            \brief Deactivates the button if it is set to togglable
            */
            void deactivate() override;
            /*!
            \see Control
            */
            void handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment a) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f& mousePos)const override;
            /*!
            \brief Allows adding one or more Callbacks to be performed when the button is activated
            */
            void addCallback(const Callback& c);
            /*!
            \brief Sets the button's text's string
            */
            void setString(const std::string& text);
            /*!
            \brief Sets the colours of the button's text
            */
            void setTextColour(const sf::Color& c);
            /*!
            \brief Sets the font size of the button's text
            */
            void setFontSize(sf::Uint16 size);
            /*!
            \brief Set whether or not the button is togglable
            */
            void setTogglable(bool b);


        private:
            enum State
            {
                Normal = 0,
                Selected,
                Active,
                Count
            };

            std::vector<Callback> m_callbacks;
            const sf::Texture& m_texture;
            sf::Sprite m_sprite;
            sf::Text m_text;
            bool m_toggleButton;

            std::vector<sf::IntRect> m_subRects;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_BUTTON_HPP_