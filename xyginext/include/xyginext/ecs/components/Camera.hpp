/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#pragma once

#include "xyginext/Config.hpp"

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

        Camera();

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
        void setViewport(sf::FloatRect);

        /*!
        \brief Locks the camera movement along a certain axis.
        For example locking the Y axis will allow the camera to follow a
        character as it travels left and right, but will not move vertically
        when the character jumps.
        \param axis Which axis to lock, or None to unlock
        \param float Value at which to lock the given axis. EG lockAxis(Y, 100.f)
        will lock the camera's vertical postion to 100 world units.
        */
        void lockAxis(Axis, float);

        /*
        \brief Locks the rotation of the camera, so that the view does
        not rotate, even if the parent entity does.
        */
        void lockRotation(bool);

        /*!
        \brief Locks the camera movement within the given bounds.
        For example if this is set to the size of the game map the camera
        will stop at the edges preventing the 'outside' from being visible
        */
        void setBounds(sf::FloatRect);

        /*!
        \brief Sets the zoom level relative to the current view size.
        Contrary to default SFML views a value of 2 will make the scene
        appear 2x larger, and a value of less than 1 will zoom out.
        Values must be greater than 0
        */
        void zoom(float);

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

        /*!
        \brief Returns the current bounds of the camera
        */
        sf::FloatRect getBounds() const { return m_bounds; }

    private:

        sf::View m_view;
        Axis m_lockAxis;
        float m_axisValue;
        bool m_lockRotation;

        sf::FloatRect m_bounds;

        friend class Scene;
        friend class CameraSystem;
    };
}
