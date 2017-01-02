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
        /*!
        \brief A variable slider control

        Slider values range from 0 - maxValue defined on construction
        */
        class XY_EXPORT_API Slider final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Slider>;
            using Callback = std::function<void(const Slider*)>;

            enum class Direction
            {
                Horizontal,
                Vertical
            };
            /*!
            \brief Slider events
            */
            enum class Event
            {
                ValueChanged,
                SetActive,
                SetInactive
            };

            /*!
            \brief Constructor
            \param font Font to use for the slider's text
            \param texture Texture atlas containing the image used for the pointer in normal and selected states
            \param length Length in pixel of the slider
            \param maxValue Maximum value this slider returns
            */
            Slider(const sf::Font& font, const sf::Texture& texture, float length = 250.f, float maxValue = 100.f);
            ~Slider() = default;
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
            void setAlignment(Alignment a) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f& mousePos) const override;
            /*!
            \brief Set the maxium value returned by the slider
            */
            void setMaxValue(float value);
            /*!
            \brief Set the visual direction of the slider
            \param direction Sliders can be Horizontal or Vertical
            */
            void setDirection(Direction direction);
            /*!
            \brief Sets the length of the slider
            */
            void setLength(float length);
            /*!
            \brief Set the current value of the slider
            \param value can be anywhere between 0 and maxValue
            */
            void setValue(float value);
            /*!
            \brief Get the current value of the slider
            */
            float getValue() const;
            /*!
            \brief Returns the current length of the slider
            */
            float getLength() const;
            /*!
            \brief Set the text string of the slider
            */
            void setString(const std::string& text);
            /*!
            \brief Set the colour of the slider text
            */
            void setTextColour(const sf::Color& colour);
            /*!
            \brief Set the character size of the font used by the slider string
            */
            void setFontSize(sf::Uint16 size);
            /*!
            \brief Add an even callback
            \param c Calbback to be executed on event
            \param e Event ot be paired with callback
            \see Slider::Event
            */
            void addCallback(const Callback& c, Event e);
            /*!
            \brief Sets the colour of the slider bar
            \param inner Inner colours of the bar
            \param outer Outer colour of the bar
            */
            void setBarColour(sf::Color inner, sf::Color outer = sf::Color::Transparent);

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