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

#ifndef XY_GUI_HPP_
#define XY_GUI_HPP_

#include <xyginext/Config.hpp>

#include <string>

namespace xy
{
    /*!
    \brief Exposes a selection of ImGui functions to the public API.
    These can be used to create stand-alone windows or to add useful
    information to the status window via App::registerStatusOutput().
    \see GuiClient
    */
    namespace Nim
    {
        /*!
        \see ImGui::Begin()
        */
        XY_EXPORT_API void begin(const std::string& title, bool* open = nullptr);

        /*!
        \see ImGui::SetNextWindowSize()
        */
        XY_EXPORT_API void setNextWindowSize(float x, float y);

        /*!
        \see ImGui::SetNextWindowSizeConstraints()
        */
        XY_EXPORT_API void setNextWindowConstraints(float minW, float minY, float maxW, float maxY);

        /*!
        \brief Set the next window position in *window* coordinates
        \see ImGui::SetNextWindowPos()
        */
        XY_EXPORT_API void setNextWindowPosition(float x, float y);

        /*!
        \see ImGui::Text()
        */
        XY_EXPORT_API void text(const std::string& str);

        /*!
        \see ImGui::Button()
        */
        XY_EXPORT_API bool button(const std::string& label, float w = 0.f, float h = 0.f);

        /*!
        \see ImGui::CheckBox()
        */
        XY_EXPORT_API void checkbox(const std::string& title, bool* value);

        /*!
        \see ImGui::FloatSlider()
        */
        XY_EXPORT_API void slider(const std::string& title, float& value, float min, float max);

        /*!
        \see ImGui::End()
        */
        XY_EXPORT_API void end();
    }
}

#endif //XY_GUI_HPP_