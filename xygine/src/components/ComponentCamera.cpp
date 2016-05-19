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

#include <xygine/components/Camera.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

using namespace xy;

Camera::Camera(MessageBus& mb, const sf::View& initialView)
    :Component      (mb, this),
    m_zoom          (1.f),
    m_lockMask      (0u),
    m_rotation      (0.f),
    m_initialView   (initialView),
    m_position      (initialView.getCenter()),
    m_lockToBounds  (false)
{

}

//public
void Camera::entityUpdate(Entity& entity, float)
{
    m_position = entity.getWorldPosition();
    m_rotation = entity.getRotation();
}

void Camera::lockTransform(Camera::TransformLock lock, bool locked)
{
    sf::Int8 val = (1 << static_cast<sf::Int8>(lock));
    (locked) ? m_lockMask |= val : ~val;
}

void Camera::lockBounds(const sf::FloatRect& bounds, bool lock)
{
    XY_ASSERT(bounds.width > 0 && bounds.height > 0, "bounds size cannot have negative values");
    m_lockToBounds = lock;
    m_bounds = bounds;
}

void Camera::setZoom(float zoom)
{
    XY_ASSERT(zoom > 0, "zoom must be greater than zero");
    m_zoom = 1.f / zoom;

    auto msg = sendMessage<xy::Message::SceneEvent>(xy::Message::SceneMessage);
    msg->action = xy::Message::SceneEvent::CameraChanged;
}

float Camera::getZoom() const
{
    return m_zoom;
}

void Camera::setViewport(const sf::FloatRect& viewPort)
{
    m_initialView.setViewport(viewPort);

    auto msg = sendMessage<xy::Message::SceneEvent>(xy::Message::SceneMessage);
    msg->action = xy::Message::SceneEvent::CameraChanged;
}

sf::View Camera::getView() const
{
    auto view = m_initialView;
    view.zoom(m_zoom);

    auto position = m_position;
    if (m_lockMask & static_cast<sf::Int8>(TransformLock::AxisX))
    {
        position.x = view.getCenter().x;
    }
    if (m_lockMask & static_cast<sf::Int8>(TransformLock::AxisY))
    {
        position.y = view.getCenter().y;
    }
    if ((m_lockMask & static_cast<sf::Int8>(TransformLock::Rotation)) == 0)
    {
        view.rotate(m_rotation);
    }

    if (m_lockToBounds)
    {
        auto offset = m_initialView.getSize() / 2.f;
        float edge = 0.f;
        if ((edge = (m_bounds.left + offset.x)) > position.x)
        {
            position.x = edge;
        }
        else if ((edge = ((m_bounds.left + m_bounds.width) - offset.x)) < position.x)
        {
            position.x = edge;
        }

        if ((edge = (m_bounds.top + offset.y)) > position.y)
        {
            position.y = edge;
        }
        else if ((edge = ((m_bounds.top + m_bounds.height) - offset.y)) < position.y)
        {
            position.y = edge;
        }
    }

    view.setCenter(position);
    return view;
}

void Camera::setView(const sf::View& view)
{
    m_initialView = view;
    m_position = view.getCenter();

    auto msg = sendMessage<xy::Message::SceneEvent>(xy::Message::SceneMessage);
    msg->action = xy::Message::SceneEvent::CameraChanged;
}

void Camera::setClearColour(const sf::Color& colour)
{
    m_clearColour = colour;
}

const sf::Color& Camera::getClearColour() const
{
    return m_clearColour;
}