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

#ifndef DEMO_GAME_COMMAND_ID_HPP_
#define DEMO_GAME_COMMAND_ID_HPP_

namespace CommandID
{
    enum
    {
        NetActor = 0x1,
        MenuText = 0x2,
        PlayerOne = 0x4,
        PlayerTwo = 0x8,
        //scoreboard/UI
        ScoreOne = 0x10,
        ScoreTwo = 0x20,
        LivesOne = 0x40,
        LivesTwo = 0x80,
        Timeout = 0x100,
        HighScore = 0x200
    };
}

#endif //DEMO_GAME_COMMAND_ID_HPP_