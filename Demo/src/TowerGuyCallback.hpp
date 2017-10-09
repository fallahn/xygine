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

#ifndef DEMO_TOWERGUY_CALLBACK_HPP_
#define DEMO_TOWERGUY_CALLBACK_HPP_

#include "ClientServerShared.hpp"

#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>

#include <SFML/System/Vector2.hpp>

class TowerGuyCallback final
{
public:
    explicit TowerGuyCallback(xy::Scene& scene) : m_scene(scene), m_velocity(-100.f, -100.f) {}
    
    void operator () (xy::Entity entity, float dt)
    {
        m_velocity.y += Gravity * dt;
        entity.getComponent<xy::Transform>().move(m_velocity * dt);
        if (entity.getComponent<xy::Transform>().getPosition().y > xy::DefaultSceneSize.y)
        {
            m_scene.destroyEntity(entity);
        }
    }

private:
    xy::Scene& m_scene;
    sf::Vector2f m_velocity;
};

#endif //DEMO_TOWERGUY_CALLBACK_HPP_