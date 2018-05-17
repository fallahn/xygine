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
#include "xyginext/core/editor/SpriteEditor.hpp"
#include "xyginext/core/editor/SceneEditor.hpp"
#include "xyginext/core/editor/ParticleEditor.hpp"
#include "xyginext/core/editor/TextureEditor.hpp"
#include "xyginext/core/editor/FontEditor.hpp"
#include "xyginext/core/editor/IconFontAwesome5.hpp"
#include "xyginext/core/Log.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/core/ConfigFile.hpp"
#include "xyginext/graphics/SpriteSheet.hpp"
#include "xyginext/gui/Gui.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/resources/Resource.hpp"
#include "xyginext/ecs/components/ParticleEmitter.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Sprite.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/systems/RenderSystem.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"
#include "xyginext/ecs/systems/CameraSystem.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio/Sound.hpp>

#include "../../imgui/imgui_dock.hpp"
#include "../../imgui/imgui-SFML.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/fonts/fa-solid-900.hpp"

#include <map>
#include <set>

using namespace xy;

namespace
{
    // Whether the editor is enabled or not
    bool enabled = false;
    
    // Flags whether to show certain editor windows
    bool shouldShowStyleEditor = false;
    bool shouldShowVideoSettings = false;
    bool shouldShowAudioSettings = false;
    bool shouldShowConsole = false;
    bool shouldShowAssetBrowser = false;
    bool shouldOpenNewPopup = false;
    bool shouldShowSettings = false;
    bool shouldShowViewport = true;
    
    // Buffer which the game renders to while the editor is active
    sf::RenderTexture viewportBuffer;
    
    // Video settings
    int currentResolution = 0;
    std::array<char, 300> resolutionNames{};
    std::vector<sf::Vector2u> resolutions;
    bool fullScreen = false;
    bool vSync = false;
    bool useFrameLimit = false;
    int frameLimit = 10;
    
    // Editor textures
    xy::TextureResource textureResource;
    
    // Editor settings
    int snapInterval = 8; // value used when selecting tex rects etc.
    
    // map of assets, keyed by the file name
    std::vector<std::unique_ptr<EditorAsset>> assets;
    
    // To play sounds in the editor
    std::vector<sf::Sound> playingSounds;
    
    // The different cursors we may use
    sf::Cursor moveCursor;
    sf::Cursor sizeTLBRCursor;
    sf::Cursor sizeTRBLCursor;
    sf::Cursor sizeXCursor;
    sf::Cursor sizeYCursor;
    sf::Cursor arrowCursor;
}

EditorSystem::EditorSystem(xy::MessageBus& mb, const std::string& sceneName) :
xy::System(mb, typeid(EditorSystem)),
m_sceneName(sceneName)
{
    // The editor system doesn't require any components
    // This means (intentionally or otherwise) All entities are added to the system
    // And it gives us a nice interface to access all entities in a scene
}

void EditorSystem::onCreate()
{
    // If no scene name provided, use a default one
    /*if (m_sceneName.empty())
    {
        m_sceneName = "Unnamed scene " + std::to_string(sceneCounter++);
        scenes[m_sceneName].scene = getScene();
    }
    
    // Otherwise, check if the scene name matches an asset, in which case link it
    // Could do with a better way of matching up user-created scenes to the assets
    else if (scenes.find(m_sceneName) != scenes.end())
    {
        scenes.find(m_sceneName)->second.scene = getScene();
    }*/
    
}

EditorSystem::~EditorSystem()
{
    /*auto scene = getScene();
    auto me = std::find_if(scenes.begin(),scenes.end(),[scene](const std::pair<std::string, SceneAsset>& m)
                           { return m.second.scene == scene; });
    
    if (me != scenes.end())
    {
        scenes.erase(me);
    }*/
}

void Editor::init()
{
    // We want imgui keyboard nav
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load dock settings
    ImGui::InitDock();
    
    // Merge in fontawesome icons
    auto& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF( fa_solid_900_ttf, fa_solid_900_ttf_len,  16.0f, &icons_config, icons_ranges );
    ImGui::SFML::UpdateFontTexture();
    
    // load cursors
    moveCursor.loadFromSystem(sf::Cursor::SizeAll);
    sizeTLBRCursor.loadFromSystem(sf::Cursor::SizeTopLeftBottomRight);
    sizeTRBLCursor.loadFromSystem(sf::Cursor::SizeBottomLeftTopRight);
    sizeXCursor.loadFromSystem(sf::Cursor::SizeHorizontal);
    sizeYCursor.loadFromSystem(sf::Cursor::SizeVertical);
    arrowCursor.loadFromSystem(sf::Cursor::Arrow);
    
    
    // Load all the video modes
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
    
    auto settings = App::getActiveInstance()->getVideoSettings();
    fullScreen = (settings.WindowStyle & sf::Style::Fullscreen) == sf::Style::Fullscreen;
    vSync = settings.VSync;
    
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
                assets.emplace_back(std::make_unique<SpriteSheetAsset>());
                auto newSheetAsset = dynamic_cast<SpriteSheetAsset*>(assets.back().get());
                newSheetAsset->sheet.loadFromFile(filePath, textureResource);
                newSheetAsset->m_path = filePath;
            }
            else if (ext == ".xyp")
            {
                // particle asset
                assets.emplace_back(std::make_unique<ParticleEmitterAsset>());
                auto newEmitter = dynamic_cast<ParticleEmitterAsset*>(assets.back().get());
                newEmitter->m_path = filePath;
                newEmitter->settings.loadFromFile(filePath, textureResource);
            }
            else if (ext == ".png")
            {
                // texture asset
                assets.emplace_back(std::make_unique<TextureAsset>());
                auto newTex = dynamic_cast<TextureAsset*>(assets.back().get());
                newTex->texture.loadFromFile(filePath);
                newTex->m_path = filePath;
            }
            else if (ext == ".ttf")
            {
                assets.emplace_back(std::make_unique<FontAsset>());
                auto newFont = dynamic_cast<FontAsset*>(assets.back().get());
                newFont->m_path = filePath;
                newFont->font.loadFromFile(filePath);
            }
            else if (ext == ".wav" || ext == ".ogg")
            {
                // audio asset
            }
            else if (ext == ".xyscn")
            {
                assets.emplace_back(std::make_unique<SceneAsset>());
                auto newScene = dynamic_cast<SceneAsset*>(assets.back().get());
                newScene->m_path = filePath;
                
                newScene->scene.reset(new Scene(App::getActiveInstance()->getMessageBus(), filePath));
                
                // Add the editor system
                newScene->scene->addSystem<EditorSystem>(App::getActiveInstance()->getMessageBus(), "Debug Scene!");
                newScene->scene->addSystem<CameraSystem>(App::getActiveInstance()->getMessageBus());
                
            }
            else
            {
                Logger::log("Resource not recognised: " + file);
            }
            
        }
    };
    
    // Check resource path and working directory (primarily required for apple...)
    //assetSearch(FileSystem::getResourcePath() + "assets");
    assetSearch("assets");
    
    // Check for imgui style config file
    auto stylePath = FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + "style.cfg";
    Nim::Style style;
    if (style.loadFromFile(stylePath))
    {
        Nim::setStyle(style);
    }
    
    // Check for an editor settings file
    ConfigFile editorSettings;
    if (editorSettings.loadFromFile(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + "editor.cfg"))
    {
        ConfigProperty* p;
        // Open window flags
        if ( p = editorSettings.findProperty("assetsOpen"))
        {
            shouldShowAssetBrowser = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("consoleOpen"))
        {
            shouldShowConsole = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("styleOpen"))
        {
            shouldShowStyleEditor = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("audioOpen"))
        {
            shouldShowAudioSettings = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("videoOpen"))
        {
            shouldShowVideoSettings = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("settingsOpen"))
        {
            shouldShowSettings = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("viewportOpen"))
        {
            shouldShowViewport = p->getValue<bool>();
        }
        if ( p = editorSettings.findProperty("snap"))
        {
            snapInterval = p->getValue<int>();
        }
        
        // Check for any open spritesheets
        // Would be better to just check any open assets?
        ConfigObject* o;
        if (o = editorSettings.findObjectWithName("OpenAssets"))
        {
            for (auto p : o->getProperties())
            {
                auto name = p.getValue<std::string>();
                auto ss = std::find_if(assets.begin(), assets.end(),
                                    [name](const std::unique_ptr<EditorAsset>& ass)
                {
                    return ass->m_path == name;
                });
                if (ss != assets.end())
                {
                    (*ss)->m_open = true;
                }
            }
        }
    }
}

void Editor::shutdown()
{
    // Save the editor config
    ConfigFile editorSettings;
    editorSettings.addProperty("assetsOpen").setValue(shouldShowAssetBrowser);
    editorSettings.addProperty("consoleOpen").setValue(shouldShowConsole);
    editorSettings.addProperty("styleOpen").setValue(shouldShowStyleEditor);
    editorSettings.addProperty("audioOpen").setValue(shouldShowAudioSettings);
    editorSettings.addProperty("videoOpen").setValue(shouldShowVideoSettings);
    editorSettings.addProperty("settingsOpen").setValue(shouldShowSettings);
    editorSettings.addProperty("viewportOpen").setValue(shouldShowViewport);
    
    // store any open spritesheets
    auto o = editorSettings.addObject("OpenAssets");
    for (auto& ss : assets)
    {
        if (ss->m_open)
        {
            // config item array required?
            o->addProperty("name").setValue(ss->m_path);
        }
    }
    
    editorSettings.addProperty("snap").setValue(snapInterval);
    
    // Finally save
    editorSettings.save(FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + "editor.cfg");
    
    for (auto& asset : assets)
    {
        asset.release();
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
        // Probably shouldn't hardcode this
        // But it's required to make sure the fontawesome font is used
        // I may well be doing something wrong to cause this
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        
        // Main menu bar first.
        float menuBarHeight = 0;
        if (ImGui::BeginMainMenuBar())
        {
            
            menuBarHeight = ImGui::GetWindowHeight();
            
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Quit","ctrl + q"))
                {
                    App::quit();
                }
                
                if (ImGui::MenuItem("New Asset", "ctrl + n"))
                {
                    shouldOpenNewPopup = true;
                }
                
                ImGui::EndMenu();
            }
            
            
            if (ImGui::BeginMenu("View"))
            {
                if (ImGui::BeginMenu("Windows"))
                {
                    ImGui::MenuItem("Style", "", &shouldShowStyleEditor);
                    ImGui::MenuItem("Video", "", &shouldShowVideoSettings);
                    ImGui::MenuItem("Audio", "", &shouldShowAudioSettings);
                    ImGui::MenuItem("Console", "ctrl + c", &shouldShowConsole);
                    ImGui::MenuItem("Assets", "ctrl + a", &shouldShowAssetBrowser);
                    ImGui::MenuItem("Viewport", "", &shouldShowViewport);
                    
                    ImGui::Separator();
                    ImGui::MenuItem("Editor settings","",&shouldShowSettings);
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
        if (ImGui::BeginDock("Viewport", &shouldShowViewport))
        {
            // Make the viewport scale to available space
            auto vpw = viewportBuffer.getSize().x;
            auto vph = viewportBuffer.getSize().y;
            auto availablew = ImGui::GetContentRegionAvailWidth();
            auto availableh = ImGui::GetContentRegionAvail().y;
            auto scale = std::min(availablew / vpw, availableh / vph);
            auto size = static_cast<sf::Vector2f>(viewportBuffer.getSize()) * scale;
            viewportBuffer.display();
            ImGui::Image(viewportBuffer, size);
        }
        ImGui::EndDock();
        
        showAssetBrowser();
        showStyleEditor();
        showVideoSettings();
        showAudioSettings();
        if (shouldShowConsole)
            Console::draw();
        showSettings();
        
        // Show any sprites open for editing
        for (auto& asset : assets)
        {
            if (ImGui::BeginDock(asset->m_path.c_str(), &asset->m_open))
            {
                asset->edit();
            }
            ImGui::EndDock();
        }
        
        ImGui::EndDockspace();
        
        // Show any open modals
        showModalPopups();
        
        ImGui::End(); // Editor
        
        ImGui::PopFont();
        
        // If any scene assets are open, draw them
        for (auto& asset : assets)
        {
            if (asset->getType() == AssetType::Scene)
            {
                if (asset->m_open)
                {
                    App::getActiveInstance()->getRenderTarget()->draw(*(dynamic_cast<SceneAsset*>(asset.get())->scene));
                }
            }
        }
    }
    
    // Check for cursor type changes here
    switch (ImGui::GetMouseCursor())
    {
        case ImGuiMouseCursor_ResizeAll:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(moveCursor);
            break;
        }
        case ImGuiMouseCursor_ResizeNESW:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(sizeTRBLCursor);
            break;
        }
        case ImGuiMouseCursor_ResizeNWSE:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(sizeTLBRCursor);
            break;
        }
        case ImGuiMouseCursor_ResizeNS:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(sizeYCursor);
            break;
        }
        case ImGuiMouseCursor_ResizeEW:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(sizeXCursor);
            break;
        }
        default:
        {
            App::getActiveInstance()->m_renderWindow.setMouseCursor(arrowCursor);
            break;
        }
    }
}

bool Editor::handleEvent(sf::Event& ev)
{
    if (!enabled)
    {
        return false;
    }
    
    switch(ev.type)
    {
            // Keyboard shortcuts
        case sf::Event::KeyPressed:
        {
            switch(ev.key.code)
            {
                case sf::Keyboard::A:
                {
                    if (ev.key.control)
                    {
                        shouldShowAssetBrowser = !shouldShowAssetBrowser;
                    }
                }
                break;
                case sf::Keyboard::C:
                {
                    if (ev.key.control)
                    {
                        shouldShowConsole = !shouldShowConsole;
                    }
                }
                break;
                    
                case sf::Keyboard::N:
                {
                    if (ev.key.control)
                    {
                        shouldOpenNewPopup = true;
                    }
                }
                break;
                case sf::Keyboard::Q:
                {
                    if (ev.key.control)
                    {
                        App::quit();
                    }
                }
                break;
            }
            break;
        }
    }
    
    return false;
}

void Editor::update(float dt)
{
    // Update any debug scenes
    for (auto& asset : assets)
    {
        if (asset->getType() == AssetType::Scene)
        {
            if (asset->m_open)
            {
                dynamic_cast<SceneAsset*>(asset.get())->scene->update(dt);
            }
        }
    }
}

void Editor::showStyleEditor()
{
    if (ImGui::BeginDock("Style Editor", &shouldShowStyleEditor))
    {
        // Path to save to
        auto savePath = FileSystem::getConfigDirectory(App::getActiveInstance()->getApplicationName()) + "style.cfg";
        
        // You can pass in a reference ImGuiStyle structure to compare to, revert to and save to (else it compares to an internally stored reference)
        auto style = Nim::getStyle();
        
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);
        
        ImGui::ShowFontSelector("Fonts##Selector");
        
        // Simplified Settings
        if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
            style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
        { bool window_border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &window_border)) style.WindowBorderSize = window_border ? 1.0f : 0.0f; }
        ImGui::SameLine();
        { bool frame_border = (style.FrameBorderSize > 0.0f); if (ImGui::Checkbox("FrameBorder", &frame_border)) style.FrameBorderSize = frame_border ? 1.0f : 0.0f; }
        ImGui::SameLine();
        { bool popup_border = (style.PopupBorderSize > 0.0f); if (ImGui::Checkbox("PopupBorder", &popup_border)) style.PopupBorderSize = popup_border ? 1.0f : 0.0f; }
        
        // Save/Revert button
        if (ImGui::Button("Save"))
            style.saveToFile(savePath);
        
        if (ImGui::TreeNode("Rendering"))
        {
            ImGui::Checkbox("Anti-aliased lines", &style.AntiAliasedLines);
            ImGui::Checkbox("Anti-aliased fill", &style.AntiAliasedFill);
            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol, 0.02f, 0.10f, FLT_MAX, NULL, 2.0f);
            if (style.CurveTessellationTol < 0.0f) style.CurveTessellationTol = 0.10f;
            ImGui::DragFloat("Global Alpha", &style.Alpha, 0.005f, 0.20f, 1.0f, "%.2f"); // Not exposing zero here so user doesn't "lose" the UI (zero alpha clips all widgets). But application code could have a toggle to switch between zero and non-zero.
            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Settings"))
        {
            ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 16.0f, "%.0f");
            ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
            ImGui::Text("BorderSize");
            ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
            ImGui::Text("Rounding");
            ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 14.0f, "%.0f");
            ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 16.0f, "%.0f");
            ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
            ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
            ImGui::Text("Alignment");
            ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Colors"))
        {
            ImGui::Text("Tip: Left-click on colored square to open color picker,\nRight-click to open edit options menu.");
            
            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", 200);
            
            static ImGuiColorEditFlags alpha_flags = 0;
            ImGui::RadioButton("Opaque", &alpha_flags, 0); ImGui::SameLine();
            ImGui::RadioButton("Alpha", &alpha_flags, ImGuiColorEditFlags_AlphaPreview); ImGui::SameLine();
            ImGui::RadioButton("Both", &alpha_flags, ImGuiColorEditFlags_AlphaPreviewHalf);
            
            ImGui::BeginChild("#colors", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < static_cast<int>(Nim::Style::Colour::Count); i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImVec4 col = style.colours[i];
                if (ImGui::ColorEdit4("##color", (float*)&col, ImGuiColorEditFlags_AlphaBar | alpha_flags))
                {
                    style.colours[i] = col;
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();
            
            ImGui::TreePop();
        }
        
        bool fonts_opened = ImGui::TreeNode("Fonts", "Fonts (%d)", ImGui::GetIO().Fonts->Fonts.Size);
        if (fonts_opened)
        {
            ImFontAtlas* atlas = ImGui::GetIO().Fonts;
            if (ImGui::TreeNode("Atlas texture", "Atlas texture (%dx%d pixels)", atlas->TexWidth, atlas->TexHeight))
            {
                ImGui::Image(atlas->TexID, ImVec2((float)atlas->TexWidth, (float)atlas->TexHeight), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                ImGui::TreePop();
            }
            ImGui::PushItemWidth(100);
            for (int i = 0; i < atlas->Fonts.Size; i++)
            {
                ImFont* font = atlas->Fonts[i];
                ImGui::PushID(font);
                bool font_details_opened = ImGui::TreeNode(font, "Font %d: \'%s\', %.2f px, %d glyphs", i, font->ConfigData ? font->ConfigData[0].Name : "", font->FontSize, font->Glyphs.Size);
                ImGui::SameLine(); if (ImGui::SmallButton("Set as default")) ImGui::GetIO().FontDefault = font;
                if (font_details_opened)
                {
                    ImGui::PushFont(font);
                    ImGui::Text("The quick brown fox jumps over the lazy dog");
                    ImGui::PopFont();
                    ImGui::DragFloat("Font scale", &font->Scale, 0.005f, 0.3f, 2.0f, "%.1f");   // Scale only this font
                    ImGui::Text("Ascent: %f, Descent: %f, Height: %f", font->Ascent, font->Descent, font->Ascent - font->Descent);
                    ImGui::Text("Fallback character: '%c' (%d)", font->FallbackChar, font->FallbackChar);
                    ImGui::Text("Texture surface: %d pixels (approx) ~ %dx%d", font->MetricsTotalSurface, (int)sqrtf((float)font->MetricsTotalSurface), (int)sqrtf((float)font->MetricsTotalSurface));
                    for (int config_i = 0; config_i < font->ConfigDataCount; config_i++)
                    {
                        ImFontConfig* cfg = &font->ConfigData[config_i];
                        ImGui::BulletText("Input %d: \'%s\', Oversample: (%d,%d), PixelSnapH: %d", config_i, cfg->Name, cfg->OversampleH, cfg->OversampleV, cfg->PixelSnapH);
                    }
                    if (ImGui::TreeNode("Glyphs", "Glyphs (%d)", font->Glyphs.Size))
                    {
                        // Display all glyphs of the fonts in separate pages of 256 characters
                        const ImFontGlyph* glyph_fallback = font->FallbackGlyph; // Forcefully/dodgily make FindGlyph() return NULL on fallback, which isn't the default behavior.
                        font->FallbackGlyph = NULL;
                        for (int base = 0; base < 0x10000; base += 256)
                        {
                            int count = 0;
                            for (int n = 0; n < 256; n++)
                                count += font->FindGlyph((ImWchar)(base + n)) ? 1 : 0;
                            if (count > 0 && ImGui::TreeNode((void*)(intptr_t)base, "U+%04X..U+%04X (%d %s)", base, base+255, count, count > 1 ? "glyphs" : "glyph"))
                            {
                                float cell_spacing = style.ItemSpacing.y;
                                ImVec2 cell_size(font->FontSize * 1, font->FontSize * 1);
                                ImVec2 base_pos = ImGui::GetCursorScreenPos();
                                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                                for (int n = 0; n < 256; n++)
                                {
                                    ImVec2 cell_p1(base_pos.x + (n % 16) * (cell_size.x + cell_spacing), base_pos.y + (n / 16) * (cell_size.y + cell_spacing));
                                    ImVec2 cell_p2(cell_p1.x + cell_size.x, cell_p1.y + cell_size.y);
                                    const ImFontGlyph* glyph = font->FindGlyph((ImWchar)(base+n));;
                                    draw_list->AddRect(cell_p1, cell_p2, glyph ? IM_COL32(255,255,255,100) : IM_COL32(255,255,255,50));
                                    font->RenderChar(draw_list, cell_size.x, cell_p1, ImGui::GetColorU32(ImGuiCol_Text), (ImWchar)(base+n)); // We use ImFont::RenderChar as a shortcut because we don't have UTF-8 conversion functions available to generate a string.
                                    if (glyph && ImGui::IsMouseHoveringRect(cell_p1, cell_p2))
                                    {
                                        ImGui::BeginTooltip();
                                        ImGui::Text("Codepoint: U+%04X", base+n);
                                        ImGui::Separator();
                                        ImGui::Text("AdvanceX: %.1f", glyph->AdvanceX);
                                        ImGui::Text("Pos: (%.2f,%.2f)->(%.2f,%.2f)", glyph->X0, glyph->Y0, glyph->X1, glyph->Y1);
                                        ImGui::Text("UV: (%.3f,%.3f)->(%.3f,%.3f)", glyph->U0, glyph->V0, glyph->U1, glyph->V1);
                                        ImGui::EndTooltip();
                                    }
                                }
                                ImGui::Dummy(ImVec2((cell_size.x + cell_spacing) * 16, (cell_size.y + cell_spacing) * 16));
                                ImGui::TreePop();
                            }
                        }
                        font->FallbackGlyph = glyph_fallback;
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            static float window_scale = 1.0f;
            ImGui::DragFloat("this window scale", &window_scale, 0.005f, 0.3f, 2.0f, "%.1f");              // scale only this window
            ImGui::DragFloat("global scale", &ImGui::GetIO().FontGlobalScale, 0.005f, 0.3f, 2.0f, "%.1f"); // scale everything
            ImGui::PopItemWidth();
            ImGui::SetWindowFontScale(window_scale);
            ImGui::TreePop();
        }
        ImGui::PopItemWidth();
        Nim::setStyle(style);
        ImGui::EndDock();
    }
}

void Editor::showVideoSettings()
{
    if (ImGui::BeginDock("Video", &shouldShowVideoSettings))
    {
        // Get the video settings
        auto settings = App::getActiveInstance()->getVideoSettings();
        
        
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
    if (ImGui::BeginDock("Audio", &shouldShowAudioSettings))
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
        ImGui::EndDock();
    }
}

void Editor::showAssetBrowser()
{
    if (ImGui::BeginDock("Assets", &shouldShowAssetBrowser))
    {
        for (auto& asset : assets)
        {
            ImGui::Selectable(asset->m_path.c_str(), &asset->m_open);
        }
        ImGui::EndDock();
    }
}

void Editor::showModalPopups()
{
    if (shouldOpenNewPopup)
    {
        ImGui::OpenPopup("New Asset");
        shouldOpenNewPopup = false;
    }
    
    // New Asset
    if (ImGui::BeginPopupModal("New Asset"))
    {
        static int selectedType = 0;
        
        ImGui::RadioButton("Sprite Sheet", &selectedType, static_cast<int>(AssetType::Spritesheet));
        ImGui::RadioButton("Scene", &selectedType, static_cast<int>(AssetType::Scene));
        ImGui::RadioButton("Particle Emitter", &selectedType, static_cast<int>(AssetType::ParticleEmitter));
        
        static std::array<char,MAX_INPUT> buf = {{0}};
        if (ImGui::InputText("Path", buf.data(), MAX_INPUT, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Create"))
        {
            switch(static_cast<AssetType>(selectedType))
            {
                case AssetType::Spritesheet:
                {
                    assets.emplace_back(std::make_unique<SpriteSheetAsset>());
                    auto& ss = assets.back();
                    if (ss)
                    {
                        ss->m_path = buf.data();
                        ss->m_open = true;
                        ss->m_dirty = true;
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }
                    
                case AssetType::Scene:
                {
                    assets.emplace_back(std::make_unique<SceneAsset>());
                    auto scn = dynamic_cast<SceneAsset*>(assets.back().get());
                    if (scn)
                    {
                        scn->m_open = true;
                        scn->m_dirty = true;
                        scn->m_path = buf.data();
                        scn->scene.reset(new Scene(App::getActiveInstance()->getMessageBus()));
                        
                        // Make sure the new scene has the editor system and camera system
                        scn->scene->addSystem<EditorSystem>(App::getActiveInstance()->getMessageBus());
                        scn->scene->addSystem<CameraSystem>(App::getActiveInstance()->getMessageBus());
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }
                case AssetType::ParticleEmitter:
                {
                    assets.emplace_back(std::make_unique<ParticleEmitterAsset>());
                    auto& emitter = assets.back();
                    if (emitter)
                    {
                        emitter->m_open = true;
                        emitter->m_dirty = true;
                        emitter->m_path = buf.data();
                        ImGui::CloseCurrentPopup();
                    }
                    break;
                }
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void Editor::showSettings()
{
    // Snap to grid
    if (ImGui::BeginDock("Editor settings"), &shouldShowSettings)
    {
        ImGui::InputInt("Snap (pixels)", &snapInterval);
    }
    ImGui::EndDock();
}

int Editor::getPixelSnap()
{
    return snapInterval;
}

std::vector<std::unique_ptr<EditorAsset>>& Editor::getAssets()
{
    return assets;
}
