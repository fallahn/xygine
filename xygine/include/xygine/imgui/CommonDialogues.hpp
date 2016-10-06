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

#ifndef XY_COMMON_DIALOGUES_HPP_
#define XY_COMMON_DIALOGUES_HPP_

#include <xygine/Config.hpp>

#include <string>

/*!
\brief Collection of imgui functions for commonly used dialogues
*/
namespace ImGui
{
    /*!
    \brief Opens a file browse dialogue.
    \param std::string Title of the window
    \param std::string Result of selected file
    \param bool Show the window or not
    \param bool Set to true if the returned file path should be relative to the current directory
    */
    XY_EXPORT_API bool fileBrowseDialogue(const std::string&, std::string&, bool, bool = true);

    /*!
    \brief Shows a message box window.
    \param title Title of window
    \param msg Message to display
    \param bool If this window is open or not
    */
    XY_EXPORT_API void showMessage(const std::string& title, const std::string& msg, bool);
}

#endif //XY_COMMON_DIALOGUES_HPP_