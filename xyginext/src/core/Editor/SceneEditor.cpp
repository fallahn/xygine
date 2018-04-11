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
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Camera.hpp"
#include "xyginext/ecs/components/Transform.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"

#include <set>

namespace xy
{
namespace SceneEditor
{
    namespace
    {
        // To keep track of which entitie's sprites we are editing
        std::set<Entity> openSprites;
    }
    void editScene(Scene& scene)
    {
        auto& editorSys = scene.getSystem<EditorSystem>();
        auto ents = editorSys.getEntities();
        ImGui::Text("Entities:");
        for (auto& e : ents)
        {
            if (ImGui::TreeNode(std::to_string(e.getIndex()).c_str()))
            {
                if (e.hasComponent<Sprite>())
                {
                    bool selected = openSprites.find(e) != openSprites.end();
                    if (ImGui::Selectable("Sprite", &selected))
                    {
                        if (openSprites.find(e) != openSprites.end())
                        {
                            openSprites.erase(e);
                        }
                        else
                        {
                            openSprites.insert(e);
                        }
                    }
                    
                    if (openSprites.find(e) != openSprites.end())
                    {
                        SpriteEditor::editSprite(e.getComponent<Sprite>());
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
                
                if (ImGui::Button("Destroy"))
                {
                    scene.destroyEntity(e);
                }
                
                ImGui::TreePop();
            }
        }
    }
        
}// namespace SceneEditor
}// namespace xy

