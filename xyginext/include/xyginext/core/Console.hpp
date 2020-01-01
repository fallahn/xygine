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
#include <SFML/Config.hpp>

#include <string>
#include <functional>
#include <string>

namespace xy
{
    class ConsoleClient;
    class GuiClient;
    class ConfigObject;
    
    /*!
    \brief Console class.
    The console class provides a feedback window and interface
    with xygine. Any class can register a command with the
    console as long as it inherits the ConsoleClient interface.

    The Console functions are not threadsafe - appropriate care
    should be taken when using the Console from multiple threads.
    */
    class XY_EXPORT_API Console final
    {
    public:
        /*!
        Command functions are registered with an associated
        string, and executed when that string is entered into the
        consle window. Any further characters entered after the
        command are passed into the command function as a parameter.
        If this is a list of parameters it is up to the command
        function implementation to properly parse the given string.
        Registering a command with the console may be done by any
        class which inherits ConsoleClient.
        \see ConsoleClient
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
        \brief Returns true if the console is currently shown
        */
        static bool isVisible();

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
        when searching for variable names.
        */
        static void addConvar(const std::string& name, const std::string& value, const std::string& helpText = std::string());

        /*!
        \brief Attempts to retrieve the current value of the given console variable
        */
        template <typename T>
        static T getConvarValue(const std::string& convar);
        
        /*!
        \brief Returns a reference to the ConfigObject containing any existing convars.
        The returned reference is const, use addConvar() to write to the ConfigObject
        */
        const ConfigObject& getConvars();

        /*!
         \brief Prints the name/value pair to the stats window
         */
        static void printStat(const std::string&, const std::string&);

    private:
        friend class App;
        friend class ConsoleClient;

        static void init();
        static void finalise();
        
        static void addStatusControl(const std::function<void()>&, const GuiClient*);
        static void removeStatusControls(const GuiClient*);

        static void addConsoleTab(const std::string&, const std::function<void()>&, const GuiClient*);
        static void removeConsoleTab(const GuiClient*);

        static void addCommand(const std::string& name, const Command& cmd, const ConsoleClient* owner);
        static void removeCommands(const ConsoleClient*); //removes all commands belonging to the given client
        
        static void draw();
    };
}
