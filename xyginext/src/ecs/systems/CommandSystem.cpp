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

#include <xyginext/ecs/components/CommandTarget.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>

using namespace xy;

namespace
{
    const std::size_t MaxCommands = 128; //this can be made larger if necessary, is only used to prevent continual reallocation of heap memory
}

CommandSystem::CommandSystem(MessageBus& mb)
    : System        (mb, typeid(CommandSystem)),
    m_commands      (MaxCommands),
    m_currentCommand(m_commands.begin()),
    m_count         (0)
{
    requireComponent<CommandTarget>();
}

//public
void CommandSystem::sendCommand(const Command& cmd)
{
    if (m_currentCommand == m_commands.end()) return;

    *m_currentCommand = cmd;
    m_currentCommand++;
    m_count++;
    //XY_ASSERT(m_currentCommand != m_commands.end(), "Exceeded max commands!");
}

void CommandSystem::process(float dt)
{
    auto& entities = getEntities();
    m_currentCommand = m_commands.begin();

    for (auto i = 0u; i < m_count; ++i, ++m_currentCommand)
    {
        for (auto& e : entities)
        {
            if (e.getComponent<CommandTarget>().ID & m_currentCommand->targetFlags)
            {
                m_currentCommand->action(e, dt);
            }
        }
    }

    m_currentCommand = m_commands.begin();
    m_count = 0;
}