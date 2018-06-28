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

#include <vector>
#include <string>

#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/util/String.hpp>

namespace Locale
{
    enum
    {
        SinglePlayer, Multiplayer, Quit, Local, Network,
        Back, Host, Join, HelpText, SkipText, Hurry, GameOver,
        Continue, Yes, No, Score, PlayerOneScore, PlayerTwoScore,
        Resume
    };

    static std::vector<std::string> Strings = 
    {
        "Single Player",
        "Multiplayer",
        "QUIT",
        "Local",
        "Network",
        "Back",
        "Host",
        "Join",
        "#Undefined", //help text
        "Press Any Button To Skip",
        "Hurry Up!",
        "Game Over",
        "CONTINUE?",
        "YES",
        "NO",
        "Score: ",
        "Player One Score: ",
        "Player Two Score: ",
        "RESUME"
    };

    static void load(const std::string& path)
    {
        xy::ConfigFile cfg;
        if (cfg.loadFromFile(path))
        {
            const auto& properties = cfg.getProperties();
            for (const auto& p : properties)
            {
                auto name = xy::Util::String::toLower(p.getName());
                if (name == "single_player")
                {
                    Strings[SinglePlayer] = p.getValue<std::string>();
                }
                else if (name == "multiplayer")
                {
                    Strings[Multiplayer] = p.getValue<std::string>();
                }
                else if (name == "quit")
                {
                    Strings[Quit] = p.getValue<std::string>();
                }
                else if (name == "local")
                {
                    Strings[Local] = p.getValue<std::string>();
                }
                else if (name == "network")
                {
                    Strings[Network] = p.getValue<std::string>();
                }
                else if (name == "back")
                {
                    Strings[Back] = p.getValue<std::string>();
                }
                else if (name == "host")
                {
                    Strings[Host] = p.getValue<std::string>();
                }
                else if (name == "join")
                {
                    Strings[Join] = p.getValue<std::string>();
                }
                else if (name == "help_text")
                {
                    Strings[HelpText] = p.getValue<std::string>();
                }
                else if (name == "skip_text")
                {
                    Strings[SkipText] = p.getValue<std::string>();
                }
                else if (name == "hurry")
                {
                    Strings[Hurry] = p.getValue<std::string>();
                }
                else if (name == "game_over")
                {
                    Strings[GameOver] = p.getValue<std::string>();
                }
                else if (name == "continue")
                {
                    Strings[Continue] = p.getValue<std::string>();
                }
                else if (name == "yes")
                {
                    Strings[Yes] = p.getValue<std::string>();
                }
                else if (name == "no")
                {
                    Strings[No] = p.getValue<std::string>();
                }
                else if (name == "score")
                {
                    Strings[Score] = p.getValue<std::string>();
                }
                else if (name == "player_one_score")
                {
                    Strings[PlayerOneScore] = p.getValue<std::string>();
                }
                else if (name == "player_two_score")
                {
                    Strings[PlayerTwoScore] = p.getValue<std::string>();
                }
                else if (name == "resume")
                {
                    Strings[Resume] = p.getValue<std::string>();
                }
            }
        }
    }
}
