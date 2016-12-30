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

//message bus to allow inter-component communication

#ifndef XY_MESSAGE_BUS_HPP_
#define XY_MESSAGE_BUS_HPP_

#define TEMPLATE_GUARD template <typename T>//, typename std::enable_if<std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value>::type...>
#ifdef __GNUC__ //GCC < 5 doesn't support these type traits
#if __GNUC__ < 5
#undef TEMPLATE_GUARD
#define TEMPLATE_GUARD template <typename T>
#endif //__GNUC__ ver
#endif //__GNUC__

#include <xygine/State.hpp>
#include <xygine/Difficulty.hpp>
#include <xygine/Assert.hpp>
#include <xygine/network/Config.hpp>

#include <SFML/Config.hpp>

#include <vector>
#include <type_traits>

class b2Joint;
class b2Fixture;

using UIControlID = sf::Int32;

namespace xy
{
    namespace Physics
    {
        class Joint;
        class CollisionShape;
        class RigidBody;
        class Contact;
    }

    class Component;
    class Entity;
    
    /*!
    \brief Message class

    The message class contains an ID used to identify
    the type of data contained in the message, and the
    message data itself. xygine uses some internal messaging
    types, so custom messages should have their IDs start at
    Message::Type::Count
    \see MessageBus
    */
    class XY_EXPORT_API Message final
    {
        friend class MessageBus;
    public:
        using ID = sf::Int32;
        enum Type
        {
            AudioMessage = 0,
            PhysicsMessage,
            EntityMessage,
            UIMessage,
            ComponentSystemMessage,
            NetworkMessage,
            SceneMessage,
            Count
        };
        /*!
        \brief Audio event message data
        */
        struct AudioEvent
        {
            sf::Uint64 entityID = 0;
            enum
            {
                Play,
                Pause,
                Stop
            }action;
        };
        /*!
        \brief Physics event message data
        */
        struct PhysicsEvent
        {
            enum
            {
                BeginContact,
                EndContact,
                JointDestroyed,
                CollisionShapeDestroyed,
                RigidBodyDestroyed
            }event;

            union
            {
                const Physics::Joint* joint;
                const Physics::CollisionShape* collisionShape;
                const Physics::RigidBody* rigidBody;
                const Physics::Contact* contact;
            };
        };
        /*!
        \brief Entity event message data
        */
        struct EntityEvent
        {
            enum
            {
                Destroyed,
                AddedToScene
            }action;
            Entity* entity = nullptr;
        };
        /*!
        \brief UI event message data
        */
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
                ResizedWindow,
                SelectionActivated,
                SelectionChanged
            }type;
            float value = 0.f;
            StateID stateID = -1;
            Difficulty difficulty;
            UIControlID controlID = -1;
        };
        /*!
        \brief Component event message data
        */
        struct ComponentEvent
        {
            enum
            {
                Deleted
            }action;
            sf::Uint64 entityID = 0;
            Component* ptr = nullptr;
        };
        /*!
        \brief Network connection message data
        */
        struct NetworkEvent
        {
            enum
            {
                ConnectionAdded,
                ConnectionRemoved
            }action;
            ClientID clientID = -1;
        };

        /*!
        \brief System events which occur in the scene class
        */
        struct SceneEvent
        {
            enum
            {
                CameraChanged
            }action;
        };

        ID id = -1;

        /*!
        \brief Returns the actual data containend in the message

        Using the ID of the message to determine the data type of the
        message, this function will return a reference to that data.
        It is important to request the correct type of data from the
        message else behaviour will be undefined.

        if(msg.id == Type::PhysicsMessage)
        {
            const PhysicsEvent& data = msg.getData<PhysicsEvent>();
            //do stuff with data
        }


        */
        TEMPLATE_GUARD
        const T& getData() const
        {
            //this isn't working on MSVC
            //static_assert(std::is_trivially_constructible<T>::value && std::is_trivially_destructible<T>::value, "");
            XY_ASSERT(sizeof(T) == m_dataSize, "size of supplied type is not equal to data size");
            return *static_cast<T*>(m_data);
        }

    private:
        void* m_data;
        std::size_t m_dataSize;
    };

    /*!
    \brief System wide message bus for custom event messaging

    The app class contains an instance of the MessageBus which allows
    events and messages to be broadcast system wide, upon which objects
    entities and components can decide whether or not to act. Custom
    message type IDs can be added by extending the ID set starting
    at Message::Type::Count. EG

    enum MyMessageTypes
    {
        AlienEvent = Message::Type::Count,
        GhostEvent,
        BadgerEvent //etc...
    };
    */
    class XY_EXPORT_API MessageBus final
    {
    public:
        MessageBus();
        ~MessageBus() = default;
        MessageBus(const MessageBus&) = delete;
        const MessageBus& operator = (const MessageBus&) = delete;

        /*!
        \brief Read and despatch all messages on the message stack

        Used internally by xygine
        */
        const Message& poll();
        /*!
        \brief Places a message on the message stack, and returns a pointer to the data
        
        The message data can then be filled in via the pointer. Custom message types can
        be defined via structs, which are then created on the message bus. Structs should
        contain only trivial data such as PODs and pointers to other objects.
        ATTEMPING TO PLACE LARGE OBJECTS DIRECTLY ON THE MESSAGE BUS IS ASKING FOR TROUBLE
        Custom message types should have a unique 32 bit integer ID which can be used
        to identify the message type when reading messages. Message data has a maximum
        size of 128 bytes.
        \param id Unique ID for this message type
        \returns Pointer to an empty message of given type.
        */
        TEMPLATE_GUARD
        T* post(Message::ID id)
        {
            if (!m_enabled) return static_cast<T*>((void*)m_pendingBuffer.data());

            auto dataSize = sizeof(T);
            static auto msgSize = sizeof(Message);
            XY_ASSERT(dataSize < 128, "message size exceeds 128 bytes"); //limit custom data to 128 bytes
            XY_ASSERT(m_pendingBuffer.size() - (m_inPointer - m_pendingBuffer.data()) > (dataSize + msgSize), "buffer overflow " + std::to_string(m_pendingCount)); //make sure we have enough room in the buffer
            XY_WARNING(m_pendingBuffer.size() - (m_inPointer - m_pendingBuffer.data()) < 128, "Messagebus buffer is heavily contended!");

            Message* msg = new (m_inPointer)Message();
            m_inPointer += msgSize;
            msg->id = id;
            msg->m_dataSize = dataSize;
            msg->m_data = new (m_inPointer)T();
            m_inPointer += dataSize;

            m_pendingCount++;
            return static_cast<T*>(msg->m_data);
        }

        /*!
        \brief Returns true if there are no messages left on the message bus
        */
        bool empty();
        /*!
        \brief Returns the number of messages currently sitting on the message bus

        Useful for stat logging and debugging.
        */
        std::size_t pendingMessageCount() const;

        /*!
        \brief Disables the message bus.
        Used internally by xygine
        */
        void disable() { m_enabled = false; }

    private:

        std::vector<char> m_currentBuffer;
        std::vector<char> m_pendingBuffer;
        char* m_inPointer;
        char* m_outPointer;
        std::size_t m_currentCount;
        std::size_t m_pendingCount;

        bool m_enabled;
    };
}
#endif //XY_MESSAGE_BUS_HPP_
