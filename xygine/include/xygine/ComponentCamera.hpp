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

//allows transforming a view of the scene based on the parent entity

#ifndef XY_CAMERA_HPP_
#define XY_CAMERA_HPP_

#include <xygine/Component.hpp>

namespace xy
{
    class Camera final : public Component
    {
    public:
        using Ptr = std::unique_ptr<Camera>;
    private:
        using FactoryFunc = Ptr(&)(MessageBus&, const sf::View&);
    public:
        Camera(MessageBus&, const sf::View& initialView);
        ~Camera() = default;
        Camera(const Camera&) = delete;
        const Camera& operator = (const Camera&) = delete;

        static const FactoryFunc create;

        Component::Type type() const override { return Component::Type::Script; }
        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;

        enum class TransformLock
        {
            AxisX = 0x1,
            AxisY = 0x2,
            Rotation = 0x4
        };

        //prevent this camera moving along the X or Y axis
        //or lock its current rotation
        void lockTransform(TransformLock, bool = true);
        //set the zoom level of the camera
        void setZoom(float);
        //returns the current camera zoom amount
        float getZoom() const;
        //set which part of the render target this camera covers
        //note that these are normalised values (0-1) as a ratio of the screen
        void setViewport(const sf::FloatRect&);
        //returns the current view of the camera
        sf::View getView() const;
        //sets the initial view of the camera before it applies
        //the parent entity's transform
        void setView(const sf::View&);

    private:
        float m_zoom;
        sf::Uint8 m_lockMask;    
        float m_rotation;
        sf::View m_initialView;
        sf::Vector2f m_position;
    };
}

#endif //XY_CAMERA_HPP_