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

#include <xygine/ComponentCamera.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>

using namespace xy;

Camera::FactoryFunc Camera::create = std::make_unique<Camera>;

Camera::Camera(MessageBus& mb, const sf::View& initialView)
    :Component      (mb, this),
    m_zoom          (1.f),
    m_lockMask      (0u),
    m_rotation      (0.f),
    m_initialView   (initialView),
    m_position      (initialView.getCenter())
{

}

//public
void Camera::entityUpdate(Entity& entity, float)
{
    m_position = entity.getWorldPosition();
    m_rotation = entity.getRotation();
}

void Camera::handleMessage(const Message&) {}

void Camera::lockTransform(Camera::TransformLock lock, bool locked)
{
    sf::Int8 val = (1 << static_cast<sf::Int8>(lock));
    (locked) ? m_lockMask |= val : ~val;
}

void Camera::setZoom(float zoom)
{
    XY_ASSERT(zoom > 0, "zoom must be greater than zero");
    m_zoom = 1.f / zoom;
}

float Camera::getZoom() const
{
    return m_zoom;
}

void Camera::setViewport(const sf::FloatRect& viewPort)
{
    m_initialView.setViewport(viewPort);
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
    view.setCenter(position);
    return view;
}

void Camera::setView(const sf::View& view)
{
    m_initialView = view;
    m_position = view.getCenter();
}