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
    namespace ui
    {
        class CheckBox final : public Control
        {
        public:
            using Ptr = std::shared_ptr<CheckBox>;
            using Callback = std::function<void(const CheckBox*)>;

            enum class Event
            {
                CheckChanged
            };

            CheckBox(const sf::Font& font, const sf::Texture& t);
            ~CheckBox() = default;

            bool selectable() const override;
            void select() override;
            void deselect() override;

            void activate() override;
            void deactivate() override;

            void handleEvent(const sf::Event&, const sf::Vector2f&) override;
            void setAlignment(Alignment) override;
            bool contains(const sf::Vector2f& mousePos) const override;

            void setText(const std::string& text);
            void setTextColour(const sf::Color&);
            void setFont(const sf::Font&);
            void setFontSize(sf::Uint8);

            bool checked() const;
            void check(bool checked = true);

            void setCallback(Callback, Event);

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

            Callback m_checkChanged;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_CHECKBOX_HPP_