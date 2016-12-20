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

#ifndef XY_UI_SELECTION_HPP_
#define XY_UI_SELECTION_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <functional>

namespace xy
{
    namespace UI
    {
        /*!
        \brief A selection of items which can be paged left or right
        */
        class XY_EXPORT_API Selection final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Selection>;
            using Callback = std::function<void(const Selection*)>;
            /*!
            \brief Constructor
            \param sf::Font Font to use for the control's text
            \param sf::Texture A texture atlas containing the normal and
            selected state icon placed at either end of the control
            \param float Length of the text area of the control
            */
            Selection(const sf::Font&, const sf::Texture&, float = 350.f);
            ~Selection() = default;
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
            void handleEvent(const sf::Event&, const sf::Vector2f&) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f&) const override;
            /*!
            \brief Adds a name/value pair to the control
            \param std::string Text string to display
            \param sf::Int32 Value of the item
            */
            void addItem(const std::string&, sf::Int32);
            /*!
            \brief Returns the string of the currently selected item
            */
            const std::string& getSelectedText() const;
            /*!
            \brief Returns the value of the currently selected item
            */
            sf::Int32 getSelectedValue() const;
            /*!
            \brief Sets the currently selected item
            \param Index Index of the item to select
            */
            void setSelectedIndex(Index);
            /*!
            \brief Returns the number of items current belonging
            to this control
            */
            sf::Uint32 itemCount() const;
            /*!
            \brief Selects an item by name, if it exists
            */
            void selectItem(const std::string&);
            void selectItem(Index);
            /*!
            \brief Set the callback to be executed when selected item is changed
            */
            void setCallback(Callback);
            /*!
            \brief Sets the colour of the text used by the control
            \param colour Colour to set the text fill colour to
            */
            void setTextColour(sf::Color colour);

        private:
            enum State
            {
                Normal = 0,
                Selected,
                Count
            };

            std::vector<sf::IntRect> m_subRects;

            struct Item
            {
                using Ptr = std::unique_ptr<Item>;
                std::string name;
                sf::Int32 value;

                Item(const std::string&, sf::Int32);
            };

            std::vector<Item::Ptr> m_items;

            float m_length;
            sf::FloatRect m_bounds;

            std::size_t m_selectedIndex;
            sf::Text m_selectedText;

            sf::Sprite m_prevArrow;
            sf::Sprite m_nextArrow;

            sf::RectangleShape m_background;

            enum class SelectedButton
            {
                Prev,
                Next
            } m_selectedButton;

            Callback m_selectionChanged;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
            void updateText();
        };
    }
}
#endif //XY_UI_SELECTION_HPP_