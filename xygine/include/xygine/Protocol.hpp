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

//spec for network protocol

#ifndef XY_NET_PROTOCOL_HPP_
#define XY_NET_PROTOCOL_HPP_

#include <SFML/Config.hpp>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    namespace Client
    {
        //packet IDs from client
        enum PacketId
        {
            RequestClientID = 1,
            RequestWorldState,
            StartReady,
            InputUpdate,
            LobbyData,
            Quit,
            Ping /*int16 UID, float original time*/
        };
    }

    namespace Server
    {
        struct EntityState
        {
            sf::Int16 uid;
            sf::Vector2f position;
        };

        //packet IDs from server
        enum PacketId
        {
            ClientID = 1,
            InitialState, /*Uint32 playercount, states, Uint32 alienCount, states, UInt32 humancount, states*/
            LobbyData, /*Uint32 count, Int16 uid - TODO add name and ready status*/
            PlayerConnect,
            PlayerDisconnect,
            UpdateClientState, /*Int32 server time, Int32 size, State entityState*/
            UpdatePlayerState, /*Int16 Id, PlayerInfo*/
            GameEvent, /*Int16 id, Int32 event type*/
            GameStart,
            GameOver,
            Ping /*float current server time*/
        };
    }

    namespace GameEvent
    {
        enum Type
        {

        };
    }
}
#endif //XY_NET_PROTOCOL_HPP_