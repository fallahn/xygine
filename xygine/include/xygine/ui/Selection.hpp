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
        class Selection final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Selection>;
            using Callback = std::function<void(const Selection*)>;

            Selection(const sf::Font&, const sf::Texture&, float = 350.f);
            ~Selection() = default;

            bool selectable() const override;
            void select() override;
            void deselect() override;

            void activate() override;
            void deactivate() override;

            void handleEvent(const sf::Event&, const sf::Vector2f&) override;
            void setAlignment(Alignment) override;
            bool contains(const sf::Vector2f&) const override;

            void addItem(const std::string&, sf::Int32);
            const std::string& getSelectedText() const;
            sf::Int32 getSelectedValue() const;

            void setSelectedIndex(Index);
            sf::Uint32 itemCount() const;
            void selectItem(const std::string&);
            void selectItem(Index);

            void setCallback(Callback);

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