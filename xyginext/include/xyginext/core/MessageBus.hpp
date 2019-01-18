/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

//message bus to allow inter-component communication

#pragma once

#include "xyginext/core/State.hpp"
#include "xyginext/core/Assert.hpp"
#include "xyginext/core/Message.hpp"

#include <vector>
#include <type_traits>

namespace xy
{
    static constexpr std::size_t MessageSize = sizeof(Message);

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
        MessageBus(MessageBus&&) = delete;
        MessageBus& operator = (const MessageBus&) = delete;
        MessageBus& operator = (MessageBus&&) = delete;

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
        template <typename T>
        T* post(Message::ID id)
        {
            if (!m_enabled) return static_cast<T*>((void*)m_pendingBuffer.data());

            auto dataSize = sizeof(T);

            XY_ASSERT(dataSize < 128, "message size exceeds 128 bytes"); //limit custom data to 128 bytes
            XY_ASSERT(m_pendingBuffer.size() - (m_inPointer - m_pendingBuffer.data()) > (dataSize + MessageSize), "buffer overflow " + std::to_string(m_pendingCount)); //make sure we have enough room in the buffer
            XY_WARNING(m_pendingBuffer.size() - (m_inPointer - m_pendingBuffer.data()) < 128, "Messagebus buffer is heavily contended!");

            Message* msg = new (m_inPointer)Message();
            m_inPointer += MessageSize;
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
