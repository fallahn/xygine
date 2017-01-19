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

#ifndef XY_UI_MARGUEE_HPP_
#define XY_UI_MARQUEE_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <string>

namespace sf
{
    class Font;
}

namespace xy
{
    namespace UI
    {
        /*!
        \brief Creates a scrolling marquue of text which can be
        placed within a UI::Container
        */
        class XY_EXPORT_API Marquee final : public xy::UI::Control
        {
        public:
            /*!
            \brief Constructor
            \param sf::Font Font to draw the marquee text with
            \param sf::Vector2u Size of the marquee text area
            */
            Marquee(const sf::Font&, sf::Vector2u);
            ~Marquee() = default;

            /*!
            \see xy::UI::Control
            */
            bool selectable() const override { return false; }
            /*!
            \see xy::UI::Control
            */
            void handleEvent(const sf::Event&, const sf::Vector2f&) override {}
            /*!
            \see xy::UI::Control
            */
            void setAlignment(Alignment) override;

            /*!
            \see xy::UI::Control
            */
            void update(float) override;

            /*!
            \brief Adds a string of text to be displayed.
            Text strings are displayed in the order in which they are added.
            Each string is completely displayed (scrolled until out of view)
            before the next string begins to be displayed. When the entire
            collection of strings has been displayed the marquee will start
            again from the first string.
            */
            void addString(const std::string&);

            /*!
            \brief Sets the character size of the text displayed by the marquee
            */
            void setCharacterSize(sf::Uint32);

            /*!
            \brief Sets the colour of the text displayed by the marquee
            */
            void setTextColour(sf::Color);

            /*!
            \brief Sets the speed in units per second at which the text is scrolled
            */
            void setScrollSpeed(float);

        private:
            sf::Text m_text;
            mutable sf::RenderTexture m_texture;
            std::vector<std::string> m_strings;
            std::size_t m_currentIndex;
            float m_scrollSpeed;
            sf::FloatRect m_bounds;
            sf::Sprite m_sprite;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}

#endif //XY_UI_MARQUEE