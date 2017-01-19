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

#include <xygine/KeyBinds.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

#include <fstream>
#include <cstring>

using namespace xy;

namespace
{
    const std::vector<std::string> keyStrings =
    {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        "Num0", "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9", "Escape", "LControl", "LShift", "LAlt", "LSystem",
        "RControl", "RShift", "RAlt", "RSystem", "Menu", "[", "]", ";", ",", ".", "'", "/", "\\", "~", "=", "-", "Space", "Return", "BackSpace",
        "Tab", "PgUp", "PgDn", "End", "Home", "Insert", "Delete", "+", "-", "*", "/", "Left", "Right", "Up", "Down",
        "NumPad0", "NumPad1", "NumPad2", "NumPad3", "NumPad4", "NumPad5", "NumPad6", "NumPad7", "NumPad8", "NumPad9",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "Pause"
    };
}

std::vector<std::pair<sf::Int32, sf::Int32>> Input::keybinds = 
{
    std::make_pair(sf::Keyboard::W, sf::Keyboard::Up),
    std::make_pair(sf::Keyboard::S, sf::Keyboard::Down),
    std::make_pair(sf::Keyboard::A, sf::Keyboard::Left),
    std::make_pair(sf::Keyboard::D, sf::Keyboard::Right),
    std::make_pair(sf::Keyboard::Return, Input::Unbound),
    std::make_pair(Input::Unbound, Input::Unbound),
    std::make_pair(sf::Keyboard::Space, Input::Unbound),
    std::make_pair(sf::Keyboard::LControl, sf::Keyboard::RControl),
    std::make_pair(Input::Unbound, Input::Unbound),
    std::make_pair(Input::Unbound, Input::Unbound)
};

std::vector<sf::Int32> Input::joybinds = 
{
    0, 1, 2, 3, 4, 5, 6, 7
};


void Input::bindKeyToDest(sf::Int32 key, sf::Int32& dest)
{
    std::find_if(std::begin(keybinds), std::end(keybinds),
        [key](std::pair<sf::Int32, sf::Int32>& pair)
    {
        if (pair.first == key) pair.first = Input::Unbound;
        else if (pair.second == key) pair.second = Input::Unbound;
        return false;
    });

    dest = key;
}


void Input::bindKey(sf::Int32 key, sf::Int32 action)
{
    XY_ASSERT(key >= Input::Unbound && key < sf::Keyboard::KeyCount, "Invalid key value");
    XY_ASSERT(action >= Input::Up && action < keybinds.size(), "Invalid action");
    bindKeyToDest(key, keybinds[action].first);
}

void Input::bindAltKey(sf::Int32 key, sf::Int32 action)
{
    XY_ASSERT(key >= Input::Unbound && key < sf::Keyboard::KeyCount, "Invalid key value");
    XY_ASSERT(action >= Input::Up && action < keybinds.size(), "Invalid action");
    bindKeyToDest(key, keybinds[action].second);
}

void Input::bindJoyButton(sf::Int32 button, sf::Int32 action)
{
    XY_ASSERT(button >= 0 && button < sf::Joystick::ButtonCount, "Invalid button value");
    XY_ASSERT(action >= ButtonA && action < joybinds.size(), "Invalid joystick action");
    std::find_if(std::begin(joybinds), std::end(joybinds), [button](sf::Int32& bind)
    {
        if (button == bind) bind = Unbound;
        return false;
    });
    joybinds[action] = button;
}

sf::Int32 Input::getKey(sf::Int32 action)
{
    XY_ASSERT(action >= Input::Up && action < keybinds.size(), "Invalid action");
    return keybinds[action].first;
}

sf::Int32 Input::getAltKey(sf::Int32 action)
{
    XY_ASSERT(action >= Input::Up && action < keybinds.size(), "Invalid action");
    return keybinds[action].second;
}

std::pair<sf::Int32, sf::Int32> Input::getKeys(sf::Int32 action)
{
    XY_ASSERT(action >= Input::Up && action < keybinds.size(), "Invalid action");
    return keybinds[action];
}

sf::Int32 Input::getJoyButton(sf::Int32 action)
{
    XY_ASSERT(action >= ButtonA && action < joybinds.size(), "Invalid joystick action");
    return joybinds[action];
}

std::string Input::getKeyAsString(sf::Int32 key)
{
    XY_ASSERT(keyStrings.size() == sf::Keyboard::KeyCount, "String representation missing...");
    return (key >= 0) ? keyStrings[key] : "";
}

void Input::extendKeyBinds(sf::Int32 newSize)
{
    XY_ASSERT(newSize > keybinds.size(), "Do not make this smaller!");

    auto oldSize = keybinds.size();
    keybinds.resize(newSize);

    for (auto i = oldSize; i < newSize; ++i)
    {
        keybinds[i] = std::make_pair(Unbound, Unbound);
    }
}

void Input::extendJoyBinds(sf::Int32 newSize)
{
    XY_ASSERT(newSize > joybinds.size(), "Do not make this smaller!");

    auto oldSize = joybinds.size();
    joybinds.resize(newSize);

    for (auto i = oldSize; i < newSize; ++i)
    {
        joybinds[i] = Unbound;
    }
}

void Input::save(const std::string& path)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.good() || !file.is_open())
    {
        xy::Logger::log("Failed opening file " + path + " for writing", xy::Logger::Type::Error, xy::Logger::Output::All);
        file.close();
        return;
    }

    std::uint32_t keySize = keybinds.size() * sizeof(std::pair<sf::Int32, sf::Int32>);
    std::uint32_t joySize = joybinds.size() * sizeof(sf::Int32);
    std::uint32_t totalSize = keySize + joySize + (sizeof(std::uint32_t) * 3); //don't forget to include sizes of sizes! :)

    file.write((char*)&totalSize, sizeof(std::uint32_t));
    file.write((char*)&keySize, sizeof(std::uint32_t));
    file.write((char*)keybinds.data(), keySize);
   
    file.write((char*)&joySize, sizeof(std::uint32_t));
    file.write((char*)joybinds.data(), joySize);

    file.close();

    LOG("Wrote keybinds to " + path, xy::Logger::Type::Info);
}

bool Input::load(const std::string& path) 
{
    std::ifstream file(path, std::ios::binary);
    if (!file.good() || !file.is_open())
    {
        xy::Logger::log("Failed opening file " + path + " for reading", xy::Logger::Type::Error, xy::Logger::Output::All);
        file.close();
        return false;
    }

    file.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < static_cast<int>(sizeof(std::uint32_t)))
    {
        Logger::log("unexpected file size for keybind data.", Logger::Type::Error, Logger::Output::All);
        file.close();
        return false;
    }

    std::vector<char>fileData(fileSize);
    file.read(fileData.data(), fileSize);
    file.close();

    char* filePtr = fileData.data();
    std::uint32_t readSize = 0;
    std::memcpy(&readSize, filePtr, sizeof(std::uint32_t));

    if (readSize != fileSize)
    {
        xy::Logger::log("Keybind data size is unexpected. Data not loaded.", xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }

    filePtr += sizeof(std::uint32_t);
    std::memcpy(&readSize, filePtr, sizeof(std::uint32_t));
    filePtr += sizeof(std::uint32_t);

    keybinds.resize(readSize / sizeof(std::pair<sf::Int32, sf::Int32>));
    std::memcpy(keybinds.data(), filePtr, readSize);
    filePtr += readSize;

    std::memcpy(&readSize, filePtr, sizeof(std::uint32_t));
    filePtr += sizeof(std::uint32_t);

    joybinds.resize(readSize / sizeof(sf::Int32));
    std::memcpy(joybinds.data(), filePtr, readSize);

    return true;
}