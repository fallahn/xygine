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

#ifndef XY_UI_SCORE_LIST_HPP_
#define XY_UI_SCORE_LIST_HPP_

#include <xygine/ui/Control.hpp>
#include <xygine/Score.hpp>

#include <SFML/Graphics/Text.hpp>

#include <memory>

namespace xy
{
    namespace UI
    {
        /*!
        \brief An animated scrolling list of items for displaying scores.
        Designed for specifically displaying xy::Scores::Item - could
        probably be made to be more flexible
        */
        class XY_EXPORT_API ScoreList final : public Control
        {
        public:
            /*!
            \brief Constructor
            \param sf::Font Font to use for th displayed text
            */
            explicit ScoreList(const sf::Font&);
            ~ScoreList() = default;
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
            void update(float) override;
            /*!
            \see Control
            */
            void setAlignment(Alignment) override;
            /*!
            \see Control
            */
            bool contains(const sf::Vector2f&) const override;
            /*!
            \brief Scrolls the item list a given distance
            */
            void scroll(float);
            /*!
            \brief Sets the list of Scores::Items to display
            */
            void setList(const std::vector<Scores::Item>&);
            /*!
            \brief Set the currently selected index
            */
            void setIndex(Index);
            /*!
            \brief Get the vertical spacing between items

            As fonts vary in size as does the distance between
            items in the list. Use this to determine how far to 
            scroll when switching between items
            */
            float getVerticalSpacing() const;
        private:

            std::vector<sf::Text> m_texts;
            sf::FloatRect m_bounds;
            const sf::Font& m_font;
            float m_scrollTargetDistance;
            float m_scrollCurrentDistance;
            float m_scrollSpeed;
            bool m_doScroll;

            void draw(sf::RenderTarget&, sf::RenderStates) const override;
            void updateTexts(float);
        };
    }
}
#endif //XY_UI_SCORE_LIST_HPP_