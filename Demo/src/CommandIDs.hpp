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

namespace CommandID
{
    enum
    {
        NetActor = 0x1,
        MenuText = 0x2,
        PlayerOne = 0x4,
        PlayerTwo = 0x8,
        NPC = 0x10,
        Hat = 0x20,
        Luggage = 0x40,
        //scoreboard/UI
        ScoreOne = 0x80,
        ScoreTwo = 0x100,
        LivesOne = 0x200,
        LivesTwo = 0x400,
        Timeout = 0x800,
        HighScore = 0x1000,
        LevelCounter = 0x2000,
        UIElement = 0x4000,
        //map data
        MapItem = 0x8000, //anything with this is removed on map changed
        MapBackground = 0x10000,
        SceneBackground = 0x20000,
        SceneMusic = 0x40000,
        //UI animations
        Princess = 0x80000,
        TowerDude = 0x100000,
        BonusBall = 0x200000,
        KeybindInput = 0x400000
    };
}
