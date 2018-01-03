/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include <xyginext/ecs/systems/CameraSystem.hpp>
#include <xyginext/ecs/components/Camera.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/util/Math.hpp>

#include <xyginext/core/App.hpp>

using namespace xy;

CameraSystem::CameraSystem(MessageBus& mb)
    : System(mb, typeid(CameraSystem))
{
    requireComponent<Transform>();
    requireComponent<Camera>();
}

//public
void CameraSystem::process(float)
{
    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& cam = entity.getComponent<Camera>();
        const auto& xForm = entity.getComponent<Transform>();
       
        auto position = xForm.getWorldTransform().transformPoint({});

        //check axis lock
        if (cam.m_lockAxis == Camera::X)
        {
            position.x = cam.m_axisValue;
        }
        else if (cam.m_lockAxis == Camera::Y)
        {
            position.y = cam.m_axisValue;
        }

        //check if camera locked within bounds
        auto offset = cam.m_view.getSize() / 2.f;
        position.x = Util::Math::clamp(position.x, cam.m_bounds.left + offset.x, (cam.m_bounds.left + cam.m_bounds.width) - offset.x);
        position.y = Util::Math::clamp(position.y, cam.m_bounds.top + offset.y, (cam.m_bounds.top + cam.m_bounds.height) - offset.y);

        cam.m_view.setCenter(position);

        //check rotation lock
        if (!cam.m_lockRotation)
        {
            cam.m_view.setRotation(xForm.getRotation());
        }
    }
}