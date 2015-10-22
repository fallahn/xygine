/*********************************************************************
Matt Marchant 2014 - 2015
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

//message bus to allow inter-component communication

#ifndef MESSAGE_BUS_HPP_
#define MESSAGE_BUS_HPP_

#include <xygine/State.hpp>
#include <xygine/Difficulty.hpp>

#include <SFML/Config.hpp>

#include <queue>

namespace xy
{
    class Component;
    class Entity;

    class Message final
    {
    public:
        enum class Type
        {
            Audio = 1,
            Physics,
            Entity,
            UI,
            Player,
            ComponentSystem,
            Network
        }type;

        struct AudioEvent
        {
            sf::Uint64 entityId;
        };

        struct PhysicsEvent
        {
            enum
            {
                Collision
            }event;

            sf::Uint64 entityId[2];
        };

        struct EntityEvent
        {
            enum
            {
                Destroyed,
                ChangedDirection
            }action;
            Entity* entity;
            sf::Int32 direction;
        };

        struct UIEvent
        {
            enum
            {
                RequestVolumeChange,
                RequestAudioMute,
                RequestAudioUnmute,
                MenuOpened,
                MenuClosed,
                RequestState,
                RequestDifficultyChange,
                RequestControllerEnable,
                RequestControllerDisable,
                ResizedWindow
            }type;
            float value;
            StateId stateId;
            Difficulty difficulty;
        };

        struct PlayerEvent
        {
            enum
            {
                Died,
                Spawned
            }action;
            sf::Int32 timestamp;
            sf::Uint64 entityID;
        };

        struct ComponentEvent
        {
            enum
            {
                Deleted
            }action;
            sf::Uint64 entityId;
            Component* ptr;
        };

        struct NetworkEvent
        {
            enum
            {
                RequestStartServer,
                RequestJoinServer,
                RequestDisconnect,
                RequestWorldState,
                ConnectSuccess,
                ConnectFail,
                StartReady
            }action;
            StateId stateID;
        };

        union
        {
            AudioEvent audio;
            PhysicsEvent physics;
            EntityEvent entity;
            UIEvent ui;
            PlayerEvent player;
            ComponentEvent component;
            NetworkEvent network;
        };
    };

    class MessageBus final
    {
    public:
        MessageBus() = default;
        ~MessageBus() = default;
        MessageBus(const MessageBus&) = delete;
        const MessageBus& operator = (const MessageBus&) = delete;

        //read and despatch all messages on the message stack
        Message poll();
        //places a message on the message stack
        void post(const Message& msg);

        bool empty();

        std::size_t pendingMessageCount() const; //used for stats

    private:
        std::queue<Message> m_currentMessages;
        std::queue<Message> m_pendingMessages;
    };
}
#endif