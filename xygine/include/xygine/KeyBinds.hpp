/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_KEYBINDS_HPP_
#define XY_KEYBINDS_HPP_

#include <xygine/Config.hpp>

#include <SFML/Config.hpp>

#include <vector>
#include <string>

namespace
{
    const std::string defaultPath = "input.kbd";
}

namespace xy
{
    /*!
    \brief Allows binding keyboard and joystick buttons to specific actions.
    */
    class XY_EXPORT_API Input final
    {
    public:

        enum
        {
            Unbound = -2
        };

        enum
        {
            Up = 0,
            Down,
            Left,
            Right,
            Start,
            Back,
            ActionOne,
            ActionTwo,
            ActionThree,
            ActionFour,
            KeyCount
        };

        enum
        {
            ButtonA,
            ButtonB,
            ButtonX,
            ButtonY,
            ButtonLB,
            ButtonRB,
            ButtonBack,
            ButtonStart,
            ButtonCount
        };
    
        /*!
        \brief Binds a given key via its enum value to an action.
        If an action needs to be unbound then use xy::Input::Unbound
        \param key Enum value according to sf::Keyboard::Key
        \param action Enum value according to xy::Input
        */
        static void bindKey(sf::Int32 key, sf::Int32 action);
        /*!
        \brief Binds an alternate key to an action.
        \see bindKey()
        */
        static void bindAltKey(sf::Int32 key, sf::Int32 action);
        /*!
        \brief Binds the value of sf::JoyStick::Button to xy::Input value.
        By default these are designed to match the xbox360 controller on Windows
        */
        static void bindJoyButton(sf::Int32 button, sf::Int32 action);

        /*!
        \brief Returns the enum value of sf::Keyboard::Key bound to the
        given action, or xy::Input::Unbound if it is not found
        */
        static sf::Int32 getKey(sf::Int32 action);
        /*!
        \brief Returns the enum value of alternate sf::Keyboard::Key bound to the
        given action, or xy::Input::Unbound if it is not found
        */
        static sf::Int32 getAltKey(sf::Int32 action);
        /*!
        \brief Returns a std::pair of sf::Keyboard::Key values bound to the given action.
        The first value contains the primary binding, and second value the alternate
        binding. Either of these bindings may have the value of xy::Input::Unbound
        */
        static std::pair<sf::Int32, sf::Int32> getKeys(sf::Int32 action);
        /*!
        \brief Returns the enum value of sf::JoyStick::Button bound to the
        given action, or xy::Input::Unbound if it is unbound.
        */
        static sf::Int32 getJoyButton(sf::Int32 action);
        /*!
        \brief Returns a string representation of the given sf::Keyboard::Key enum value
        */
        static std::string getKeyAsString(sf::Int32);

        /*!
        \brief Extends the number of keybinds availble.
        By default the number of keybinds available is Input::KeyCount. To
        create additional key bindings create a new enumeration for your values

        enum MyKeybinds
        {
            SpecialKeyOne = xy::Input::KeyCount,
            SpecialKeyTwo
            //etc..
        };

        Note that the first member has the value of Input::KeyCount. Then call
        this function ONCE to resize the the internal keybind list, passing the
        new maximum size which ought to be the value of the last custom key + 1

        Input::extendKeyBinds(MyKeyBinds::SpecialKeyTwo + 1);
        */
        static void extendKeyBinds(sf::Int32 newSize);

        /*!
        \brief Extends the number of joystick buttons availble.
        By default the number of button available is Input::ButtonCount. To
        create additional button bindings create a new enumeration for your values

        enum MyJoybinds
        {
        SpecialButtonOne = xy::Input::ButtonCount,
        SpecialButtonTwo
        //etc..
        };

        Note that the first member has the value of Input::ButtonCount. Then call
        this function ONCE to resize the the internal joystick button list, passing the
        new maximum size which ought to be the value of the last custom value + 1

        Input::extendJoyBinds(MyJoyBinds::SpecialButtonTwo + 1);
        */
        static void extendJoyBinds(sf::Int32 newSize);

        /*!
        \brief Saves the current keybinds to a file with the given name
        */
        static void save(const std::string& path = defaultPath);

        /*!
        \brief Attempts to load keybinds from the given file path.
        \returns false if loading failed
        */
        static bool load(const std::string& path = defaultPath);

    private:

        static std::vector<std::pair<sf::Int32, sf::Int32>> keybinds;
        static std::vector<sf::Int32> joybinds;

        static void bindKeyToDest(sf::Int32, sf::Int32&);
    };
}


#endif //XY_KEYBINDS_HPP_