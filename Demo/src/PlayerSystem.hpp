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

#ifndef DEMO_PLAYER_SYSTEM_HPP_
#define DEMO_PLAYER_SYSTEM_HPP_

#include <xyginext/ecs/System.hpp>

#include <array>

struct Input final
{
    sf::Uint16 mask = 0;
    sf::Int32 timestamp = 0;
};

struct Player final
{
    Input input;
    std::array<Input, 120u> history;
    std::size_t currentInput = 0;
    sf::Uint8 playerNumber = 0;
};

class PlayerSystem final : public xy::System
{
public:
    explicit PlayerSystem(xy::MessageBus&);

    void process(float) override;

    void reconcile(float, float, sf::Int32, xy::Entity);

private:

    sf::Vector2f parseInput(sf::Uint16);
};

#endif //DEMO_PLAYER_SYSTEM_HPP_