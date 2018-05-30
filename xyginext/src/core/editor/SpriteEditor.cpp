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

#include "xyginext/core/Editor.hpp"
#include "xyginext/core/editor/SpriteEditor.hpp"
#include "xyginext/core/editor/TextureEditor.hpp"
#include "xyginext/core/editor/IconFontAwesome5.hpp"
#include "xyginext/ecs/Scene.hpp"
#include "xyginext/ecs/systems/CameraSystem.hpp"
#include "xyginext/ecs/systems/RenderSystem.hpp"
#include "xyginext/ecs/systems/SpriteSystem.hpp"
#include "xyginext/ecs/components/Drawable.hpp"
#include "xyginext/ecs/components/Transform.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"

#include <cmath>

namespace xy
{
namespace SpriteEditor
{

namespace
{
    
    // For selecting sprite from assets
    std::string selectedSheetName = "Select a sheet";
    SpriteSheetAsset*   selectedSheet;
    
    constexpr std::size_t MAX_INPUT_CHARS = 400;
}
    
    Sprite* selectSpriteFromAssets()
    {
        ImGui::SetNextWindowPosCenter();
        if (ImGui::BeginPopupModal("Select Sprite from assets"))
        {
            if (ImGui::BeginCombo("Spritesheets", selectedSheetName.c_str()))
            {
                for (auto& asset : Editor::getAssets())
                {
                    if (asset->getType() == AssetType::Spritesheet)
                    {
                        if (ImGui::Selectable(asset->m_path.c_str()))
                        {
                            selectedSheetName = asset->m_path;
                            selectedSheet = dynamic_cast<SpriteSheetAsset*>(asset.get());
                        }
                    }
                }
                
                ImGui::EndCombo();
            }
            if (selectedSheet)
            {
                // Show sprites from sheet
                if (ImGui::BeginCombo("Sprites", "Select sprite to load"))
                {
                    for (auto s : selectedSheet->sheet.getSprites())
                    {
                        if (ImGui::Selectable(s.first.c_str()))
                        {
                            ImGui::CloseCurrentPopup();
                            ImGui::EndCombo();
                            ImGui::EndPopup();
                            return &s.second;
                            
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::EndPopup();
        }
        return nullptr;
    }
    
}// namespace SpriteEditor
    
SpriteSheetAsset::SpriteSheetAsset(const std::string sheetPath, xy::TextureResource& textures)
{
    m_path = sheetPath;
    sheet.loadFromFile(sheetPath, textures);
    m_textureAsset.texture = textures.get(sheet.getTexturePath()); // bit of a hack
}
    
void SpriteSheetAsset::edit()
{
    // Select which texture to use
    auto file = FileSystem::getFileName(sheet.getTexturePath());
    std::string texName =  file.length() > 0 ? file : "Select a texture";
    if (ImGui::BeginCombo("Texture", texName.c_str()))
    {
        for (auto& asset : Editor::getAssets())
        {
            if (asset->getType() == AssetType::Texture)
            {
                if (ImGui::Selectable(asset->m_path.c_str()))
                {
                    texName = asset->m_path;
                    sheet.setTexturePath(asset->m_path);
                    m_textureAsset = *dynamic_cast<TextureAsset*>(asset.get());
                    m_dirty = true;
                }
            }
        }
        ImGui::EndCombo();
    }
    
    // triple meh
    if (texName != "Select a texture")
    {
        // Select sprite to edit
        auto sprites = sheet.getSprites();
        static std::string selectedSprite = "Select a sprite";
        if (ImGui::BeginCombo("Sprites", selectedSprite.c_str()))
        {
            for (auto& spr : sprites)
            {
                if (ImGui::Selectable(spr.first.c_str()))
                {
                    selectedSprite = spr.first;
                }
            }
            ImGui::EndCombo();
        }
        
        // Add/delete sprite
        if (ImGui::Button("+##sprite"))
        {
            xy::Sprite spr(m_textureAsset.texture);
            spr.setTextureResourceID(xy::TextureResource::getID(texName));
            sheet.setSprite("New Sprite", spr);
            selectedSprite = "New Sprite";
            m_dirty = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("-##sprite"))
        {
            sheet.removeSprite(selectedSprite);
            selectedSprite = "Select a sprite";
            m_dirty = true;
        }
        
        // meh...
        if (selectedSprite != "Select a sprite")
        {
            // sprite name
            static std::array<char, SpriteEditor::MAX_INPUT_CHARS> input = {{0}};
            selectedSprite.copy(input.data(), selectedSprite.length());
            
            auto spr = sheet.getSprite(selectedSprite);
            
            if (ImGui::InputText("Name", input.data(), SpriteEditor::MAX_INPUT_CHARS))
            {
                // much inefficient
                sheet.removeSprite(selectedSprite);
                selectedSprite = std::string(input.data());
                sheet.setSprite(selectedSprite, spr);
                m_dirty = true;
            }
            
            ImGui::Separator();
            
            // Texture Rect
            auto texRect = static_cast<sf::IntRect>(spr.getTextureRect());
            if (ImGui::InputInt4("Texture Rect", (int*)&texRect))
            {
                spr.setTextureRect(static_cast<sf::FloatRect>(texRect));
            }
            ImGui::SameLine();
            static bool showTexSelector = false;
            if (ImGui::Button(ICON_FA_PENCIL_ALT))
            {
                showTexSelector = true;
            }
            if (showTexSelector)
            {
                auto rect = spr.getTextureRect();
                if (m_textureAsset.selectRect(rect))
                {
                    showTexSelector = false;
                }
                spr.setTextureRect(rect);
            }
            
            // Sprite Colour
            ImVec4 col = spr.getColour();
            if (ImGui::ColorEdit3("Colour", (float*)&col))
            {
                spr.setColour(col);
            }
            
            // Animation combo list
            static std::string selectedAnim = "Select an animation";
            if (ImGui::BeginCombo("Animations", selectedAnim.c_str()))
            {
                for (auto& anim : spr.getAnimations())
                {
                    bool animSelected(false);
                    ImGui::Selectable(anim.id.data(), &animSelected);
                    if (animSelected)
                    {
                        selectedAnim = anim.id.data();
                    }
                }
                ImGui::EndCombo();
            }
            
            // Add/Remove animations
            if (ImGui::Button("+##anim"))
            {
                spr.getAnimations()[spr.getAnimationCount()] = Sprite::Animation();
                selectedAnim = "New Anim";
                selectedAnim.copy(spr.getAnimations()[spr.getAnimationCount()].id.data(), selectedAnim.length());
                spr.setAnimationCount(spr.getAnimationCount()+1);
                sheet.setSprite(selectedSprite, spr);
                m_dirty = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("-##anim"))
            {
                auto& anims = spr.getAnimations();
                for (auto i = 0; i < spr.getAnimationCount(); i++)
                {
                    if (std::string(anims[i].id.data()) == selectedAnim)
                    {
                        // move all anims up one
                        for (auto j = i+1; j <= spr.getAnimationCount(); j++, i++)
                        {
                            anims[i] = anims[j];
                        }
                        break;
                    }
                }
                sheet.setSprite(selectedSprite, spr);
                m_dirty = true;
            }
            
            // double meh...
            if (selectedAnim != "Select an animation")
            {
                
                for (auto i = 0u ; i < spr.getAnimationCount(); i++)
                {
                    auto& anim = spr.getAnimations()[i];
                    if (std::string(anim.id.data()) == selectedAnim)
                    {
                        // Name
                        if (ImGui::InputText("Name##anim", anim.id.data(), Sprite::Animation::MaxAnimationIdLength))
                        {
                            selectedAnim = anim.id.data();
                        }
                        
                        // Properties
                        ImGui::InputFloat("Framerate", &anim.framerate);
                        ImGui::Checkbox("Loop", &anim.looped);
                        
                        // Frames
                        int fc = anim.frameCount;
                        if (ImGui::InputInt("Frame count", &fc, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            if (fc > anim.frameCount)
                            {
                                // set frame to texture size
                                auto texSize = m_textureAsset.texture.getSize();
                                anim.frames[fc-1].width = texSize.x;
                                anim.frames[fc-1].height = texSize.y;
                            }
                            anim.frameCount = fc;
                        }
                        if (fc > 0)
                        {
                            static int currentFrame(0);
                            ImGui::SliderInt("Frames", &currentFrame, 0, anim.frameCount -1 );
                            
                            // Tex rect of current frame
                            auto& frame = anim.frames[currentFrame];
                            auto texRect = static_cast<sf::IntRect>(frame);
                            if (ImGui::InputInt4("Texture Rect##anim", (int*)&texRect))
                            {
                                frame = (static_cast<sf::FloatRect>(texRect));
                            }
                            
                            ImGui::SameLine();
                            static bool showTexSelector = false;
                            if (ImGui::Button(ICON_FA_PENCIL_ALT"##anim"))
                            {
                                showTexSelector = true;
                            }
                            if (showTexSelector)
                            {
                                if (m_textureAsset.selectRect(frame))
                                {
                                    showTexSelector = false;
                                }
                            }
                        }
                    }
                }
            }
            
            // should restructure to avoid this
            sheet.setSprite(selectedSprite, spr);
            
            // Add sprite to a scene
           /* auto& scenes = Editor::getSceneAssets();
            if (ImGui::BeginCombo("Add to scene", "Select a scene"))
            {
                for (auto& s : scenes)
                {
                    if (ImGui::Selectable(s.first.c_str()))
                    {
                        auto e = s.second.scene->createEntity();
                    }
                }
            }*/
        }
        
        // Save button
        if (ImGui::Button("Save"))
        {
            sheet.saveToFile(m_path);
        }
        
        // Delete spritesheet
        if (ImGui::Button("Delete (Danger, no undo...)"))
        {
            if (std::remove(m_path.c_str()))
            {
                Logger::log("Error deleting file " + m_path);
            }
        }
    }
}
}// namespace xy
