//
//  Editor.cpp
//  xyginext
//
//  Created by Jonny Paton on 12/03/2018.
//

#include <xyginext/core/Editor.hpp>
#include <xyginext/core/Log.hpp>
#include <xyginext/core/App.hpp>
#include <xyginext/gui/Gui.hpp>

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include "../imgui/imgui_dock.hpp"
#include "../imgui/imgui-SFML.h"

using namespace xy;

namespace
{
    bool enabled = false;
    
    // bc name clash with function and bc lazy...
    bool shouldShowStyleEditor = false;
    bool shouldShowVideoSettings = false;
    bool shouldShowAudioSettings = false;
    bool shouldShowConsole = false;
    std::vector<Editable*> editables;
    sf::RenderTexture viewportBuffer;
    int currentResolution = 0;
    std::array<char, 300> resolutionNames{};
    std::vector<sf::Vector2u> resolutions;
    bool fullScreen = false;
    bool vSync = false;
    bool useFrameLimit = false;
    int frameLimit = 10;
}

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
    editables.push_back(this);
}

Editable::~Editable()
{
    auto me = std::find(editables.begin(), editables.end(), this);
    editables.erase(me);
}

void Editor::init()
{
    auto modes = sf::VideoMode::getFullscreenModes();
    for (const auto& mode : modes)
    {
        if (mode.bitsPerPixel == 32)
        {
            resolutions.emplace_back(mode.width, mode.height);
        }
    }
    
    std::reverse(std::begin(resolutions), std::end(resolutions));
    
    auto rwSize = App::getActiveInstance()->m_renderWindow.getSize();
    
    int i = 0;
    for (auto r = resolutions.begin(); r != resolutions.end(); ++r)
    {
        std::string width = std::to_string(r->x);
        std::string height = std::to_string(r->y);
        
        for (char c : width)
        {
            resolutionNames[i++] = c;
        }
        resolutionNames[i++] = ' ';
        resolutionNames[i++] = 'x';
        resolutionNames[i++] = ' ';
        for (char c : height)
        {
            resolutionNames[i++] = c;
        }
        resolutionNames[i++] = '\0';
        
        if (r->x == rwSize.x && r->y == rwSize.y)
            currentResolution = std::distance(resolutions.begin(),r);
    }
}

void Editor::toggle()
{
    enabled = !enabled;
    xy::Logger::log(std::string("Editor ") + (enabled ? "enabled" : "disabled"));
    
    // Modify the apps rendertarget
    if (enabled)
    {
        viewportBuffer.create(App::getRenderTarget()->getSize().x, App::getRenderTarget()->getSize().y);
        App::getActiveInstance()->setRenderTarget(&viewportBuffer);
    }
    else
    {
        App::getActiveInstance()->setRenderTarget(&App::getActiveInstance()->m_renderWindow);
    }
}

bool Editor::isEnabled()
{
    return enabled;
}

void Editor::draw()
{
    if (enabled)
    {
        // Main menu bar first.
        float menuBarHeight = 0;
        if (ImGui::BeginMainMenuBar())
        {
            // Some stuff will go here...
            
            menuBarHeight = ImGui::GetWindowHeight();
            
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::BeginMenu("Windows"))
                {
                    ImGui::MenuItem("Style", "s", &shouldShowStyleEditor);
                    ImGui::MenuItem("Video", "v", &shouldShowVideoSettings);
                    ImGui::MenuItem("Audio", "a", &shouldShowAudioSettings);
                    ImGui::MenuItem("Console", "c", &shouldShowConsole);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            
            
            ImGui::EndMainMenuBar();
        }
        
        // Create an imgui window covering the entire renderwindow (minus the menu bar)
        auto rwSize = App::getActiveInstance()->m_renderWindow.getSize();
        ImGui::SetNextWindowSize({static_cast<float>(rwSize.x), rwSize.y - menuBarHeight});
        ImGui::SetNextWindowPos({0,menuBarHeight});
        
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar );
        ImGui::BeginDockspace();
        
        // Draw any other registered editable objects
        for (auto& e : editables)
        {
            e->editorDraw();
        }
        
        // Show viewport
        ImGui::SetNextDock(ImGuiDockSlot_Right); // should only affect first run
        if (ImGui::BeginDock("Viewport", nullptr, ImGuiDockSlot_Right))
        {
            // Make the viewport scale to available space
            auto vpw = viewportBuffer.getSize().x;
            auto vph = viewportBuffer.getSize().y;
            auto availablew = ImGui::GetContentRegionAvailWidth();
            auto availableh = ImGui::GetContentRegionAvail().y;
            auto scale = std::min(availablew / vpw, availableh / vph);
            auto size = static_cast<sf::Vector2f>(viewportBuffer.getSize()) * scale;
            ImGui::Image(viewportBuffer, size);
        }
        ImGui::EndDock();
        
        // Show asset browser
        ImGui::SetNextDock(ImGuiDockSlot_Bottom); // should only affect first run
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
        
        // Style editor
        if (shouldShowStyleEditor)
        {
            showStyleEditor();
        }
        
        // Video settings
        if (shouldShowVideoSettings)
        {
            showVideoSettings();
        }
        
        // Audio settings
        if (shouldShowAudioSettings)
        {
            showAudioSettings();
        }
        
        if (shouldShowConsole)
        {
            Console::draw();
        }
        
        ImGui::EndDockspace();
        ImGui::End(); // Editor
    }
}

void Editor::showStyleEditor()
{
    if (ImGui::BeginDock("Style Editor"))
    {
    
        // You could cache the style to improve performance if needed
        auto style = Nim::getStyle();
        
        Nim::slider("Alpha", style.Alpha, 0.f, 1.f);
        Nim::checkbox("Antialiased fill", &style.AntiAliasedFill);
        Nim::checkbox("Antialiased lines", &style.AntiAliasedLines);
        Nim::slider("Window rounding", style.WindowRounding, 0.f, 100.f);
        
        Nim::setStyle(style);
        
        if (Nim::button("Save"))
        {
            style.saveToFile(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + "style.cfg");
        }
    }
    ImGui::EndDock();
}

void Editor::showVideoSettings()
{
    if (ImGui::BeginDock("Video"))
    {
        // Get the video settings
        auto settings = App::getActiveInstance()->getVideoSettings();
        
        fullScreen = (settings.WindowStyle & sf::Style::Fullscreen) == sf::Style::Fullscreen;
        vSync = settings.VSync;
        
        ImGui::Combo("Resolution", &currentResolution, resolutionNames.data());
        
        XY_ASSERT(App::getRenderTarget(), "no valid render target");
        
        ImGui::Checkbox("Full Screen", &fullScreen);
        
        ImGui::Checkbox("V-Sync", &vSync);
        if (vSync)
        {
            useFrameLimit = false;
        }
        
        ImGui::Checkbox("Limit Framerate", &useFrameLimit);
        if (useFrameLimit)
        {
            vSync = false;
        }
        
        ImGui::SameLine();
        ImGui::PushItemWidth(80.f);
        ImGui::InputInt("Frame Rate", &frameLimit);
        ImGui::PopItemWidth();
        frameLimit = std::max(10, std::min(frameLimit, 360));
        
        if (ImGui::Button("Apply", { 50.f, 20.f }))
        {
            //apply settings
            auto settings = App::getActiveInstance()->getVideoSettings();
            settings.VideoMode.width = resolutions[currentResolution].x;
            settings.VideoMode.height = resolutions[currentResolution].y;
            settings.WindowStyle = (fullScreen) ? sf::Style::Fullscreen : sf::Style::Close;
            settings.VSync = vSync;
            settings.FrameLimit = useFrameLimit ? frameLimit : 0;
            
            App::getActiveInstance()->applyVideoSettings(settings);
        }
    }
    ImGui::EndDock();
}

void Editor::showAudioSettings()
{
    if (ImGui::BeginDock("Audio"))
    {
        ImGui::Text("NOTE: only AudioSystem sounds are affected.");
        
        static float maxVol = AudioMixer::getMasterVolume();
        ImGui::SliderFloat("Master", &maxVol, 0.f, 1.f);
        AudioMixer::setMasterVolume(maxVol);
        
        static std::array<float, AudioMixer::MaxChannels> channelVol;
        for (auto i = 0u; i < AudioMixer::MaxChannels; ++i)
        {
            channelVol[i] = AudioMixer::getVolume(i);
            ImGui::SliderFloat(AudioMixer::getLabel(i).c_str(), &channelVol[i], 0.f, 1.f);
            AudioMixer::setVolume(channelVol[i], i);
        }
    }
    ImGui::EndDock();
}

