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

//checkbox control with text label

#ifndef XY_UI_CHECKBOX_HPP_
#define XY_UI_CHECKBOX_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>
#include <functional>

namespace xy
{
    namespace UI
    {
        /*!
        \brief A togglable checkbox control
        */
        class XY_EXPORT_API CheckBox final : public Control
        {
        public:
            using Ptr = std::shared_ptr<CheckBox>;
            using Callback = std::function<void(const CheckBox*)>;

            enum class Event
            {
                CheckChanged
            };

            /*!
            \brief Constructor
            \param font Font to use with the checkbox's text
            \param texture Texture containing an atlas of the
            the states: Normal Unchecked, Normal Checked,
            Selected Unchecked, Selected Checked
            */
            CheckBox(const sf::Font& font, const sf::Texture& t);
            ~CheckBox() = default;
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

            Any associated callbacks are called if they exist
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
            void handleEvent(const sf::Event&, const sf::Vector2f&) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f& mousePos) const override;
            /*!
            \brief Set the text for the checkbox's label
            */
            void setText(const std::string& text);
            /*!
            \brief Set the colour of the text of the checkbox's label
            */
            void setTextColour(const sf::Color&);
            /*!
            \brief Set the font used by the checkbox's label
            */
            void setFont(const sf::Font&);
            /*!
            \brief Set the character size of the text used by the checkbox's label
            */
            void setFontSize(sf::Uint8);
            /*!
            \brief Returns true if checkbox is checked
            */
            bool checked() const;
            /*!
            \brief Check or uncheck the checkbox

            Any callback associated with CheckChanged event will be called
            \param checked Set to true to check, false to uncheck
            */
            void check(bool checked = true);
            /*!
            \brief Supply a Callback to be executed on the given event
            \param Callback Callback to execute
            \param Event Event on which to execute Callback
            */
            void addCallback(const Callback&, Event);

        private:
            enum State
            {
                Normal = 0,
                Selected,
                CheckedNormal,
                CheckedSelected
            };

            const sf::Texture& m_texture;
            sf::Sprite m_sprite;
            sf::Text m_text;
            std::vector<sf::IntRect> m_subRects;
            bool m_checked;
            Alignment m_alignment;

            std::vector<Callback> m_checkChangedCallbacks;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_CHECKBOX_HPP_