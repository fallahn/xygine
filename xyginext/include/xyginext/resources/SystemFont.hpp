/*********************************************************************
(c) Matt Marchant 2017 - 2019
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
#elif defined (__APPLE__)
#define FONT_PATH "/Library/Fonts/"
#else
//linux is an odd case because every distro is different
//and certain paths require admin rights - so the user
//will have to supply their own font directory path
#ifndef LINUX_FONT_PATH
#warning "define LINUX_FONT <fontpath> if a fallback font is required"
#define FONT_PATH "/usr/share/fonts/"
#else
#define FONT_PATH LINUX_FONT_PATH
#endif
#endif

static inline std::string getFontPath()
{
    std::string fontPath = FONT_PATH;

    if (!fontPath.empty())
    {
        return fontPath;
    }
    else
    {
        xy::Logger::log("Could not find candidate font file for fallback font!", xy::Logger::Type::Error);
#if defined(__linux__) && !defined(LINUX_FONT_PATH)
        xy::Logger::log("define LINUX_FONT_PATH as a path to default font directory", xy::Logger::Type::Info);
#endif
    }
    return {};
}