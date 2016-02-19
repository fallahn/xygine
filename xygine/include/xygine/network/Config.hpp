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

#ifndef XY_NETWORK_CONFIG_HPP_
#define XY_NETWORK_CONFIG_HPP_

#include <SFML/Config.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>

#include <unordered_map>
#include <functional>

namespace xy
{
    namespace Network
    {
        //need to enforce sending IDs as a single byte
        enum PacketType
        {
            Disconnect = -1,
            Connect,
            HeartBeat,
            ServerFull,
            ClientLeft, //<followed by client ID
            ClientJoined, //<followed by client ID
            Count
        };

        enum
        {
            HighestTimestamp = 2147483647,
            ServerPort = 20715,
            NullID = -1
        };

        static const sf::Uint32 PROTOCOL_ID = 0x444C4142u;
    }

    using PacketID = sf::Int8;
    using ClientID = sf::Int32;
    using PortNumber = sf::Uint16;
}
#endif //XY_NETWORK_CONFIG_HPP_
