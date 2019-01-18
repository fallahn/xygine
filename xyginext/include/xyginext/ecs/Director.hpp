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

#pragma once

#include "xyginext/Config.hpp"
#include "xyginext/core/MessageBus.hpp"

namespace xy
{
    class Message;
    class MessageBus;
    struct Command;
    class CommandSystem;
    class Scene;

    /*!
    \brief Abstract Base Class for Directors.
    A director is an 'observer' of the scene to which it belongs,
    and has the ability to command, or direct entities within it.
    While Directors do not exist within the world, they can receive
    both external events, such as player input, and system messages
    emitted from the scene. Based on this input the Director can then
    use a CommandSystem (one of which is automatically added to the
    scene when a Director is first added, if it does not exist) to 
    send commands to entities within the scene. For example a Director
    may take player input, process it then send movement commands to a
    player entity. It may also be used to implement gameplay rules by
    using the events it has observed to decide when new enemies should
    be spawned, where a player should restart and so on.
    */
    class XY_EXPORT_API Director
    {
    public:
        Director();
        virtual ~Director() = default;

    protected:
        /*!
        \brief Implement to handle system messages
        */
        virtual void handleMessage(const Message&) = 0;

        /*!
        \brief Implement to handle Events
        */
        virtual void handleEvent(const sf::Event&) {}

        /*!
        \brief Implement to process time based data
        */
        virtual void process(float) {}

        /*
        \brief Places a message on the system wide MessageBus
        */
        template <typename T>
        T* postMessage(Message::ID id);

        /*!
        \brief Sends a Command to the Scene
        \see CommandSystem
        */
        void sendCommand(const Command&);

        /*!
        \brief Returns a reference to this Director's parent Scene
        */
        Scene& getScene();

    private:

        MessageBus* m_messageBus;
        CommandSystem* m_commandSystem;
        Scene* m_scene;

        friend class Scene;
    };

    template <typename T>
    T* Director::postMessage(Message::ID id)
    {
        XY_ASSERT(m_messageBus, "Not yet initialised - are you using this properly?");
        return m_messageBus->post<T>(id);
    }
}
