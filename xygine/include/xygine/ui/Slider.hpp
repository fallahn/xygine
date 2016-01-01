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

//slider control

#ifndef XY_UI_SLIDER_HPP_
#define XY_UI_SLIDER_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <functional>

namespace xy
{
    namespace UI
    {
        class Slider final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Slider>;
            using Callback = std::function<void(const Slider*)>;

            enum class Direction
            {
                Horizontal,
                Vertical
            };

            enum class Event
            {
                ValueChanged,
                SetActive,
                SetInactive
            };

            Slider(const sf::Font& font, const sf::Texture& texture, float length = 250.f, float maxValue = 100.f);
            ~Slider() = default;

            bool selectable() const override;
            void select() override;
            void deselect() override;

            void activate() override;
            void deactivate() override;

            void handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) override;

            void setAlignment(Alignment a) override;
            bool contains(const sf::Vector2f& mousePos) const override;

            void setMaxValue(float value);
            void setDirection(Direction direction);
            void setLength(float length);
            void setValue(float value);
            float getValue() const;
            float getLength() const;

            void setText(const std::string& text);
            void setTextColour(const sf::Color& colour);
            void setFontSize(sf::Uint16 size);

            void setCallback(Callback c, Event e);

        private:
            enum State
            {
                Normal = 0,
                Selected
            };

            float m_maxValue;
            float m_length;
            Direction m_direction;
            sf::FloatRect m_localBounds;

            sf::Sprite m_handleSprite;
            sf::RectangleShape m_slotShape;
            std::vector<sf::IntRect> m_subRects;
            sf::Text m_text;
            sf::Text m_valueText;

            sf::Color m_borderColour;
            sf::Color m_activeColour;

            std::vector<Callback> m_valueChanged;
            Callback m_setActive;
            Callback m_setInactive;

            void draw(sf::RenderTarget& rt, sf::RenderStates states) const;
            void updateText();

            void increase();
            void decrease();

            void valueChanged(const Slider*);
        };
    }
}
#endif //XY_UI_SLIDER_HPP_