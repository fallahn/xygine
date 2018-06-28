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

#pragma once

#include <SFML/Window/Keyboard.hpp>

#include <map>
#include <string>

static const std::map<sf::Keyboard::Key, std::string> KeyMap = 
{
    std::make_pair(sf::Keyboard::Unknown, "Invalid"),
    std::make_pair(sf::Keyboard::A, "A"),
    std::make_pair(sf::Keyboard::B, "B"),
    std::make_pair(sf::Keyboard::C, "C"),
    std::make_pair(sf::Keyboard::D, "D"),
    std::make_pair(sf::Keyboard::E, "E"),
    std::make_pair(sf::Keyboard::F, "F"),
    std::make_pair(sf::Keyboard::G, "G"),
    std::make_pair(sf::Keyboard::H, "H"),
    std::make_pair(sf::Keyboard::I, "I"),
    std::make_pair(sf::Keyboard::J, "J"),
    std::make_pair(sf::Keyboard::K, "K"),
    std::make_pair(sf::Keyboard::L, "L"),
    std::make_pair(sf::Keyboard::M, "M"),
    std::make_pair(sf::Keyboard::N, "N"),
    std::make_pair(sf::Keyboard::O, "O"),
    std::make_pair(sf::Keyboard::P, "P"),
    std::make_pair(sf::Keyboard::Q, "Q"),
    std::make_pair(sf::Keyboard::R, "R"),
    std::make_pair(sf::Keyboard::S, "S"),
    std::make_pair(sf::Keyboard::T, "T"),
    std::make_pair(sf::Keyboard::U, "U"),
    std::make_pair(sf::Keyboard::V, "V"),
    std::make_pair(sf::Keyboard::W, "W"),
    std::make_pair(sf::Keyboard::X, "X"),
    std::make_pair(sf::Keyboard::Y, "Y"),
    std::make_pair(sf::Keyboard::Z, "Z"),
    std::make_pair(sf::Keyboard::Num0, "0"),
    std::make_pair(sf::Keyboard::Num1, "1"),
    std::make_pair(sf::Keyboard::Num2, "2"),
    std::make_pair(sf::Keyboard::Num3, "3"),
    std::make_pair(sf::Keyboard::Num4, "4"),
    std::make_pair(sf::Keyboard::Num5, "5"),
    std::make_pair(sf::Keyboard::Num6, "6"),
    std::make_pair(sf::Keyboard::Num7, "7"),
    std::make_pair(sf::Keyboard::Num8, "8"),
    std::make_pair(sf::Keyboard::Num9, "9"),
    std::make_pair(sf::Keyboard::Escape, "Esc"),
    std::make_pair(sf::Keyboard::LControl, "LCtrl"),
    std::make_pair(sf::Keyboard::LShift, "LShift"),
    std::make_pair(sf::Keyboard::LAlt, "LAlt"),
    std::make_pair(sf::Keyboard::LSystem, "Sys"),
    std::make_pair(sf::Keyboard::RControl, "RCtrl"),
    std::make_pair(sf::Keyboard::RShift, "RShift"),
    std::make_pair(sf::Keyboard::RAlt, "RAlt"),
    std::make_pair(sf::Keyboard::RSystem, "Sys"),
    std::make_pair(sf::Keyboard::Menu, "Menu"),
    std::make_pair(sf::Keyboard::LBracket, "{"),
    std::make_pair(sf::Keyboard::RBracket, "}"),
    std::make_pair(sf::Keyboard::SemiColon, ";"),
    std::make_pair(sf::Keyboard::Comma, ","),
    std::make_pair(sf::Keyboard::Period, "."),
    std::make_pair(sf::Keyboard::Quote, "\""),
    std::make_pair(sf::Keyboard::Slash, "/"),
    std::make_pair(sf::Keyboard::BackSlash, "\\"),
    std::make_pair(sf::Keyboard::Tilde, "~"),
    std::make_pair(sf::Keyboard::Equal, "="),
    std::make_pair(sf::Keyboard::Dash, "-"),
    std::make_pair(sf::Keyboard::Space, "Space"),
    std::make_pair(sf::Keyboard::Return, "Return"),
    std::make_pair(sf::Keyboard::BackSpace, "Backspace"),
    std::make_pair(sf::Keyboard::Tab, "Tab"),
    std::make_pair(sf::Keyboard::PageUp, "PgUp"),
    std::make_pair(sf::Keyboard::PageDown, "PgDn"),
    std::make_pair(sf::Keyboard::End, "End"),
    std::make_pair(sf::Keyboard::Home, "Home"),
    std::make_pair(sf::Keyboard::Insert, "Ins"),
    std::make_pair(sf::Keyboard::Delete, "Del"),
    std::make_pair(sf::Keyboard::Add, "+"),
    std::make_pair(sf::Keyboard::Subtract, "-"),
    std::make_pair(sf::Keyboard::Multiply, "*"),
    std::make_pair(sf::Keyboard::Divide, "/"),
    std::make_pair(sf::Keyboard::Left, "Left"),
    std::make_pair(sf::Keyboard::Right, "Right"),
    std::make_pair(sf::Keyboard::Up, "Up"),
    std::make_pair(sf::Keyboard::Down, "Down"),
    std::make_pair(sf::Keyboard::Numpad0, "Keypad 0"),
    std::make_pair(sf::Keyboard::Numpad1, "Keypad 1"),
    std::make_pair(sf::Keyboard::Numpad2, "Keypad 2"),
    std::make_pair(sf::Keyboard::Numpad3, "Keypad 3"),
    std::make_pair(sf::Keyboard::Numpad4, "Keypad 4"),
    std::make_pair(sf::Keyboard::Numpad5, "Keypad 5"),
    std::make_pair(sf::Keyboard::Numpad6, "Keypad 6"),
    std::make_pair(sf::Keyboard::Numpad7, "Keypad 7"),
    std::make_pair(sf::Keyboard::Numpad8, "Keypad 8"),
    std::make_pair(sf::Keyboard::Numpad9, "Keypad 9"),
    std::make_pair(sf::Keyboard::F1, "F1"),
    std::make_pair(sf::Keyboard::F2, "F2"),
    std::make_pair(sf::Keyboard::F3, "F3"),
    std::make_pair(sf::Keyboard::F4, "F4"),
    std::make_pair(sf::Keyboard::F5, "F5"),
    std::make_pair(sf::Keyboard::F6, "F6"),
    std::make_pair(sf::Keyboard::F7, "F7"),
    std::make_pair(sf::Keyboard::F8, "F8"),
    std::make_pair(sf::Keyboard::F9, "F9"),
    std::make_pair(sf::Keyboard::F10, "F10"),
    std::make_pair(sf::Keyboard::F11, "F11"),
    std::make_pair(sf::Keyboard::F12, "F12"),
    std::make_pair(sf::Keyboard::F13, "F13"),
    std::make_pair(sf::Keyboard::F14, "F14"),
    std::make_pair(sf::Keyboard::F15, "F15"),
    std::make_pair(sf::Keyboard::Pause, "Pause")
};
