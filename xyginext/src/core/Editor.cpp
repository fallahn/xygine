/*********************************************************************
 (c) Jonny Paton 2017 - 2018
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

#include "xyginext/core/Editor.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/graphics/SpriteSheet.hpp"
#include "xyginext/gui/Gui.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/resources/Resource.hpp"
#include "xyginext/ecs/components/ParticleEmitter.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include "../imgui/imgui_dock.hpp"
#include "../imgui/imgui-SFML.h"

#include <map>
#include <set>

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
    bool shouldShowSceneEditor = false;
    sf::RenderTexture viewportBuffer;
    int currentResolution = 0;
    std::array<char, 300> resolutionNames{};
    std::vector<sf::Vector2u> resolutions;
    bool fullScreen = false;
    bool vSync = false;
    bool useFrameLimit = false;
    int frameLimit = 10;
    
    TextureResource textureResource;
    
    // For asset editing
    std::string selectedAsset;
    
    // Any scenes which have the EditorSystem added
    std::unordered_map<std::string, xy::Scene*> editableScenes;
    static int sceneCounter(0);
    std::string selectedSceneName = "Select a scene";
    Scene*  selectedScene = nullptr;
    
    // Asset lists
    std::map<std::string, SpriteSheet> spriteSheets;
    std::map<std::string, ParticleEmitter> particleEmitters;
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, std::fstream> textFiles;
    std::set<std::string> tmxFiles;
    std::map<std::string, sf::SoundBuffer> sounds;
}

EditorSystem::EditorSystem(xy::MessageBus& mb, const std::string& sceneName) :
xy::System(mb, typeid(this))
{
    requireComponent<Editable>();
    
    // Register the scene we've been added to with the editor
    auto name = sceneName;
    if (name.empty())
    {
        name = "Unnamed scene " + std::to_string(sceneCounter++);
    }
    
    editableScenes[name] = getScene();
}

EditorSystem::~EditorSystem()
{
    auto scene = getScene();
    auto me = std::find_if(editableScenes.begin(),editableScenes.end(),[scene](const std::pair<std::string, xy::Scene*>& m)
                           { return m.second == scene; });
    
    if (me != editableScenes.end())
    {
        editableScenes.erase(me);
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
    
    // Load any assets
    std::function<void(std::string)> assetSearch = [&](std::string path)
    {
        // Search subdirectories
        for (auto& dir : xy::FileSystem::listDirectories(path))
        {
            assetSearch(path + "/" + dir);
        }
        
        // Save assets
        for (auto& file : xy::FileSystem::listFiles(path))
        {
            auto ext = FileSystem::getFileExtension(file);
            auto filePath = path + "/" + file;
            
            // Would prefer a better way to handle file types - enum?
            if (ext == ".spt")
            {
                spriteSheets[file].loadFromFile(filePath, textureResource);
            }
            else if (ext == ".xyp")
            {
                particleEmitters[file].settings.loadFromFile(filePath, textureResource);
            }
            else if (ext == ".png")
            {
                textures[file].loadFromFile(filePath);
            }
            else if (ext == ".ttf")
            {
                fonts[file].loadFromFile(filePath);
            }
            else if (ext == ".txt")
            {
                textFiles[file].open(filePath);
            }
            else if (ext == ".tmx")
            {
                tmxFiles.emplace(filePath);
            }
            else if (ext == ".wav" || ext == ".ogg")
            {
                sounds[file].loadFromFile(filePath);
            }
            else
            {
                Logger::log("Resource not recognised: " + file);
            }
            
        }
    };
    
    assetSearch("assets");
    
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
                    ImGui::MenuItem("Scenes", "I need better shortcuts", &shouldShowSceneEditor);
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
        if (ImGui::BeginDock("Viewport", nullptr))
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
        
        // Console
        if (shouldShowConsole)
        {
            Console::draw();
        }
        
        // Scene editor
        if (shouldShowSceneEditor)
        {
            showSceneEditor();
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
    if (ImGui::BeginDock("Assets"))
    {
       // Show all assets
        if (ImGui::TreeNode("SpriteSheets"))
        {
            for (auto ss : spriteSheets)
            {
                // Show the texture as a preview
                auto& tex = textureResource.get(ss.second.getTexturePath());
                const sf::Vector2f PreviewSize = {50.f,50.f};
                if (ImGui::ImageButton(tex, PreviewSize))
                {
                    
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndDock();
}

void Editor::showSpriteEditor()
{
    
}

void Editor::showSceneEditor()
{
    if (ImGui::BeginDock("Scenes"))
    {
        // Show combo box to select scene first
        if (ImGui::BeginCombo("Scenes", selectedSceneName.c_str()))
        {
            for (auto& s : editableScenes)
            {
                if (ImGui::Selectable(s.first.c_str()))
                {
                    selectedSceneName = s.first;
                    selectedScene = s.second;
                }
            }
            ImGui::EndCombo();
        }
        
        if (selectedScene)
        {
        }
        
    }
    ImGui::EndDock();
}

void Editor::registerScene(Scene* scene, const std::string& name)
{
    editableScenes[name] = scene;
}

void Editor::deregisterScene(Scene* scene)
{
    auto me = std::find_if(editableScenes.begin(),editableScenes.end(),[scene](const std::pair<std::string, xy::Scene*>& m)
                           { return m.second == scene; });
    
    if (me != editableScenes.end())
    {
        editableScenes.erase(me);
    }
}
