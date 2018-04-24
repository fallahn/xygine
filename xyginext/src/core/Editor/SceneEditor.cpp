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
#include "xyginext/core/Editor.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/Text.hpp"
#include "xyginext/ecs/systems/TextRenderer.hpp"
#include "xyginext/ecs/systems/RenderSystem.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"
#include "xyginext/ecs/systems/CameraSystem.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"

#include <set>

namespace xy
{
    namespace
    {
        // To keep track of which entitie's sprites we are editing
        std::set<Entity> openSprites;
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
                ImGui::Text(s->getType().name());
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
                                        e.addComponent<Transform>();
                                    }
                                }
                                ImGui::TreePop();
                            }
                        }
                        if (e.hasComponent<Drawable>())
                        {
                            ImGui::Selectable("Drawable");
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
                                
                                auto x = t.getPosition().x;
                                auto y = t.getPosition().y;
                                
                                if (ImGui::InputFloat("x", &x))
                                {
                                    t.setPosition(x,y);
                                }
                                if (ImGui::InputFloat("y", &y))
                                {
                                    t.setPosition(x,y);
                                }
                                
                                ImGui::TreePop();
                            }
                        }
                        if (e.hasComponent<Text>())
                        {
                            if (ImGui::TreeNode("Text"))
                            {
                                auto& t = e.getComponent<Text>();
                                
                                // Select font
                               /* auto& fonts = Editor::getFonts();
                                
                                if (ImGui::BeginCombo("Font", "Select to change"))
                                {
                                    for (auto& f : fonts)
                                    {
                                        if (ImGui::Selectable(f.first.c_str()))
                                        {
                                            t.setFont(f.second);
                                        }
                                    }
                                    ImGui::EndCombo();
                                }*/
                                
                                // Modify string
                                std::array<char,MAX_INPUT> buf;
                                t.getString().toAnsiString().copy(buf.begin(), t.getString().toAnsiString().length());
                                if (ImGui::InputText("String",buf.data(),MAX_INPUT))
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
                            
                            ImGui::EndCombo();
                        }
                        
                        if (ImGui::Button("Destroy"))
                        {
                            scene->destroyEntity(e);
                        }
                        
                        ImGui::TreePop();
                    }
                }
            }
            
            // New entity
            if (ImGui::Button("Create Entity"))
            {
                scene->createEntity();
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

