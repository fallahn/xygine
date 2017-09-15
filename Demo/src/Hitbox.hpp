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

#include <xyginext/core/Assert.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>
#include <limits>

enum class CollisionType
{
    None,
    Player,
    Foot,
    Platform,
    Solid,
    Teleport,
    Bubble
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
    CollisionType m_collisionType = CollisionType::None;
    std::array<Manifold, MaxCollisions> m_manifolds{};
    std::size_t m_collisionCount = 0;
    sf::FloatRect m_collisionRect;

    friend class CollisionSystem;
};

class CollisionComponent final
{
public:
    static constexpr sf::Uint32 MaxHitBoxes = 2;

    CollisionComponent()
    {
        float fMax = std::numeric_limits<float>::max();
        m_localBounds.left = fMax;
        m_localBounds.top = fMax;
        m_localBounds.width = -fMax;
        m_localBounds.height = -fMax;
    }

    void addHitbox(sf::FloatRect rect, CollisionType type)
    {
        XY_ASSERT(m_hitboxCount < MaxHitBoxes, "No more hitboxes may be added");
        m_hitboxes[m_hitboxCount].setCollisionRect(rect);
        m_hitboxes[m_hitboxCount].setType(type);
        m_hitboxCount++;
        //merge boxes for broad phase culling
        if (m_localBounds.left > rect.left)
        {
            m_localBounds.left = rect.left;
        }
        if (m_localBounds.top > rect.top)
        {
            m_localBounds.top = rect.top;
        }
        if (m_localBounds.width < (rect.left + rect.width))
        {
            m_localBounds.width = (rect.left + rect.width);
        }
        if (m_localBounds.height < (rect.top + rect.height))
        {
            m_localBounds.height = (rect.top + rect.height);
        }
    }
    std::size_t getHitboxCount() const { return m_hitboxCount; }

    const std::array<Hitbox, MaxHitBoxes>& getHitboxes() const { return m_hitboxes; }

    //returns the combined bounds for all hitboxes for a broadphase pass
    sf::FloatRect getLocalBounds() const { return m_localBounds; }

private:
    std::size_t m_hitboxCount = 0;
    std::array<Hitbox, MaxHitBoxes> m_hitboxes{};
    sf::FloatRect m_localBounds{};
    friend class CollisionSystem;
};

#endif //DEMO_HITBOX_HPP_
