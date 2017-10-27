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

#ifndef DEMO_INVENTORY_DIRECTOR_HPP_
#define DEMO_INVENTORY_DIRECTOR_HPP_

#include <xyginext/ecs/Director.hpp>

#include <array>

namespace xy
{
    class NetHost;
}

class InventoryDirector final : public xy::Director
{
public:
    explicit InventoryDirector(xy::NetHost&);

    void handleMessage(const xy::Message&) override;
    void handleEvent(const sf::Event&) override {}
    void process(float) override;

private:
    xy::NetHost& m_host;
    
    struct Inventory final
    {
        sf::Uint32 score = 0;
        sf::Uint8 lives = 0;
        sf::Uint8 bonusFlags = 0;
    };
    std::array<Inventory, 2> m_playerValues{};

    std::array<std::pair<sf::Uint8, sf::Uint32>, 12> m_updateQueue;
    std::size_t m_queuePos;

    void sendUpdate(sf::Uint8, sf::Uint32);

    void checkLifeBonus(sf::Uint8, sf::Uint32);
};

#endif //DEMO_INVENTORY_DIRECTOR_HPP_