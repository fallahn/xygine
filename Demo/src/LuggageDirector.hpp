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

#ifndef DEMO_LUGGAGE_DIRECTOR_HPP_
#define DEMO_LUGGAGE_DIRECTOR_HPP_

#include <xyginext/ecs/Director.hpp>

struct Luggage final
{
    bool enabled = false;
    sf::Uint32 entityID = 0;

    enum
    {
        PlayerOne = 0x1,
        PlayerTwo = 0x2,
        PickedUp = 0x4,
        Dropped = 0x8,
        Normal = 0x10,
        Explosive = 0x20
    };
};

namespace xy
{
    class NetHost;
}

class LuggageDirector final : public xy::Director
{
public:
    explicit LuggageDirector(xy::NetHost&);

    void handleEvent(const sf::Event&) override {}
    void handleMessage(const xy::Message&) override;
    void process(float) override {}

private:
    xy::NetHost& m_host;
};

#endif //DEMO_LUGGAGE_DIRECTOR_HPP_