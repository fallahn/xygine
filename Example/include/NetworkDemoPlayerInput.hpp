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

#ifndef NET_PLAYER_INPUT_HPP_
#define NET_PLAYER_INPUT_HPP_

#include <SFML/Config.hpp>
#include <xygine/network/Config.hpp>

namespace NetDemo
{
    struct Input final
    {
        float position = 0; //<position of mouse Y axis
        sf::Uint64 counter = 0; //<input counter for ordering
        xy::ClientID clientID = -1;
        sf::Int32 timestamp = 0; //<server time as client knows it in ms
    };
}
sf::Packet& operator <<(sf::Packet& packet, const NetDemo::Input&);

sf::Packet& operator >>(sf::Packet& packet, NetDemo::Input&);

#endif //NET_PLAYER_INPUT_HPP_