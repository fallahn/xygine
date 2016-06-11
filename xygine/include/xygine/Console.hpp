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

#ifndef XY_CONSOLE_HPP_
#define XY_CONSOLE_HPP_

#include <xygine/Config.hpp>

#include <string>
#include <functional>

namespace xy
{
    /*!
    \brief Console class
    The console is used to display messages and xygine specific information
    as well as allow input of commands registered with the console class
    */
    class XY_EXPORT_API Console final
    {
        friend class App;
    public:
        using Command = std::function<void(const std::string&)>;
        
        /*!
        \brief Prints the given string to the console
        */
        static void print(const std::string&);

        /*!
        \brief Draws ths console.
        This must be called every frame you wish the console to be visible
        */
        static void show();

        /*!
        \brief Adds an executable command.
        \param name String to enter to execute this command
        \param command instance of a std::function matching the Console::Command
        signature. The string parameter for the command should contain any
        optional paramters used by the command. xy::Util::String can be used to
        aid string management such as splitting multiple parameters inside the command
        \param owner Optional pointer to an object registering a command which can
        be used to unregister a command should that object no longer exist. See
        unregisterCommands()
        */
        static void addCommand(const std::string& name, const Command& command, void* owner = nullptr);

        /*!
        \brief Unregisters any commands registered by the objected pointed to by the
        given parameter.
        For example: should a command be registered by an object which captures local
        varibles via a lambda, then those variables will no longer exist when the
        registering objected is destroyed. This will lead to undefined behaviour
        when calling such a command. To help negate this commands can be registered
        with a pointer to the registering object:
        addCommand("close", myCommand, (void*)this);
        Then, when the registering object is being destroyed, via its destructor a
        call to unregisterCommands((void*)this) will unregister all commands from
        the console, so that trying to execute them will result in defined (non)
        behaviour.
        */
        static void unregisterCommands(void*);

        /*!
        \brief Executes the given command line.
        Allows programatically executing an arbitrary console command.
        */
        static void doCommand(const std::string&);

    private:
        static void draw();
        static void registerDefaultCommands();
        
    };
}

#endif //XY_CONSOLE_HPP_