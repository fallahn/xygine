/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#ifndef XY_CAMERA_HPP_
#define XY_CAMERA_HPP_

#include <xyginext/Config.hpp>

#include <SFML/Graphics/View.hpp>

namespace xy
{
    /*!
    \brief Represents a 2D camera.
    Camera components can be attached to entities so that they
    take on the transform of any transform component, and also be
    used to bound the game world view.
    */
    class XY_EXPORT_API Camera final
    {
    public:
        enum Axis
        {
            X, Y, None
        };


        /*!
        \brief Sets the visible area, in world units, of the camera.
        This defaults to the size of the window.
        */
        void setView(sf::Vector2f);

        /*!
        \brief Sets the normalised viewport (0.f - 1.f) of this camera.
        The viewport represents the percentage of the render target
        which is rendered to.
        \see sf::View::setViewport()
        */
        void setViewPort(sf::FloatRect);

        /*!
        \brief Locks the camera movement along a certain axis.
        For example locking the Y axis will allow the camera to follow a
        character as it travels left and right, but will not move vertically
        when the character jumps.
        */
        void lockAxis(Axis);

        /*
        \brief Locks the rotation of the camera, so that the view does
        not rotate, even if the parent entity does.
        */
        void lockRotation(bool);

        /*!
        \brief Returns the current view of the camera, in world units
        */
        sf::Vector2f getView() const;

        /*!
        \brief Returns the current viewport
        */
        sf::FloatRect getViewport() const;

        /*!
        \brief Returns which axis is currently locked
        */
        Axis getLockedAxis() const;

        /*!
        \brief Returns true if rotation is locked, else
        returns false
        */
        bool rotationLocked() const;

    private:

        sf::View m_view;
        Axis m_lockAxis;
        bool m_lockRotation;
    };
}


#endif //XY_CAMERA_HPP_