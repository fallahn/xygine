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

#pragma once

#include <xyginext/core/Assert.hpp>
#include <xyginext/ecs/Entity.hpp>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>
#include <limits>

namespace CollisionType
{
    enum ID //these are actually in a specific order for collision checks
    {
        None = 0,
        Player = 0x1,
        Foot = 0x2,
        Solid = 0x4,
        Teleport = 0x8,
        NPC = 0x10,
        Fruit = 0x20,
        Powerup = 0x40,
        Platform = 0x80,
        Bubble = 0x100,
        HardBounds = 0x200,
        MagicHat = 0x400,
        Crate = 0x800,
        Explosion = 0x1000,
        Dynamite = 0x2000
    };
};

struct Manifold final
{
    float penetration = 0.f;
    sf::Vector2f normal;
    CollisionType::ID otherType = CollisionType::None;
    xy::Entity otherEntity;
};

class Hitbox final
{
public:
    static constexpr std::uint32_t MaxCollisions = 16;
    
    CollisionType::ID getType() const { return m_collisionType; }
    void setType(CollisionType::ID type) { m_collisionType = type; }

    const std::array<Manifold, MaxCollisions>& getManifolds() const { return m_manifolds; }
    std::size_t getCollisionCount() const { return m_collisionCount; }

    sf::FloatRect getCollisionRect() const { return m_collisionRect; }
    void setCollisionRect(sf::FloatRect rect) { m_collisionRect = rect; }

private:
    CollisionType::ID m_collisionType = CollisionType::None;
    std::array<Manifold, MaxCollisions> m_manifolds{};
    std::size_t m_collisionCount = 0;
    sf::FloatRect m_collisionRect;

    friend class CollisionSystem;
    friend class CollisionComponent;
};

class CollisionComponent final
{
public:
    static constexpr std::uint32_t MaxHitBoxes = 2;

    CollisionComponent();

    void addHitbox(sf::FloatRect rect, CollisionType::ID type);

    std::size_t getHitboxCount() const { return m_hitboxCount; }

    const std::array<Hitbox, MaxHitBoxes>& getHitboxes() const { return m_hitboxes; }

    //returns the combined bounds for all hitboxes for a broadphase pass
    sf::FloatRect getLocalBounds() const { return m_localBounds; }

    void setCollisionCategoryBits(std::uint32_t bits) { m_categoryBits = bits; }

    void setCollisionMaskBits(std::uint32_t bits) { m_maskBits = bits; }

    std::uint32_t getCollisionCategoryBits() const { return m_categoryBits; }

    std::vector<std::uint8_t> serialise() const;

    void deserialise(const std::vector<std::uint8_t>&);

private:
    std::size_t m_hitboxCount = 0;
    std::array<Hitbox, MaxHitBoxes> m_hitboxes{};
    sf::FloatRect m_localBounds{};

    std::uint32_t m_categoryBits = 1;
    std::uint32_t m_maskBits = std::numeric_limits<std::uint32_t>::max();

    friend class CollisionSystem;
};
