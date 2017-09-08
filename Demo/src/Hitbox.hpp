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

#ifndef DEMO_HITBOX_HPP_
#define DEMO_HITBOX_HPP_

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>

enum class CollisionType
{
    None,
    Player,
    Platform,
    Solid
};

struct Manifold final
{
    float penetration = 0.f;
    sf::Vector2f normal;
    CollisionType otherType = CollisionType::None;
};

class Hitbox final
{
public:
    static constexpr sf::Uint32 MaxCollisions = 16;
    
    CollisionType getType() const { return m_collisionType; }
    void setType(CollisionType type) { m_collisionType = type; }

    const std::array<Manifold, MaxCollisions>& getManifolds() const { return m_manifolds; }
    std::size_t getCollisionCount() const { return m_collisionCount; }

    sf::FloatRect getCollisionRect() const { return m_collisionRect; }
    void setCollisionRect(sf::FloatRect rect) { m_collisionRect = rect; }

private:
    CollisionType m_collisionType;
    std::array<Manifold, MaxCollisions> m_manifolds{};
    std::size_t m_collisionCount;
    sf::FloatRect m_collisionRect;
};

#endif //DEMO_HITBOX_HPP_
