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

#ifndef XY_GUI_HPP_
#define XY_GUI_HPP_

#include <xyginext/Config.hpp>

#include <SFML/Graphics/Color.hpp>

#include <string>
#include <array>

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
        \see ImGui::ImGuiStyle
        */
        struct Style
        {
            
            // Enumeration for colours
            enum class Color
            {
                Text,
                TextDisabled,
                WindowBg,              // Background of normal windows
                ChildBg,               // Background of child windows
                PopupBg,               // Background of popups, menus, tooltips windows
                Border,
                BorderShadow,
                FrameBg,               // Background of checkbox, radio button, plot, slider, text input
                FrameBgHovered,
                FrameBgActive,
                TitleBg,
                TitleBgActive,
                TitleBgCollapsed,
                MenuBarBg,
                ScrollbarBg,
                ScrollbarGrab,
                ScrollbarGrabHovered,
                ScrollbarGrabActive,
                CheckMark,
                SliderGrab,
                SliderGrabActive,
                Button,
                ButtonHovered,
                ButtonActive,
                Header,
                HeaderHovered,
                HeaderActive,
                Separator,
                SeparatorHovered,
                SeparatorActive,
                ResizeGrip,
                ResizeGripHovered,
                ResizeGripActive,
                CloseButton,
                CloseButtonHovered,
                CloseButtonActive,
                PlotLines,
                PlotLinesHovered,
                PlotHistogram,
                PlotHistogramHovered,
                TextSelectedBg,
                ModalWindowDarkening,  // darken entire screen when a modal window is active
                DragDropTarget,
                Count
            };
            
            float           Alpha = 1.f;                      // Global alpha applies to everything in Nim
            sf::Vector2i    WindowPadding = {8,8};              // Padding within a window
            float           WindowRounding = 7.f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
            float           WindowBorderSize = 0.f;           // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly)
            sf::Vector2u    WindowMinSize = {32,32};              // Minimum window size
            sf::Vector2f    WindowTitleAlign = {0.f,0.5f};           // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
            float           ChildRounding = 0.f;              // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
            float           ChildBorderSize = 1.f;            // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly)
            float           PopupRounding = 0.f;              // Radius of popup window corners rounding.
            float           PopupBorderSize = 1.f;            // Thickness of border around popup windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly)
            sf::Vector2i    FramePadding = {4,3};               // Padding within a framed rectangle (used by most widgets)
            float           FrameRounding = 0.f;              // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
            float           FrameBorderSize = 1.f;            // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly)
            sf::Vector2i    ItemSpacing = {8,4} ;                // Horizontal and vertical spacing between widgets/lines
            sf::Vector2i    ItemInnerSpacing = {4,4};           // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
            sf::Vector2i    TouchExtraPadding {0,0};          // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
            float           IndentSpacing = 21.f;              // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
            float           ColumnsMinSpacing = 6.f;          // Minimum horizontal spacing between two columns
            float           ScrollbarSize = 16.f;              // Width of the vertical scrollbar, Height of the horizontal scrollbar
            float           ScrollbarRounding = 9.f;          // Radius of grab corners for scrollbar
            float           GrabMinSize = 10.f;                // Minimum width/height of a grab box for slider/scrollbar.
            float           GrabRounding = 0.f;               // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
            sf::Vector2f    ButtonTextAlign = {0.5f,0.5f};            // Alignment of button text when button is larger than text. Defaults to (0.5f,0.5f) for horizontally+vertically centered.
            sf::Vector2i    DisplayWindowPadding = {22,22};       // Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
            sf::Vector2i    DisplaySafeAreaPadding = {4,4};     // If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
            bool            AntiAliasedLines = true;           // Enable anti-aliasing on lines/borders. Disable if you are really tight on CPU/GPU.
            bool            AntiAliasedFill = true;            // Enable anti-aliasing on filled shapes (rounded rectangles, circles, etc.)
            float           CurveTessellationTol = 1.25f;       // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
            
            std::array<sf::Color, static_cast<size_t>(Color::Count)>         Colors;
            
            XY_EXPORT_API   bool loadFromFile(const std::string& path);
            XY_EXPORT_API   bool saveToFile(const std::string& path);
            
        };
        
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
        
        /*!
        \see ImGui::GetIo().WantCaptureMouse
        */
        XY_EXPORT_API bool wantsMouse();
        
        /*!
        \see ImGui::GetIo().WantCaptureKeyboard
        */
        XY_EXPORT_API bool wantsKeyboard();
        
        /*!
        \brief Get the current style
        */
        XY_EXPORT_API const Style& getStyle();
        
        /*!
        \brief Set the current style
        */
        XY_EXPORT_API void setStyle(const Style& style);
        
        /*!
         \brief Set the current style
         */
        XY_EXPORT_API void setStyle(const Style& style);
    }
}

#endif //XY_GUI_HPP_
