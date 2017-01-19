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

#ifndef XY_UI_KEYBINDS_HPP_
#define XY_UI_KEYBINDS_HPP_

#include <xygine/ui/Control.hpp>
#include <xygine/KeyBinds.hpp>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <array>

namespace xy
{
    namespace UI
    {
        /*!
        \brief UI control for displaying the currently stored keybinds
        and updating them - such as via an options or pause menu
        */
        class XY_EXPORT_API KeyBinds final : public Control
        {
        public:
            using Ptr = std::shared_ptr<KeyBinds>;

            /*!
            \brief Constructor
            \param sf::Font font with which to draw this control's text
            */
            explicit KeyBinds(const sf::Font&);
            ~KeyBinds() = default;

            /*!
            \see UI::Control
            */
            bool selectable() const override { return true; }
            
            /*!
            \see UI::Control
            */
            bool contains(const sf::Vector2f&) const override;

            /*!
            \see UI::Control
            */
            void handleEvent(const sf::Event&, const sf::Vector2f&) override;

            /*!
            \see UI::Control
            */
            void setAlignment(Alignment) override;


        private:

            enum class ItemInput
            {
                First,
                Second,
                None
            };

            class Item final : public sf::Drawable, public sf::Transformable
            {
            public:
                Item();
                ~Item() = default;

                void setLabel(const std::string&);
                void setValues(const std::string&, const std::string&);
                void setFont(const sf::Font&);

                ItemInput onClick(const sf::Vector2f&);
                void clearSelection();

                sf::FloatRect getLocalBounds() const;

            private:
                std::pair<sf::FloatRect, sf::FloatRect> m_boundingBoxes;
                std::array<sf::Vertex, 12u> m_vertices;
                std::array<sf::Text, 4u> m_texts;
                void draw(sf::RenderTarget&, sf::RenderStates) const override;
            };

            std::array<Item, xy::Input::KeyCount> m_items;
            sf::Int32 m_selectedIndex;
            ItemInput m_selectedInput;

            sf::FloatRect m_bounds;

            void updateLayout();
            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}

#endif //XY_UI_KEYBINDS_HPP_