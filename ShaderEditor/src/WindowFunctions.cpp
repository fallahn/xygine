/*********************************************************************
(c) Matt Marchant 2019
http://trederia.blogspot.com

xygineXT Shader Editor - Zlib license.

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

#include "WindowFunctions.hpp"
#include "imgui/imgui.h"

namespace
{

}

void showOptions(std::bitset<WindowFlags::Count>& windowFlags)
{
    if (!ImGui::Begin("Options"))
    {
        ImGui::End();
        return;
    }

    //TODO set up uniforms - ideally we want to parse this from the editor

    if (ImGui::Button("Compile"))
    {
        windowFlags.set(RunShader);
    }
    ImGui::SameLine();
    if (windowFlags.test(CompileSuccess))
    {
        //colours are ABGR
        ImGui::PushStyleColor(ImGuiCol_Button, 0xff00ff00);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xff000000);
        ImGui::Button("Succeeded");
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, 0xff0000ff);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xffffffff);
        ImGui::Button("Failed");
        ImGui::Text("See console for errors.");
    }
    ImGui::PopStyleColor(2);

    ImGui::End();
}