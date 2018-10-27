/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/core/FileSystem.hpp"
#include "xyginext/core/Log.hpp"

#include <SFML/System/String.hpp>

#include <sstream>
#include <array>

#if defined (_WIN32)
#include <shlobj.h>
//oh dear lord - don't look!
static inline sf::String getFontDir()
{
    wchar_t* retPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &retPath); //yes it's a pointer to a pointer

    if (retPath)
    {
        std::wstringstream ss;
        ss << retPath << "\\";

        CoTaskMemFree(static_cast<void*>(retPath)); //manually free this 0.o
        return { ss.str() };
    }
    return {};
}
#define FONT_PATH getFontDir().toAnsiString();
#elif defined (_APPLE_)
#define FONT_PATH "/Library/Fonts/"
#else
//linux may not technically have a fonts directory
//but if you're playing games we'll assume you're at
//least running a desktop environment :)
#define FONT_PATH "/usr/share/fonts/ttf/"
#endif

static inline std::string getFontPath()
{
    //candidate font names on various platforms - feel free to add
    //any for your platform of choice!!
    static const std::array<std::string, 4u> fontNames =
    {
        "arial.ttf",
        "tahoma.ttf",
        "georgia.ttf",
        "VeraMono.ttf"
    };

    auto fontPath = FONT_PATH;
    auto fileList = xy::FileSystem::listFiles(fontPath);

    if (!fileList.empty())
    {
        for (const auto& fontName : fontNames)
        {
            if (auto result = std::find(fileList.begin(), fileList.end(), fontName);
                result != fileList.end())
            {
                return fontPath + *result;
            }
        }
    }
    else
    {
        //some platforms have searchable subdirs (hello linux!)
        //auto dirList = xy::FileSystem::listDirectories(fontPath);
        xy::Logger::log("Could not find candidate font file for fallback font!", xy::Logger::Type::Error);
    }
    return {};
}