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

#ifndef DEMO_GAME_PACKET_IDS_HPP_
#define DEMO_GAME_PACKET_IDS_HPP_

namespace PacketID
{
    enum
    {
        MapJoin = 0,
        MapChange, //map is changed when client already connected
        MapReady, //client has loaded new map and is ready to continue
        GameOver, //game ended
        ClientData, //this has been assigned by the server to the client
        ActorAbsolute, //applied to actors immediately rather than interpolated
        ActorUpdate, //interpolated position
        ActorEvent, //spawned, died, fire weapon etc
        ClientReady, //notify the server we're connected, loaded and ready
        ClientInput, //input from the client
        ClientUpdate, //server's authorative position of a client and input timestamp for reconciliation
        ClientDisconnected, //copy of client data who disconnected
        ServerMessage, //ident of a message the server wants to print to client console
        ConsoleMessage, //as server message only contains an arbitrary string (prefer ServerMessage where possible)
        InventoryUpdate, //tells clients to update player inventory display (lives etc)
        RoundWarning, //notifies clients of near time limit
        RoundSkip, //notifies clients someone collected BONUS
        LevelUpdate, //tells client which level they are on
        GameComplete, //signals client game is complete
        CollisionFlag, //and actor wit hthe given ID should have its client side collison flags updated
        ClientContinue, //player with ent ID requested continue
        RequestServerPause, //client request server pause game - 0 to pause, 1 to unpause
        RequestClientPause, //server paused, request client to display pause state - 0 to pause, 1 to unpause
        ServerFull,

        DebugMapCount, //sends the number of active NPCs for debug printing

        Count
    };
}
#endif //DEMO_GAME_PACKET_IDS_HPP_