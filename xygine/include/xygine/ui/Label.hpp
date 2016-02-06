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

#ifndef XY_UI_LABEL_HPP_
#define XY_UI_LABEL_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/Text.hpp>

namespace xy
{
    namespace UI
    {
        /*!
        \brief Creates a text label control. Labels are not interactive
        */
        class XY_EXPORT_API Label final : public Control
        {
        public:
            using Ptr = std::shared_ptr<Label>;

            /*!
            \brief Constructor
            \param font Font to use with this label's text
            */
            explicit Label(const sf::Font&);
            /*!
            \see Control
            */
            bool selectable() const override;
            /*!
            \see Control
            */
            void handleEvent(const sf::Event&, const sf::Vector2f&) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment) override;
            /*!
            \brief Set the label's text string
            */
            void setString(const std::string&);
            /*!
            \brief Set the label's text colour
            */
            void setColour(const sf::Color&);
            /*!
            \brief Set the label's text size
            */
            void setCharacterSize(sf::Uint32);

        private:

            sf::Text m_text;
            Alignment m_currentAlignment;

            void updateText();
            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
    }
}
#endif //XY_UI_LABEL_HPP_