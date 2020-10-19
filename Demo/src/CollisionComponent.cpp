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

#include "Hitbox.hpp"

#include <cstring>

namespace
{
    constexpr std::size_t ManSize = sizeof(Manifold::penetration) + sizeof(Manifold::normal) + sizeof(Manifold::otherType);
    constexpr std::size_t HitboxSize = sizeof(CollisionType::ID) + sizeof(std::size_t);
    constexpr std::size_t ComponentSize = sizeof(std::size_t) + sizeof(std::uint32_t) + sizeof(std::uint32_t);
}

CollisionComponent::CollisionComponent()
{
    float fMax = std::numeric_limits<float>::max();
    m_localBounds.left = fMax;
    m_localBounds.top = fMax;
    m_localBounds.width = -fMax;
    m_localBounds.height = -fMax;
}

//public
void CollisionComponent::addHitbox(sf::FloatRect rect, CollisionType::ID type)
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

std::vector<std::uint8_t> CollisionComponent::serialise() const
{
    std::size_t size = 0;
    for (auto i = 0u; i < m_hitboxCount; ++i)
    {
        size += (m_hitboxes[i].getCollisionCount() * ManSize) + HitboxSize;
    }
    size += ComponentSize;

    std::vector<std::uint8_t> data(size);
    auto ptr = data.data();

    //catbits, maskbits, hitbox count
    std::memcpy(ptr, &m_categoryBits, sizeof(m_categoryBits));
    ptr += sizeof(m_categoryBits);
    std::memcpy(ptr, &m_maskBits, sizeof(m_maskBits));
    ptr += sizeof(m_maskBits);
    std::memcpy(ptr, &m_hitboxCount, sizeof(m_hitboxCount));
    ptr += sizeof(m_hitboxCount);

    //each hitbox
    for (auto i = 0u; i < m_hitboxCount; ++i)
    {
        //collision type, manifold count
        const auto& hitbox = m_hitboxes[i];
        
        auto type = hitbox.getType();
        std::memcpy(ptr, &type, sizeof(type));
        ptr += sizeof(type);
        auto count = hitbox.getCollisionCount();
        std::memcpy(ptr, &count, sizeof(count));
        ptr += sizeof(count);

        //each manifold in current hitbox
        for (auto j = 0u; j < count; ++j)
        {
            const auto& man = hitbox.getManifolds()[j];
            //penetration, normal, other type
            std::memcpy(ptr, &man.penetration, sizeof(man.penetration));
            ptr += sizeof(man.penetration);
            std::memcpy(ptr, &man.normal, sizeof(man.normal));
            ptr += sizeof(man.normal);
            std::memcpy(ptr, &man.otherType, sizeof(man.otherType));
            ptr += sizeof(man.otherType);
        }
    }

    return data;
}

void CollisionComponent::deserialise(const std::vector<std::uint8_t>& data)
{
    auto ptr = data.data();

    //catbits, maskbits, hitbox count
    std::memcpy(&m_categoryBits, ptr, sizeof(m_categoryBits));
    ptr += sizeof(m_categoryBits);
    std::memcpy(&m_maskBits, ptr, sizeof(m_maskBits));
    ptr += sizeof(m_maskBits);
    std::memcpy(&m_hitboxCount, ptr, sizeof(m_hitboxCount));
    ptr += sizeof(m_hitboxCount);

    //hitboxes
    for (auto i = 0u; i < m_hitboxCount; ++i)
    {
        //collision type, manifold count
        auto& hitbox = m_hitboxes[i];

        CollisionType::ID type;
        std::memcpy(&type, ptr, sizeof(type));
        ptr += sizeof(type);
        hitbox.setType(type);

        std::size_t count;
        std::memcpy(&count, ptr, sizeof(count));
        ptr += sizeof(count);
        hitbox.m_collisionCount = count;

        //each manifold in hitbox
        for (auto j = 0u; j < count; ++j)
        {
            auto& man = hitbox.m_manifolds[j];
            //penetration, normal, other type
            std::memcpy(&man.penetration, ptr, sizeof(man.penetration));
            ptr += sizeof(man.penetration);
            std::memcpy(&man.normal, ptr, sizeof(man.normal));
            ptr += sizeof(man.normal);
            std::memcpy(&man.otherType, ptr, sizeof(man.otherType));
            ptr += sizeof(man.otherType);
        }
    }
}