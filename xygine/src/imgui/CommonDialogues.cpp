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

#include <xygine/imgui/CommonDialogues.hpp>
#include <xygine/imgui/imgui.h>
#include <xygine/FileSystem.hpp>

#include <cstring>

bool nim::fileBrowseDialogue(const std::string& title, std::string& output, bool open, bool returnRelative)
{
    static std::string currentDir = xy::FileSystem::getCurrentDirectory();
    static const std::string rootDir = currentDir;
    static auto directories = xy::FileSystem::listDirectories(currentDir);
    static auto files = xy::FileSystem::listFiles(currentDir);
    
    if (open)
    {
        nim::OpenPopup(title.c_str());
    }

    nim::SetNextWindowSize({ 400.f, 360.f });
    if (nim::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        nim::TextWrapped(currentDir.c_str());

        if (nim::Button("Up"))
        {
            //trim working dir and update lists
            auto result = currentDir.find_last_of('/');
            if (result != std::string::npos)
            {
                currentDir = currentDir.substr(0, result);
                directories = xy::FileSystem::listDirectories(currentDir);
                files = xy::FileSystem::listFiles(currentDir);
            }
        }
        bool refresh = false;
        //nim::SameLine(); //can't popup inside a popup :/
        //if (nim::Button("Create Directory"))
        //{
        //    nim::OpenPopup("Create:");
        //    if (nim::BeginPopupModal("Create:", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        //    {
        //        static char dirName[40] = "\0";
        //        nim::InputText("Directory Name", dirName, 40);
        //        if (nim::Button("OK", { 80.f, 0.f }))
        //        {
        //            //TODO create dir
        //            refresh = true;
        //            nim::CloseCurrentPopup();
        //        }
        //        nim::SameLine();
        //        if (nim::Button("Cancel", { 80.f, 0.f }))
        //        {
        //            nim::CloseCurrentPopup();
        //        }
        //        nim::EndPopup();
        //    }
        //}

        nim::BeginChild("ScrollRegion", ImVec2(0, -nim::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);        
        for (const auto& dir : directories)
        {
            //move to selected dir on double click
            if (nim::Selectable(dir.c_str(), false, ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (nim::IsMouseDoubleClicked(0))
                {
                    currentDir += "/" + dir;
                    refresh = true;
                }
            }
        }

        static char fileName[50] = "";
        
        for (const auto& file : files)
        {
            //set output 
            if (nim::Selectable(file.c_str(), false, ImGuiSelectableFlags_::ImGuiSelectableFlags_AllowDoubleClick))
            {
                std::memcpy(fileName, file.data(), file.size());
                fileName[file.size()] = '\0';

                if (nim::IsMouseDoubleClicked(0))
                {
                    std::string filepath = currentDir + "/" + file;
                    if (returnRelative)
                    {
                        output = xy::FileSystem::getRelativePath(filepath, rootDir);
                    }
                    else
                    {
                        output = filepath;
                    }
                    nim::EndChild();
                    nim::CloseCurrentPopup();
                    nim::EndPopup();
                    return true;
                }
            }
        }

        if (refresh)
        {
            directories = xy::FileSystem::listDirectories(currentDir);
            files = xy::FileSystem::listFiles(currentDir);
        }
        nim::EndChild();

        nim::InputText("", fileName, 50);
        nim::SameLine();

        if (nim::Button("Select"))
        {
            std::string filepath = currentDir + "/" + fileName;
            if (returnRelative)
            {
                output = xy::FileSystem::getRelativePath(filepath, rootDir);
            }
            else
            {
                output = filepath;
            }
            fileName[0] = '\0';
            nim::CloseCurrentPopup();
            nim::EndPopup();
            return true;
        }
        nim::SameLine();
        if (nim::Button("Close"))
        {
            nim::CloseCurrentPopup();
            fileName[0] = '\0';
        }
        nim::EndPopup();
        return false;        
    }
    return false;
}

void nim::showMessage(const std::string& title, const std::string& msg, bool open)
{
    if (open)
    {
        nim::OpenPopup(title.c_str());
    }

    nim::SetNextWindowSize({ 200.f, 80.f });
    nim::SetNextWindowPosCenter();

    if (nim::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        nim::Text(msg.c_str());

        if (nim::Button("Close"))
        {
            nim::CloseCurrentPopup();
        }
        nim::EndPopup();
    }
}
