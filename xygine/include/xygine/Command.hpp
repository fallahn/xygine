/*********************************************************************
© Matt Marchant 2014 - 2017
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

//command class for targeting entities within a scene graph

#ifndef XY_COMMAND_HPP_
#define XY_COMMAND_HPP_

#include <xygine/Config.hpp>

#include <SFML/Config.hpp>

#include <functional>
#include <queue>

namespace xy
{
    class Entity;
    /*!
    \brief Command struct
    */
    struct XY_EXPORT_API Command final
    {
        std::function<void(Entity&, float)> action; //< Action to be performed by the command on the targeted entity
        sf::Uint64 entityID = 0u; //< UID of the target entity. Overrides any target categories
        using Category = sf::Int32;
        enum
        {
            None = 0,
            All = (1 << 31)
        };
        Category category = None; //< Category of entities to target. Categories are user defined and should be bitwise flag values
    };

    /*!
    \brief Queue of commands to be executed by Entities in a scene

    Each from the scene owning the CommandQueue will execute the entire
    queue of commands on the targeted entities, should they exist.
    \see Scene
    */
    class XY_EXPORT_API CommandQueue final
    {
    public:
        CommandQueue() = default;
        ~CommandQueue() = default;
        CommandQueue(const CommandQueue&) = delete;
        CommandQueue& operator = (const CommandQueue&) = delete;

        /*!
        \brief Pushes a Command on to the queue
        */
        void push(const Command&);
        /*!
        \brief Pops a command from the queue
        */
        Command pop();
        /*!
        \brief Returns true if the queue is empty
        */
        bool empty() const;
        /*!
        \brief Returns the current size of the queue

        Useful for debugging, outputting via Reporter etc
        */
        std::size_t size() const { return m_queue.size(); }

    private:
        std::queue<Command> m_queue;
    };
}
#endif //XY_COMMAND_HPP_