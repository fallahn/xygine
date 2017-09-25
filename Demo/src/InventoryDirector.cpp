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

#include "InventoryDirector.hpp"
#include "MessageIDs.hpp"
#include "PacketIDs.hpp"
#include "MapData.hpp"

#include <xyginext/network/NetHost.hpp>

namespace
{
    const sf::Uint32 NpcScore = 50;
    const sf::Uint32 SmallFruitScore = 35;
    const sf::Uint32 LargeFruitScore = 100;
}

InventoryDirector::InventoryDirector(xy::NetHost& host)
    : m_host(host)
{

}

//public
void InventoryDirector::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    default: break;
    case MessageID::NpcMessage:
    {
        const auto& data = msg.getData<NpcEvent>();
        if (data.type == NpcEvent::Died)
        {
            m_playerValues[data.playerID].score += NpcScore;
        }
        sendUpdate(data.playerID);
    }
        break;
    case MessageID::ItemMessage:
    {
        const auto& data = msg.getData<ItemEvent>();
        switch (data.actorID)
        {
        default: break;
        case ActorID::FruitLarge:

            break;
        case ActorID::FruitSmall:
            m_playerValues[data.playerID].score += SmallFruitScore;
            break;
        }

        sendUpdate(data.playerID);
    }
        break;
    case MessageID::PlayerMessage:
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::Spawned)
        {
            //reset the old scores
            auto id = data.entity.getComponent<Player>().playerNumber;
            m_playerValues[id].lives = 3;
            m_playerValues[id].score = 0;

            sendUpdate(id);
        }
    }
        break;
    }
}

//private
void InventoryDirector::sendUpdate(sf::Uint8 player)
{
        InventoryUpdate update;
        update.lives = m_playerValues[player].lives;
        update.score = m_playerValues[player].score;
        update.playerID = player;

        m_host.broadcastPacket(PacketID::InventoryUpdate, update, xy::NetFlag::Reliable, 1);
}