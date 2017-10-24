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

#ifndef DEMO_SPRING_FLOWER_HPP_
#define DEMO_SPRING_FLOWER_HPP_

#include <xyginext/ecs/System.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <vector>

struct SpringFlower final
{
    SpringFlower(float length = -80.f)
        : headPos(0.f, length), restPos(headPos) {}

    sf::Vector2f rootPos;
    sf::Vector2f headPos;
    sf::Vector2f restPos;
    sf::Vector2f externalForce;

    sf::Vector2f velocity;
    float mass = 1.f;
    float stiffness = -5.f;
    float damping = -1.f;

    sf::FloatRect textureRect;
    sf::Color colour = sf::Color::White;
};

class SpringFlowerSystem final : public xy::System
{
public:
    explicit SpringFlowerSystem(xy::MessageBus&);

    void process(float) override;

private:
    void onEntityAdded(xy::Entity) override;

    std::vector<float> m_windTable;
    std::size_t m_windIndex;

    std::vector<float> m_windModulator;
    std::size_t m_modulatorIndex;
};

#endif //DEMO_SPRING_FLOWER_HPP_