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

//base class for ui controls

#ifndef XY_UI_CONTROL_HPP_
#define XY_UI_CONTROL_HPP_

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <memory>

namespace sf
{
    class Event;
}

namespace xy
{
    namespace UI
    {
        using Index = std::size_t;
        enum class Alignment
        {
            TopLeft,
            BottomLeft,
            Centre,
            TopRight,
            BottomRight
        };

        class Control : public sf::Drawable, public sf::Transformable
        {
        public:
            using Ptr = std::shared_ptr<Control>;

            Control();
            virtual ~Control() = default;
            Control(const Control&) = delete;
            const Control& operator = (const Control&) = delete;

            virtual bool selectable() const = 0;
            bool selected() const;

            virtual void select();
            virtual void deselect();

            virtual bool active() const;
            virtual void activate();
            virtual void deactivate();

            virtual void handleEvent(const sf::Event&, const sf::Vector2f&) = 0;
            virtual void update(float dt){};

            virtual void setAlignment(Alignment) = 0;
            virtual bool contains(const sf::Vector2f& mousePos) const;

            void setVisible(bool visible);
            bool visible() const;

        private:

            bool m_selected;
            bool m_active;
            bool m_visible;
            Index m_index;
        };
    }
}
#endif //XY_UI_CONTROL_H_