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

//base class for ui controls

#ifndef XY_UI_CONTROL_HPP_
#define XY_UI_CONTROL_HPP_

#include <xygine/ShaderProperty.hpp>
#include <xygine/Config.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <memory>

namespace sf
{
    class Event;
}

namespace xy
{
    /*!
    \brief Controls from which xygine's UI objects are composed
    */
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

        /*!
        \brief Abstract base class for all UI controls
        */
        class XY_EXPORT_API Control : public sf::Drawable, public sf::Transformable, public ShaderProperty
        {
        public:
            using Ptr = std::shared_ptr<Control>;

            Control();
            virtual ~Control() = default;
            Control(const Control&) = delete;
            const Control& operator = (const Control&) = delete;
            /*!
            \brief Returns true if this control is selectable

            When implementing a concrete type this should return true
            if the control is selectable when navigating controls in a
            container, else it should return false.
            */
            virtual bool selectable() const = 0;
            /*!
            \brief Returns true if the control is currently selected
            */
            bool selected() const;
            /*!
            \brief Used to select the control when navigating controls
            belonging to a Container
            */
            virtual void select();
            /*!
            \brief Used to deselect the control when navigating controls
            belonging to a Container
            */
            virtual void deselect();
            /*!
            \brief Returns true if the control is currently active.
            */
            virtual bool active() const;
            /*!
            \brief Activates the control if it is selected

            Override this if the control requires any special animation
            or needsd to call custom callbacks.
            */
            virtual void activate();
            /*!
            \brief Deactivates activated controls
            */
            virtual void deactivate();
            /*!
            \brief Handle input events

            Must be implemented by concrete types to handle any mouse, keyboard
            or controller input
            \param sf::Event Current event structure
            \param sf::Vector2f current mouse position, in UI space
            */
            virtual void handleEvent(const sf::Event&, const sf::Vector2f&) = 0;
            /*!
            \brief Updates the control once per frame

            Override this when controls require time dependent updates, such as
            animated textures.
            */
            virtual void update(float dt){};
            /*!
            \brief Allows setting the alignment of the control about its origin

            This is required for all controls to implement placement for each
            of the Alignment enum options
            */
            virtual void setAlignment(Alignment) = 0;
            /*!
            \brief Returns true if the Control bounds contains the given
            mouse position in UI space coordinates.

            This can be optionally overriden for controls composed of
            complex shapes.
            */
            virtual bool contains(const sf::Vector2f& mousePos) const;
            /*!
            \brief Hide or show the control
            \param visible If true shows the control, else hides it

            Hidden controls are neither drawn, nor handle any input events
            effectively disabling them
            */
            void setVisible(bool visible);
            /*!
            \brief Returns true if the control is currently visible, else returns false
            */
            bool visible() const;

        private:

            bool m_selected;
            bool m_active;
            bool m_visible;
            Index m_index;
        };

        /*!
        \brief Utility function for creating controls
        */
        template <typename T, typename... Args>
        std::shared_ptr<T> create(Args&&... args)
        {
            static_assert(std::is_base_of<Control, T>::value, "Must derive from Control class");
            return std::move(std::make_shared<T>(std::forward<Args>(args)...));
        }
    }
}
#endif //XY_UI_CONTROL_H_