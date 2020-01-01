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

#include "xyginext/Config.hpp"

#include <functional>
#include <string>

namespace xy
{
    /*!
    \brief Interface for registering console commands.
    To register a command with the console a class needs
    to inherit this interface, which ensures proper lifetime
    of console commands which refer to specific instances of
    an object.
    */
    class XY_EXPORT_API ConsoleClient
    {
    public:
        ConsoleClient() = default;
        virtual ~ConsoleClient();
        ConsoleClient(const ConsoleClient&) = default;
        ConsoleClient(ConsoleClient&&) = default;
        ConsoleClient& operator = (const ConsoleClient&) = default;
        ConsoleClient& operator = (ConsoleClient&&) = default;

        /*!
        \brief Registers a command with the console.
        \param command A string containing the name of the command. Must NOT contain spaces
        \param func A Console::Command containing the function to execute when the command is received
        */
        void registerCommand(const std::string& command, const std::function<void(const std::string&)>& func);
    };
}
