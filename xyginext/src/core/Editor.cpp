//
//  Editor.cpp
//  xyginext
//
//  Created by Jonny Paton on 12/03/2018.
//

#include <xyginext/core/Editor.hpp>
#include <xyginext/core/Log.hpp>
#include <xyginext/core/App.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

#include "../imgui/imgui_dock.hpp"
#include "../imgui/imgui-SFML.h"

using namespace xy;

bool Editor::m_enabled = false;
std::vector<Editable*> Editor::m_editables;
sf::RenderTexture Editor::m_viewportBuffer;

namespace
{
    // To keep track of multiple editables of the same type
    // This is only really required to prevent conflicts in the imgui window names
    // Adding some sort of name to editable stuff would negate this
    static int eId = 0;
}

Editable::Editable() :
m_id(eId++)
{
    Editor::m_editables.push_back(this);
}

Editable::~Editable()
{
    auto me = std::find(Editor::m_editables.begin(), Editor::m_editables.end(), this);
    Editor::m_editables.erase(me);
}

void Editor::toggle()
{
    m_enabled = !m_enabled;
    xy::Logger::log(std::string("Editor ") + (m_enabled ? "enabled" : "disabled"));
    
    // Modify the apps rendertarget
    if (m_enabled)
    {
        m_viewportBuffer.create(App::getRenderTarget()->getSize().x, App::getRenderTarget()->getSize().y);
        App::getActiveInstance()->setRenderTarget(&m_viewportBuffer);
    }
    else
    {
        App::getActiveInstance()->setRenderTarget(&App::getActiveInstance()->m_renderWindow);
    }
}

bool Editor::isEnabled()
{
    return m_enabled;
}

void Editor::draw()
{
    if (m_enabled)
    {
        // Main menu bar first.
        float menuBarHeight = 0;
        if (ImGui::BeginMainMenuBar())
        {
            // Some stuff will go here...
            
            menuBarHeight = ImGui::GetWindowHeight();
            
            ImGui::EndMainMenuBar();
        }
        
        // Create an imgui window covering the entire renderwindow (minus the menu bar)
        auto rwSize = App::getActiveInstance()->m_renderWindow.getSize();
        ImGui::SetNextWindowSize({static_cast<float>(rwSize.x), rwSize.y - menuBarHeight});
        ImGui::SetNextWindowPos({0,menuBarHeight});
        
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
        ImGui::BeginDockspace();
        
        // Draw the console
        Console::draw();
        
        // Draw any other registered editable objects
        for (auto& e : m_editables)
        {
            e->editorDraw();
        }
        
        // Show viewport
        if (ImGui::BeginDock("Viewport"))
        {
            // Make the viewport scale to available space
            auto vpw = m_viewportBuffer.getSize().x;
            auto vph = m_viewportBuffer.getSize().y;
            auto availablew = ImGui::GetContentRegionAvailWidth();
            auto availableh = ImGui::GetContentRegionAvail().y;
            auto scale = std::min(availablew / vpw, availableh / vph);
            auto size = static_cast<sf::Vector2f>(m_viewportBuffer.getSize()) * scale;
            ImGui::Image(m_viewportBuffer, size);
        }
        ImGui::EndDock();
        
        // Show asset browser
        if (ImGui::BeginDock("Assets"))
        {
            std::function<void(std::string)> imFileTreeRecurse = [&](std::string path)
            {
                // List directories, recurse if selected
                for (auto& dir : xy::FileSystem::listDirectories(path))
                {
                    if (ImGui::TreeNode(dir.c_str()))
                    {
                        imFileTreeRecurse(path + "/" + dir);
                        ImGui::TreePop();
                    }
                }
                
                // List files, broadcast message if selected
                for (auto& file : xy::FileSystem::listFiles(path))
                {
                    if (ImGui::Selectable(file.c_str()))
                    {
                        // Do something... show specific asset editor?
                    }
                }
            };
            
            imFileTreeRecurse("assets");
        }
        
        ImGui::EndDock();
        
        ImGui::EndDockspace();
        ImGui::End(); // Editor
    }
}

