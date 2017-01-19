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

//allows transforming a view of the scene based on the parent entity

#ifndef XY_CAMERA_HPP_
#define XY_CAMERA_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Color.hpp>

namespace xy
{
    /*!
    \brief Camera Component

    Cameras are used to define the view of the scene to be rendered
    on screen. The scene class contains a default camera, which can be
    overriden by attaching a camera component to an entity, and setting
    the scene's active camera to the component (see Scene class). Cameras
    attached to entities take on the entities transform, both translation
    and rotation. This allows a camera to easily follow a player or other
    in game entity.
    */
    class XY_EXPORT_API Camera final : public Component
    {
    public:
        using Ptr = std::unique_ptr<Camera>;

        Camera(MessageBus&, const sf::View& initialView);
        ~Camera() = default;
        Camera(const Camera&) = delete;
        const Camera& operator = (const Camera&) = delete;

        Component::Type type() const override { return Component::Type::Script; }
        void entityUpdate(Entity&, float) override;

        enum class TransformLock
        {
            AxisX = 0x1,
            AxisY = 0x2,
            Rotation = 0x4
        };

        /*!
        \brief Locks or unlocks a degree of camera movement

        It may be desirable to lock the motion of the camera,
        either horizontally, vertically or rotation.

        \param TransformLock Axis or rotation to lock or unlock
        \param bool whether to lock  or unlock the given degree of motion
        */
        void lockTransform(TransformLock, bool = true);
        /*!
        \brief Lock the camera within specific bounds

        The cameras motion can be bound within a given area
        so that it can be prevented from moving out the bounds
        of a game map for example

        \param bounds An sf::FloatRect describing the area in world
        coordinates within which to bind the camera.
        \param bool lock or unlock the camera to the specified bounds
        */
        void lockBounds(const sf::FloatRect& bounds, bool = true);
        /*!
        \brief Set the zoom level of the camera

        Increasing the zoom value of the camera increases the zoom
        (and therefore reduces the visible area) of the rendered scene

        \param float Zoom amount. Must be greter than zero, but can be
        values less than one, which zoom out. Note zooming out doesn't
        respects any locked bounds, which always assumes a zoom value
        of one (the defalt value).
        */
        void setZoom(float);
        /*!
        \brief Returns the current camera zoom amount

        \see setZoom
        */
        float getZoom() const;
        /*!
        \brief Set which part of the render target this camera covers

        \param sf::FloatRect depecting the area of the screen to be
        rendered. Note that these are normalised values (0-1) as a ratio
        of the screen resolution
        */
        void setViewport(const sf::FloatRect&);
        /*!
        \brief Returns the current view of the camera

        Use this to set the view of any render target to which
        you wish to render the scene as it appears to the camera
        */
        sf::View getView() const;
        /*!
        \brief Sets the initial view of the camera
        
        This includes setting the resolution in world units of the camera
        as well as the viewport. This is normally set with the default view
        of the current game state's context, as it is automatically set to
        correctly letterbox the camera to match the current window resolution.
        This is the view seen when the camera zoom is set to one.

        \param sf::View to apply to the camera's initial state
        */
        void setView(const sf::View&);
        /*!
        \brief Sets the colour with which to clear the scene buffer
        
        This is the clear colour applied when using this camera to render
        with a PostProcess effect. Useful for debugging.

        \param sf::Color colour to use.
        */
        void setClearColour(const sf::Color&);
        /*!
        \brief Gets the current clear colour for this camera

        \see setClearColour
        */
        const sf::Color& getClearColour() const;

    private:
        float m_zoom;
        sf::Uint8 m_lockMask;    
        float m_rotation;
        sf::View m_initialView;
        sf::Vector2f m_position;
        sf::FloatRect m_bounds;
        bool m_lockToBounds;
        sf::Color m_clearColour;
    };
}

#endif //XY_CAMERA_HPP_