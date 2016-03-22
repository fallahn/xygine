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

#ifndef LM_COLLISION_COMPONENT_HPP_
#define LM_COLLISION_COMPONENT_HPP_

#include <xygine/components/Component.hpp>

namespace lm
{
    class CollisionComponent final : public xy::Component
    {
    public:
        using Ptr = std::unique_ptr<CollisionComponent>;

        enum class ID
        {
            Alien,
            Bounds,
            Bullet,            
            Mothership,
            Player,
            Tower
        };

        CollisionComponent(xy::MessageBus&, sf::FloatRect, ID);
        ~CollisionComponent() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Physics; }

        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;

        ID getID() const;

        using Callback = std::function<void(CollisionComponent*)>;
        void setCallback(const Callback&);

        void addCollider(CollisionComponent*);

    private:
        xy::Entity* m_entity;
        sf::FloatRect m_localBounds;
        ID m_id;

        Callback resolve;
        std::vector<CollisionComponent*> m_collisions;
    };
}

#endif //LM_COLLISION_COMPONENT_HPP_