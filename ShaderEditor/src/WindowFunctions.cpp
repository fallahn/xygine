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
#include "TextEditor.h"

void doTextEditor(TextEditor& editor)
{
    //based on https://github.com/BalazsJako/ColorTextEditorDemo 

    auto cpos = editor.GetCursorPosition();
    ImGui::Begin("Text Editor Demo", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                auto textToSave = editor.GetText();
                /// save text....
            }
            /*if (ImGui::MenuItem("Quit", "Alt-F4"))
            {
                return;
            }*/
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = editor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
            {
                editor.SetReadOnly(ro);
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
            {
                editor.Undo();
            }
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
            {
                editor.Redo();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
            {
                editor.Copy();
            }
            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
            {
                editor.Cut();
            }
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
            {
                editor.Delete();
            }
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
            {
                editor.Paste();
            }
            ImGui::Separator();

            if (ImGui::MenuItem("Select all", nullptr, nullptr))
            {
                editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
            {
                editor.SetPalette(TextEditor::GetDarkPalette());
            }
            if (ImGui::MenuItem("Light palette"))
            {
                editor.SetPalette(TextEditor::GetLightPalette());
            }
            if (ImGui::MenuItem("Retro blue palette"))
            {
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
        editor.IsOverwrite() ? "Ovr" : "Ins",
        editor.CanUndo() ? "*" : " ",
        editor.GetLanguageDefinition().mName.c_str(), /*fileToEdit*/"");

    editor.Render("TextEditor");
    ImGui::End();
}