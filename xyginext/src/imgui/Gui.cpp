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

#include "xyginext/gui/Gui.hpp"
#include "xyginext/core/ConfigFile.hpp"

#include "imgui.h"

// Saves the hassle of mapping strings to all style vars
#define GET_VARIABLE_NAME(Variable) (#Variable)

using namespace xy;

namespace
{
    static Nim::Style currentStyle;
}

void Nim::begin(const std::string& title, bool* b)
{
    ImGui::Begin(title.c_str(), b);
}

void Nim::setNextWindowSize(float x, float y)
{
    ImGui::SetNextWindowSize({ x, y });
}

void Nim::setNextWindowConstraints(float minW, float minH, float maxW, float maxH)
{
    ImGui::SetNextWindowSizeConstraints({ minW, minH }, { maxW, maxH });
}

void Nim::setNextWindowPosition(float x, float y)
{
    ImGui::SetNextWindowPos({ x, y });
}

void Nim::text(const std::string& str)
{
    ImGui::Text("%s", str.c_str());
}

bool Nim::button(const std::string& label, float w, float h)
{
    return ImGui::Button(label.c_str(), { w,h });
}

void Nim::checkbox(const std::string& title, bool* value)
{
    ImGui::Checkbox(title.c_str(), value);
}

void Nim::slider(const std::string& title, float& value, float min, float max, float itemWidth)
{
    ImGui::PushItemWidth(itemWidth);
    ImGui::SliderFloat(title.c_str(), &value, min, max);
    ImGui::PopItemWidth();
}

void Nim::separator()
{
    ImGui::Separator();
}

void Nim::sameLine(float posX, float spacing)
{
    ImGui::SameLine(posX, spacing);
}

bool Nim::simpleCombo(const std::string& label, std::int32_t& index, const char* items, float itemWidth)
{
    ImGui::PushItemWidth(itemWidth);
    auto result = ImGui::Combo(label.c_str(), &index, items);
    ImGui::PopItemWidth();
    return result;
}

bool Nim::input(const std::string& label, std::int32_t& value, float itemWidth)
{
    ImGui::PushItemWidth(itemWidth);
    auto result = ImGui::InputInt(label.c_str(), &value);
    ImGui::PopItemWidth();
    return result;
}

void Nim::showToolTip(const std::string& message)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted(message.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool Nim::colourPicker(const std::string& label, sf::Color& output)
{
    float op[4] = 
    {
        static_cast<float>(output.r) / 255.f,
        static_cast<float>(output.g) / 255.f, 
        static_cast<float>(output.b) / 255.f, 
        static_cast<float>(output.a) / 255.f 
    };

    if (ImGui::ColorEdit4(label.c_str(), op))
    {
        output.r = static_cast<sf::Uint8>(op[0] * 255.f);
        output.g = static_cast<sf::Uint8>(op[1] * 255.f);
        output.b = static_cast<sf::Uint8>(op[2] * 255.f);
        output.a = static_cast<sf::Uint8>(op[3] * 255.f);

        return true;
    }
    return false;
}

void Nim::end()
{
    ImGui::End();
}

bool Nim::wantsMouse()
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool Nim::wantsKeyboard()
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

const Nim::Style& Nim::getStyle()
{
    auto style = ImGui::GetStyle();
    auto& s = currentStyle;
    s.Alpha = style.Alpha;
    s.AntiAliasedFill = style.AntiAliasedFill;
    s.AntiAliasedLines = style.AntiAliasedLines;
    s.ButtonTextAlign = style.ButtonTextAlign;
    s.ChildBorderSize = style.ChildBorderSize;
    s.ChildRounding = style.ChildRounding;
    s.ColumnsMinSpacing = style.ColumnsMinSpacing;
    s.CurveTessellationTol = style.CurveTessellationTol;
    s.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
    s.DisplayWindowPadding = style.DisplayWindowPadding;
    s.FrameBorderSize = style.FrameBorderSize;
    s.FramePadding = style.FramePadding;
    s.FrameRounding = style.FrameRounding;
    s.GrabMinSize = style.GrabMinSize;
    s.GrabRounding = style.GrabRounding;
    s.IndentSpacing = style.IndentSpacing;
    s.ItemInnerSpacing = style.ItemInnerSpacing;
    s.ItemSpacing = style.ItemSpacing;
    s.PopupBorderSize = style.PopupBorderSize;
    s.PopupRounding = style.PopupRounding;
    s.ScrollbarRounding = style.ScrollbarRounding;
    s.ScrollbarSize = style.ScrollbarSize;
    s.TouchExtraPadding = style.TouchExtraPadding;
    s.WindowBorderSize = style.WindowBorderSize;
    s.WindowMinSize = style.WindowMinSize;
    s.WindowPadding = style.WindowPadding;
    s.WindowRounding = style.WindowRounding;
    s.WindowTitleAlign = style.WindowTitleAlign;
    for (auto i=0u; i < s.colours.size(); i++)
        s.colours[i] = style.Colors[i];
    return currentStyle;
}

void Nim::setStyle(const Nim::Style& style)
{
    // copy vars to imgui style
    currentStyle = style;
    auto& s = ImGui::GetStyle();
    s.Alpha = style.Alpha;
    s.AntiAliasedFill = style.AntiAliasedFill;
    s.AntiAliasedLines = style.AntiAliasedLines;
    s.ButtonTextAlign = style.ButtonTextAlign;
    s.ChildBorderSize = style.ChildBorderSize;
    s.ChildRounding = style.ChildRounding;
    s.ColumnsMinSpacing = style.ColumnsMinSpacing;
    s.CurveTessellationTol = style.CurveTessellationTol;
    s.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
    s.DisplayWindowPadding = style.DisplayWindowPadding;
    s.FrameBorderSize = style.FrameBorderSize;
    s.FramePadding = style.FramePadding;
    s.FrameRounding = style.FrameRounding;
    s.GrabMinSize = style.GrabMinSize;
    s.GrabRounding = style.GrabRounding;
    s.IndentSpacing = style.IndentSpacing;
    s.ItemInnerSpacing = style.ItemInnerSpacing;
    s.ItemSpacing = style.ItemSpacing;
    s.PopupBorderSize = style.PopupBorderSize;
    s.PopupRounding = style.PopupRounding;
    s.ScrollbarRounding = style.ScrollbarRounding;
    s.ScrollbarSize = style.ScrollbarSize;
    s.TouchExtraPadding = style.TouchExtraPadding;
    s.WindowBorderSize = style.WindowBorderSize;
    s.WindowMinSize = style.WindowMinSize;
    s.WindowPadding = style.WindowPadding;
    s.WindowRounding = style.WindowRounding;
    s.WindowTitleAlign = style.WindowTitleAlign;
    for (auto i=0u; i < style.colours.size(); i++)
        s.Colors[i] = style.colours[i];
}

bool Nim::Style::saveToFile(const std::string& path)
{
    xy::ConfigFile styleFile;
    
    styleFile.addProperty(GET_VARIABLE_NAME(Alpha)).setValue(Alpha);
    styleFile.addProperty(GET_VARIABLE_NAME(AntiAliasedFill)).setValue(AntiAliasedFill);
    styleFile.addProperty(GET_VARIABLE_NAME(AntiAliasedLines)).setValue(AntiAliasedLines);
    styleFile.addProperty(GET_VARIABLE_NAME(ButtonTextAlign)).setValue(ButtonTextAlign);
    styleFile.addProperty(GET_VARIABLE_NAME(ChildBorderSize)).setValue(ChildBorderSize);
    styleFile.addProperty(GET_VARIABLE_NAME(ChildRounding)).setValue(ChildRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(ColumnsMinSpacing)).setValue(ColumnsMinSpacing);
    styleFile.addProperty(GET_VARIABLE_NAME(CurveTessellationTol)).setValue(CurveTessellationTol);
    styleFile.addProperty(GET_VARIABLE_NAME(DisplaySafeAreaPadding)).setValue(DisplaySafeAreaPadding);
    styleFile.addProperty(GET_VARIABLE_NAME(DisplayWindowPadding)).setValue(DisplayWindowPadding);
    styleFile.addProperty(GET_VARIABLE_NAME(FrameBorderSize)).setValue(FrameBorderSize);
    styleFile.addProperty(GET_VARIABLE_NAME(FramePadding)).setValue(FramePadding);
    styleFile.addProperty(GET_VARIABLE_NAME(FrameRounding)).setValue(FrameRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(GrabMinSize)).setValue(GrabMinSize);
    styleFile.addProperty(GET_VARIABLE_NAME(GrabRounding)).setValue(GrabRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(IndentSpacing)).setValue(IndentSpacing);
    styleFile.addProperty(GET_VARIABLE_NAME(ItemInnerSpacing)).setValue(ItemInnerSpacing);
    styleFile.addProperty(GET_VARIABLE_NAME(ItemSpacing)).setValue(ItemSpacing);
    styleFile.addProperty(GET_VARIABLE_NAME(PopupBorderSize)).setValue(PopupBorderSize);
    styleFile.addProperty(GET_VARIABLE_NAME(PopupRounding)).setValue(PopupRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(ScrollbarRounding)).setValue(ScrollbarRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(ScrollbarSize)).setValue(ScrollbarSize);
    styleFile.addProperty(GET_VARIABLE_NAME(TouchExtraPadding)).setValue(TouchExtraPadding);
    styleFile.addProperty(GET_VARIABLE_NAME(WindowBorderSize)).setValue(WindowBorderSize);
    styleFile.addProperty(GET_VARIABLE_NAME(WindowMinSize)).setValue(WindowMinSize);
    styleFile.addProperty(GET_VARIABLE_NAME(WindowPadding)).setValue(WindowPadding);
    styleFile.addProperty(GET_VARIABLE_NAME(WindowRounding)).setValue(WindowRounding);
    styleFile.addProperty(GET_VARIABLE_NAME(WindowTitleAlign)).setValue(WindowTitleAlign);
    
    auto colourObj = styleFile.addObject("Colours");
    for (auto i=0u; i<colours.size(); i++)
    {
        colourObj->addProperty(std::to_string(i)).setValue(colours[i]);
    }
    
    return styleFile.save(path);
}

bool Nim::Style::loadFromFile(const std::string& path)
{
    xy::ConfigFile styleFile;
    if (!styleFile.loadFromFile(path))
        return false;
    
    // I'm not going to check the property is found, bc I'm a loose cannon
    // Also kind of wish I'd written a helper function before doing this...
    Alpha = styleFile.findProperty(GET_VARIABLE_NAME(Alpha))->getValue<decltype(Alpha)>();
    AntiAliasedFill = styleFile.findProperty(GET_VARIABLE_NAME(AntiAliasedFill))->getValue<decltype(AntiAliasedFill)>();
    AntiAliasedLines = styleFile.findProperty(GET_VARIABLE_NAME(AntiAliasedLines))->getValue<decltype(AntiAliasedLines)>();
    ButtonTextAlign = styleFile.findProperty(GET_VARIABLE_NAME(ButtonTextAlign))->getValue<decltype(ButtonTextAlign)>();
    ChildBorderSize = styleFile.findProperty(GET_VARIABLE_NAME(ChildBorderSize))->getValue<decltype(ChildBorderSize)>();
    ChildRounding = styleFile.findProperty(GET_VARIABLE_NAME(ChildRounding))->getValue<decltype(ChildRounding)>();
    ColumnsMinSpacing = styleFile.findProperty(GET_VARIABLE_NAME(ColumnsMinSpacing))->getValue<decltype(ColumnsMinSpacing)>();
    CurveTessellationTol = styleFile.findProperty(GET_VARIABLE_NAME(CurveTessellationTol))->getValue<decltype(CurveTessellationTol)>();
    DisplaySafeAreaPadding = styleFile.findProperty(GET_VARIABLE_NAME(DisplaySafeAreaPadding))->getValue<decltype(DisplaySafeAreaPadding)>();
    DisplayWindowPadding = styleFile.findProperty(GET_VARIABLE_NAME(DisplayWindowPadding))->getValue<decltype(DisplayWindowPadding)>();
    FrameBorderSize = styleFile.findProperty(GET_VARIABLE_NAME(FrameBorderSize))->getValue<decltype(FrameBorderSize)>();
    FramePadding = styleFile.findProperty(GET_VARIABLE_NAME(FramePadding))->getValue<decltype(FramePadding)>();
    FrameRounding = styleFile.findProperty(GET_VARIABLE_NAME(FrameRounding))->getValue<decltype(FrameRounding)>();
    GrabMinSize = styleFile.findProperty(GET_VARIABLE_NAME(GrabMinSize))->getValue<decltype(GrabMinSize)>();
    GrabRounding = styleFile.findProperty(GET_VARIABLE_NAME(GrabRounding))->getValue<decltype(GrabRounding)>();
    IndentSpacing = styleFile.findProperty(GET_VARIABLE_NAME(IndentSpacing))->getValue<decltype(IndentSpacing)>();
    ItemInnerSpacing = styleFile.findProperty(GET_VARIABLE_NAME(ItemInnerSpacing))->getValue<decltype(ItemInnerSpacing)>();
    ItemSpacing = styleFile.findProperty(GET_VARIABLE_NAME(ItemSpacing))->getValue<decltype(ItemSpacing)>();
    PopupBorderSize = styleFile.findProperty(GET_VARIABLE_NAME(PopupBorderSize))->getValue<decltype(PopupBorderSize)>();
    PopupRounding = styleFile.findProperty(GET_VARIABLE_NAME(PopupRounding))->getValue<decltype(currentStyle.PopupRounding)>();
    ScrollbarRounding = styleFile.findProperty(GET_VARIABLE_NAME(ScrollbarRounding))->getValue<decltype(ScrollbarRounding)>();
    ScrollbarSize = styleFile.findProperty(GET_VARIABLE_NAME(ScrollbarSize))->getValue<decltype(ScrollbarSize)>();
    TouchExtraPadding = styleFile.findProperty(GET_VARIABLE_NAME(TouchExtraPadding))->getValue<decltype(TouchExtraPadding)>();
    WindowBorderSize = styleFile.findProperty(GET_VARIABLE_NAME(WindowBorderSize))->getValue<decltype(WindowBorderSize)>();
    WindowMinSize = styleFile.findProperty(GET_VARIABLE_NAME(WindowMinSize))->getValue<decltype(WindowMinSize)>();
    WindowPadding = styleFile.findProperty(GET_VARIABLE_NAME(WindowPadding))->getValue<decltype(WindowPadding)>();
    WindowRounding = styleFile.findProperty(GET_VARIABLE_NAME(WindowRounding))->getValue<decltype(WindowRounding)>();
    WindowTitleAlign = styleFile.findProperty(GET_VARIABLE_NAME(WindowTitleAlign))->getValue<decltype(WindowTitleAlign)>();
    
    auto colourObj = styleFile.findObjectWithName("Colours");
    for (auto i=0u; i<colours.size(); i++)
    {
        // Need a better method for this...
        colours[i] = colourObj->findProperty(std::to_string(i))->getValue<sf::Color>();
    }
    
    return true;
}

//I miss you like hell.
