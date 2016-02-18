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

#ifndef NET_BALL_LOGIC_HPP_
#define NET_BALL_LOGIC_HPP_

#include <xygine/components/Component.hpp>

namespace NetDemo
{
    class BallLogic final : public xy::Component
    {
    public:
        explicit BallLogic(xy::MessageBus&);
        ~BallLogic() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void setCollisionObjects(const std::vector<xy::Entity*>&);

        void setVelocity(const sf::Vector2f& vel) { m_velocity = vel; }
        const sf::Vector2f& getVelocity() const { return m_velocity; }
        sf::Vector2f getPosition() const;
        sf::Uint32 getCurrentStep() const { return m_stepCount; }

        void reconcile(const sf::Vector2f& position, const sf::Vector2f& velocity, sf::Uint32 step);

    private:

        sf::Vector2f m_velocity;
        std::vector<xy::Entity*> m_collisionObjects;
        sf::Uint32 m_stepCount; //TODO handle wrap
        xy::Entity* m_entity;

        void resolveCollision(const sf::FloatRect&, const sf::Vector2f&, xy::Entity&);
        void killBall(xy::Entity&);
    };
}
#endif //NET_BALL_LOGIC_HPP_