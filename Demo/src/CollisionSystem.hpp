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

#ifndef DEMO_COLLISION_SYSTEM_HPP_
#define DEMO_COLLISION_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <set>

class CollisionSystem final :public xy::System
#ifdef _DEBUG_
    , public sf::Drawable
#endif
{
public: 
    explicit CollisionSystem(xy::MessageBus&, bool = false);

    void process(float) override;

private:

    void onEntityAdded(xy::Entity) override;
    void onEntityRemoved(xy::Entity) override;
    
    std::set<std::pair<xy::Entity, xy::Entity>> m_collisions;

    bool m_isServer;

#ifdef _DEBUG_
    bool m_drawDebug;
    std::vector<sf::Vertex> m_vertices;
    void draw(sf::RenderTarget&, sf::RenderStates) const override;
#endif
};

#endif //DEMO_COLLISION_SYSTEM_HPP_
