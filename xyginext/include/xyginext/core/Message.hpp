/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#pragma once

#include "xyginext/core/Assert.hpp"

#include <SFML/Config.hpp>

namespace xy
{
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
            UIMessage,
            WindowMessage,
            SceneMessage,
            StateMessage,
            Count
        };

        ID id = -1;

        /*!
        \brief Events raised by the AudioSystem
        */
        struct AudioEvent final
        {
            enum
            {
                ChannelVolumeChanged
            }type;
        };

        /*!
        \brief Events raised by the RenderWindow
        */
        struct WindowEvent final
        {
            enum
            {
                Resized,
                LostFocus,
                GainedFocus
            }type;

            sf::Uint32 width, height;
        };

        /*!
        \brief Events raised by the Scecne class
        */
        struct SceneEvent final
        {
            enum
            {
                EntityDestroyed
            }event;
            sf::Int32 entityID = -1;
        };

        /*!
        \brief Events raised by the StateStack
        */
        struct StateEvent final
        {
            enum
            {
                Pushed, Popped, Cleared
            }type;
            std::int32_t id = -1;
        };

        /*!
        \brief Returns the actual data contained in the message

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
        template <typename T>
        const T& getData() const
        {
            XY_ASSERT(sizeof(T) == m_dataSize, "size of supplied type is not equal to data size");
            return *static_cast<T*>(m_data);
        }

    private:
        void* m_data = nullptr;
        std::size_t m_dataSize = 0;
    };
}
