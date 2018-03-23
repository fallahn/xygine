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

#include <unordered_map>

using namespace xy;

namespace
{
    bool enabled = false;
    
    // bc name clash with function and bc lazy...
    bool shouldShowStyleEditor = false;
    bool shouldShowVideoSettings = false;
    bool shouldShowAudioSettings = false;
    bool shouldShowConsole = false;
    bool shouldShowAssetBrowser = false;
    sf::RenderTexture viewportBuffer;
    int currentResolution = 0;
    std::array<char, 300> resolutionNames{};
    std::vector<sf::Vector2u> resolutions;
    bool fullScreen = false;
    bool vSync = false;
    bool useFrameLimit = false;
    int frameLimit = 10;
    
    // For asset editing
    std::string selectedAsset;
    
    // Any scenes which have the EditorSystem added
    std::unordered_map<std::string, xy::Scene*> m_editableScenes;
    static int sceneCounter(0);
}

EditorSystem::EditorSystem(xy::MessageBus& mb, const std::string& sceneName) :
xy::System(mb, typeid(this))
{
    requireComponent<Editable>();
    
    // Register the scene we've been added to with the editor
    auto name = sceneName;
    if (name.empty())
    {
        name = "Scene " + std::to_string(sceneCounter++);
    }
        
    m_editableScenes[name] = getScene();
    
}

EditorSystem::~EditorSystem()
{
    auto scene = getScene();
    auto me = std::find_if(m_editableScenes.begin(),m_editableScenes.end(),[scene](const std::pair<std::string, xy::Scene*>& m)
                           { return m.second == scene; });
                           
    if (me != m_editableScenes.end())
    {
        m_editableScenes.erase(me);
    }
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
            
            menuBarHeight = ImGui::GetWindowHeight();
            
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Quit","q"))
                {
                    App::quit();
                }
                ImGui::EndMenu();
            }
            
            
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::BeginMenu("Windows"))
                {
                    ImGui::MenuItem("Style", "s", &shouldShowStyleEditor);
                    ImGui::MenuItem("Video", "v", &shouldShowVideoSettings);
                    ImGui::MenuItem("Audio", "a", &shouldShowAudioSettings);
                    ImGui::MenuItem("Console", "c", &shouldShowConsole);
                    ImGui::MenuItem("Assets", "erm...", &shouldShowAssetBrowser);
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
        
        // Asset browser
        if (shouldShowAssetBrowser)
        {
            showAssetBrowser();
        }
        
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

void Editor::showAssetBrowser()
{
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
                bool selected = selectedAsset == file;
                if (ImGui::Selectable(file.c_str(), &selected))
                {
                    selectedAsset = file;
                }
            }
        };
        
        imFileTreeRecurse("assets");
    }
    
    ImGui::EndDock();
}

void Editor::showSpriteEditor()
{
    
}


