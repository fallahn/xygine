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

//contains one or more ui controls

#ifndef XY_UI_CONTAINER_HPP_
#define XY_UI_CONTAINER_HPP_

#include <xygine/ui/Control.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

#include <vector>

namespace xy
{
    class MessageBus;
    namespace UI
    {
        //not final, forms inherit from this

        /*!
        \brief Container for controls

        Using a container allows easy navigation around a set of UI controls, as well as
        easy placement on screen. A container is like an invisible window, although it
        can have a colour or texture set as its background. Controls are navigated in
        the order in which they are added to the container.
        */
        class XY_EXPORT_API Container : public Control
        {
        public:
            using Ptr = std::shared_ptr<Container>;

            explicit Container(xy::MessageBus&);
            Container(Container&& c) : m_messageBus(c.m_messageBus), m_selectedIndex(c.m_selectedIndex), m_background(c.m_background){}
            Container& operator=(Container&&){ return *this; }

            ~Container() = default;

            /*!
            \brief Add a control to this container
            */
            void addControl(Control::Ptr control);
            /*!
            \see Control
            */
            virtual bool selectable() const override;
            /*!
            \see Control
            */
            virtual void handleEvent(const sf::Event& e, const sf::Vector2f& mousePos) override;
            /*!
            \see Control
            */
            virtual void update(float dt) override;
            /*!
            \see Control
            */
            virtual void setAlignment(Alignment a) override{} //TODO flow layout
            /*!
            \brief Set the background colour of the container

            This is overriden if a background texture is set.
            */
            void setBackgroundColour(const sf::Color&);
            /*!
            \brief Set he texture to be drawn in the background of the container
            */
            void setBackgroundTexture(const sf::Texture&);
        private:
            xy::MessageBus& m_messageBus;

            std::vector<Control::Ptr> m_controls;
            Index m_selectedIndex;

            sf::RectangleShape m_background;

            bool hasSelection() const;
            void select(Index index);
            void selectNext();
            void selectPrevious();

            virtual void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        };
    }
}
#endif //XY_UI_CONTAINER_HPP_