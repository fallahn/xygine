/*********************************************************************
 (c) Matt Marchant & Jonny Paton 2017 - 2018
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


#include "xyginext/core/editor/SceneEditor.hpp"
#include "xyginext/core/editor/SpriteEditor.hpp"
#include "xyginext/core/editor/FontEditor.hpp"
#include "xyginext/core/editor/IconFontAwesome5.hpp"
#include "xyginext/core/Editor.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/Text.hpp"
#include "xyginext/ecs/components/UIHitBox.hpp"
#include "xyginext/ecs/components/SpriteAnimation.hpp"
#include "xyginext/ecs/systems/TextRenderer.hpp"
#include "xyginext/ecs/systems/RenderSystem.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"
#include "xyginext/ecs/systems/SpriteAnimator.hpp"
#include "xyginext/ecs/systems/CameraSystem.hpp"
#include "xyginext/ecs/systems/UISystem.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"
#include "../../imgui/imgui_dock.hpp"

#include <set>

namespace xy
{
    namespace
    {
        constexpr std::size_t MAX_INPUT_CHARS = 400;
    }
    
    SceneAsset::SceneAsset() :
    scene(nullptr)
    {
    }
    
    void SceneAsset::edit()
    {
        if (scene)
        {
            // Systems
            ImGui::Text("Systems:");
            auto& systems = scene->m_systemManager.getSystems();
            for (auto& s : systems)
            {
                ImGui::TextUnformatted(s->getType().name());
            }
            
            // Entities
            auto& editorSys = scene->getSystem<EditorSystem>();
            auto ents = editorSys.getEntities();
            ImGui::Text("Entities:");
            for (auto& e : ents)
            {
                // Ignore default entity (making assumption it will be id 0,
                // as it is added in the scene constructor)
                if (e.getIndex() != 0)
                {
                    if (ImGui::TreeNode(std::to_string(e.getIndex()).c_str()))
                    {
                        ImGui::Text("Components:");
                        if (e.hasComponent<Sprite>())
                        {
                            if (ImGui::TreeNode("Sprite"))
                            {
                                // Select a sprite from the assets
                                static bool showSpriteSelector = false;
                                if (ImGui::Button("Select from asset"))
                                {
                                    showSpriteSelector = true;
                                    ImGui::OpenPopup("Select Sprite from assets"); // super fragile
                                }
                                if (showSpriteSelector)
                                {
                                    auto spr = SpriteEditor::selectSpriteFromAssets();
                                    
                                    if (spr)
                                    {
                                        showSpriteSelector = false;
                                        e.getComponent<Sprite>() = *spr;
                                        e.addComponent<Drawable>();
                                        if (!e.hasComponent<Transform>())
                                        {
                                            // don't reset transform unnecessarily
                                            e.addComponent<Transform>();
                                        }
                                    }
                                }
                                ImGui::TreePop();
                            }
                            
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<Sprite>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        if (e.hasComponent<SpriteAnimation>())
                        {
                            if (ImGui::TreeNode("Sprite Animation"))
                            {
                                auto& anim = e.getComponent<SpriteAnimation>();
                                auto spr = e.getComponent<Sprite>();
                                
                                // select anim
                                static int selectedIndex(-1); // bad
                                if (ImGui::BeginCombo("Animations", selectedIndex > -1 ? spr.getAnimations()[selectedIndex].id.data() : "Select an animation"))
                                {
                                    int index(0);
                                    for (auto a : spr.getAnimations())
                                    {
                                        if (ImGui::Selectable(a.id.data()))
                                        {
                                            anim.play(index);
                                            selectedIndex = index;
                                        }
                                        index++;
                                    }
                                    ImGui::EndCombo();
                                }
                                
                                // Play/pause/stop controls
                                if (anim.stopped())
                                {
                                    if (ImGui::Button(ICON_FA_PLAY))
                                    {
                                        anim.play(selectedIndex);
                                    }
                                }
                                else
                                {
                                    if (ImGui::Button(ICON_FA_STOP))
                                    {
                                        anim.stop();
                                    }
                                }
                                
                                ImGui::TreePop();
                            }
                            
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<SpriteAnimation>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        if (e.hasComponent<Drawable>())
                        {
                            if (ImGui::TreeNode("Drawable"))
                            {
                                auto& d = e.getComponent<Drawable>();
                                
                                auto culled = d.getCulled();
                                if (ImGui::Checkbox("Cull", &culled))
                                {
                                    d.setCulled(culled);
                                }

                                
                                auto bounds = d.getLocalBounds();
                                ImGui::Text("Local bounds: ");
                                ImGui::TextUnformatted(("Left: " + std::to_string(bounds.left)).c_str());
                                ImGui::TextUnformatted(("Top: " + std::to_string(bounds.top)).c_str());
                                ImGui::TextUnformatted(("Width: " + std::to_string(bounds.width)).c_str());
                                ImGui::TextUnformatted(("Height: " + std::to_string(bounds.height)).c_str());
                                
                                auto& verts = d.getVertices();
                                ImGui::Text("Vertices:");
                                ImGui::SameLine();
                                if (ImGui::Button(ICON_FA_PLUS))
                                {
                                    verts.emplace_back();
                                }
                                int vertID = 0;
                                for (auto& v : verts)
                                {
                                    if (ImGui::TreeNode(std::to_string(vertID++).c_str()))
                                    {
                                        if (ImGui::InputFloat2("Position", (float*)&v.position))
                                        {
                                            d.updateLocalBounds();
                                        }
                                        ImVec4 col = v.color;
                                        if (ImGui::ColorEdit4("##color", (float*)&col))
                                        {
                                            v.color = col;
                                        }
                                        ImGui::TreePop();
                                    }
                                }
                                ImGui::TreePop();
                            }
                            
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<Drawable>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        if (e.hasComponent<Camera>())
                        {
                            ImGui::Selectable("Camera");
                        }
                        if (e.hasComponent<Transform>())
                        {
                            if (ImGui::TreeNode("Transform"))
                            {
                                auto& t = e.getComponent<Transform>();
                                
                                ImVec2 pos = t.getPosition();
                                ImVec2 org = t.getOrigin();
                                
                                if (ImGui::InputFloat2("Pos (x,y)", (float*)&pos))
                                {
                                    t.setPosition(pos);
                                }
                                if (ImGui::InputFloat2("Origin (x,y)", (float*)&org))
                                {
                                    t.setOrigin(org);
                                }
                                ImGui::TreePop();
                            }
                            
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<Transform>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        if (e.hasComponent<Text>())
                        {
                            if (ImGui::TreeNode("Text"))
                            {
                                auto& t = e.getComponent<Text>();
                                
                                auto bounds = t.getLocalBounds();
                                ImGui::Text("Local bounds: ");
                                ImGui::TextUnformatted(("Left: " + std::to_string(bounds.left)).c_str());
                                ImGui::TextUnformatted(("Top: " + std::to_string(bounds.top)).c_str());
                                ImGui::TextUnformatted(("Width: " + std::to_string(bounds.width)).c_str());
                                ImGui::TextUnformatted(("Height: " + std::to_string(bounds.height)).c_str());
                                
                                if (ImGui::BeginCombo("Font", "Select to change"))
                                {
                                    for (auto& a : Editor::getAssets())
                                    {
                                        if (a->getType() == AssetType::Font)
                                        {
                                            if (ImGui::Selectable(a->m_path.c_str()))
                                            {
                                                t.setFont(dynamic_cast<FontAsset*>(a.get())->font);
                                                t.setFontResourceID(xy::TextureResource::getID(a->m_path));
                                            }
                                        }
                                    }
                                    ImGui::EndCombo();
                                }
                                
                                // Modify string
                                std::array<char,MAX_INPUT_CHARS> buf;
                                t.getString().toAnsiString().copy(buf.data(), t.getString().toAnsiString().length());
                                if (ImGui::InputText("String",buf.data(),MAX_INPUT_CHARS))
                                {
                                    t.setString(std::string(buf.data()));
                                }
                                
                                // Change colour
                                ImVec4 col = t.getFillColour();
                                if (ImGui::ColorEdit4("Colour", (float*)&col))
                                {
                                    t.setFillColour(col);
                                }
                                
                                // Change size
                                int size = t.getCharacterSize();
                                if (ImGui::InputInt("Size", &size))
                                {
                                    t.setCharacterSize(size);
                                }
                                
                                ImGui::TreePop();
                            }
                            
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<Text>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        if (e.hasComponent<UIHitBox>())
                        {
                            if (ImGui::TreeNode("UI Hitbox"))
                            {
                                auto& u = e.getComponent<UIHitBox>();
                                ImGui::Checkbox("Active", &u.active);
                                auto& area = u.area;
                                sf::Vector2f pos = {area.left, area.top};
                                sf::Vector2f size = {area.width, area.height};
                                ImGui::Text("Area: ");
                                
                                if (ImGui::InputFloat2("Pos (x,y)", (float*)&pos))
                                {
                                    area.top = pos.y;
                                    area.left = pos.x;
                                }
                                if (ImGui::InputFloat2("Size (x,y)", (float*)&size))
                                {
                                    area.width = size.x;
                                    area.height = size.y;
                                }
                                ImGui::TreePop();
                            }
                            else if (ImGui::Button(ICON_FA_MINUS))
                            {
                                e.removeComponent<UIHitBox>();
                                
                                // Because removeComponent doesn't remove entity from systems
                                scene->m_systemManager.removeFromSystems(e);
                                scene->m_systemManager.addToSystems(e);
                            }
                        }
                        
                        // Add a new component to entity (And make sure requisite system is added
                        if (ImGui::BeginCombo("Add component", "Select type"))
                        {
                            if (ImGui::Selectable("Sprite"))
                            {
                                scene->addSystem<SpriteSystem>(App::getActiveInstance()->getMessageBus());
                                scene->addSystem<RenderSystem>(App::getActiveInstance()->getMessageBus());
                                e.addComponent<Sprite>();
                                e.addComponent<Transform>();
                                e.addComponent<Drawable>();
                                
                                // Must manually add to system, as it's only checked when creating the entity
                                scene->getSystem<SpriteSystem>().addEntity(e);
                                scene->getSystem<RenderSystem>().addEntity(e);
                            }
                            if (ImGui::Selectable("Transform"))
                            {
                                e.addComponent<Transform>();
                            }
                            if (ImGui::Selectable("Text"))
                            {
                                scene->addSystem<RenderSystem>(App::getActiveInstance()->getMessageBus());
                                scene->addSystem<TextRenderer>(App::getActiveInstance()->getMessageBus());
                                e.addComponent<Transform>();
                                e.addComponent<Text>();
                                e.addComponent<Drawable>();
                                
                                // Must manually add to system, as it's only checked when creating the entity
                                scene->getSystem<TextRenderer>().addEntity(e);
                                scene->getSystem<RenderSystem>().addEntity(e);
                            }
                            if (ImGui::Selectable("Drawable"))
                            {
                                scene->addSystem<RenderSystem>(App::getActiveInstance()->getMessageBus());
                                e.addComponent<Drawable>();
                                e.addComponent<Transform>();
                                
                                scene->getSystem<RenderSystem>().addEntity(e);
                            }
                            if (ImGui::Selectable("UI Hitbox"))
                            {
                                scene->addSystem<UISystem>(App::getActiveInstance()->getMessageBus());
                                e.addComponent<UIHitBox>();
                                
                                scene->getSystem<UISystem>().addEntity(e);
                            }
                            if (ImGui::Selectable("Sprite Animation"))
                            {
                                scene->addSystem<SpriteAnimator>(App::getActiveInstance()->getMessageBus());
                                e.addComponent<SpriteAnimation>();
                                
                                scene->getSystem<SpriteAnimator>().addEntity(e);
                            }
                            
                            ImGui::EndCombo();
                        }
                        
                        if (ImGui::Button("Destroy"))
                        {
                            scene->destroyEntity(e);
                        }
                        
                        ImGui::TreePop();
                        scene->getSystem<xy::EditorSystem>().highlightEntity(e, sf::Color::Red);
                    }
                    else if (ImGui::IsItemHovered())
                    {
                        scene->getSystem<xy::EditorSystem>().highlightEntity(e, sf::Color::Yellow);
                    }
                }
            }
            
            // New entity
            if (ImGui::Button("Create Entity"))
            {
                scene->createEntity();
            }
            
            // Path to save
            std::array<char, MAX_INPUT_CHARS> buf = {{0}};
            m_path.copy(buf.data(), m_path.length());
            if (ImGui::InputText("rename (path)", buf.data(), buf.size(), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_path = buf.data();
            }
            
            // Save scene
            if (ImGui::Button("Save"))
            {
                scene->saveToFile(m_path);
            }
        }
        else
        {
            // Scene not loaded, must be added in code
            ImGui::Text("Scene not Loaded");
        }
    }
}// namespace xy

