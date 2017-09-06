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

#ifndef XY_CONSOLE_HPP_
#define XY_CONSOLE_HPP_

#include <xyginext/Config.hpp>
#include <SFML/Config.hpp>

#include <functional>

namespace xy
{
    class ConsoleClient;
    
    /*!
    \brief Console class.
    The console class provides a feedback window and interface
    with xygine. Any class can register a command with the
    console as long as it inherits the ConsoleClient interface.
    */
    class XY_EXPORT_API Console final
    {
    public:
        /*!
        Command functions are registered with their associated
        string, and executed when that string is entered into the
        consle window. Any further characters entered after the
        command are passed into the command function as a paramter.
        If this is a list of paramters it is up to the command
        function implementation to properly parse the given string.
        */
        using Command = std::function<void(const std::string&)>;

        /*!
        \brief Prints the given string to the console window
        */
        static void print(const std::string&);

        /*!
        \brief Toggles the console window visibility
        */
        static void show();

        /*!
        \brief Executes the given command line.
        Allows for programatically executing arbitrary commands
        */
        static void doCommand(const std::string&);

        /*!
        \brief Adds a convar it if it doesn't exist.
        \param name Name of the variable as it appears in the console.
        \param value the default value of the variable
        \param helpText Optional string describing the variable which appears the console
        when search for variable names.
        */
        static void addConvar(const std::string& name, const std::string& value, const std::string& helpText = "");

        /*!
        \brief Attempts to retrieve the current value of the given console variable
        */
        template <typename T>
        static T getConvarValue(const std::string& convar);

    private:
        friend class App;
        friend class ConsoleClient;

        static void init();
        static void finalise();

        static void addCommand(const std::string& name, const Command& cmd, const ConsoleClient* owner);
        static void removeCommands(const ConsoleClient*); //removes all commands belonging to the given client

        static void draw();
    };

//#include "Console.inl"

}

#endif //XY_CONSOLE_HPP_
