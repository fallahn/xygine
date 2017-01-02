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

#ifndef XY_UI_COMBOBOX_HPP_
#define XY_UI_COMBOBOX_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace xy
{
    namespace UI
    {
        /*!
        \brief ComboBox control

        A drop down collection of name/value pairs of which one can be selected
        */
        class XY_EXPORT_API ComboBox final : public Control
        {
        public:
            using Ptr = std::shared_ptr<ComboBox>;

            /*!
            \brief Constructor
            \param font Font to use with the combobox text
            \param t Texture containing the background image for the combobox
            */
            ComboBox(const sf::Font& font, const sf::Texture& t);
            ~ComboBox() = default;
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
            bool contains(const sf::Vector2f& mousePos) const override;
            /*!
            \brief Add a name / value pair to the combobox

            The name will appear in the drop down list, and the value
            is the value returned when the item is currently selected
            */
            void addItem(const std::string& name, sf::Int32 value);
            /*!
            \brief Get the text string of the currently selected item
            */
            const std::string& getSelectedText() const;
            /*!
            \brief Get the value of the currently selected item
            */
            sf::Int32 getSelectedValue() const;
            /*!
            \brief Set the background colour of the drop-down
            */
            void setBackgroundColour(const sf::Color&);
            /*!
            \brief Set colour of the highlight item in the drop-down
            */
            void setHighlightColour(const sf::Color&);
            /*!
            \brief Selects the item at the given index
            */
            void setSelectedIndex(Index index);
            /*!
            \brief Returns the number of items in the combobox
            */
            sf::Uint32 size() const;
            /*!
            \brief sets the selected item by name, if it exists
            */
            void selectItem(const std::string&);
            void selectItem(Index);

        private:
            struct Item
            {
                using Ptr = std::unique_ptr<Item>;
                std::string name;
                sf::Int32 value;

                sf::FloatRect bounds;
                sf::Text text;

                Item(const std::string& name, sf::Int32 value, const sf::Font& font);
            };

            std::vector<Item::Ptr> m_items;

            bool m_showItems;
            sf::RectangleShape m_mainShape;
            sf::RectangleShape m_dropDownShape;
            mutable sf::RectangleShape m_highlightShape;

            Index m_selectedIndex;
            Index m_nextIndex;
            sf::Text m_selectedText;

            const sf::Font& m_font;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_COMBOBOX_HPP_