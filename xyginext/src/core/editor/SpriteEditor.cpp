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

namespace xy
{
namespace SpriteEditor
{

namespace
{
    // I figure this being global is OK, as the user can only ever be using
    // one mouse (realistically)
    enum class RectDragPos
    {
        NONE,
        MOVE,
        T,
        B,
        L,
        R,
        TL,
        TR,
        BR,
        BL
    };
    bool draggingRect = false;
    RectDragPos rectDragPos;
    
    // For selecting sprite from assets
    std::string selectedSheetName = "Select a sheet";
    SpriteSheetAsset*   selectedSheet;
}


void showTextureRectSelector(Sprite& sprite)
{
    auto texRect = sprite.getTextureRect();
    auto newRect = texRect;
    ImGui::SetNextWindowPos({0,0});
    if (ImGui::BeginPopupModal("Texture Rect Selector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
    {
        sf::Vector2f origin = ImGui::GetCursorScreenPos();
        
        // If the mouse is near the edges/corners, resize
        sf::Vector2f mousePos = ImGui::GetIO().MousePos;
        mousePos -= origin; // make relative to this texture preview
        
        static const float tolerance = 5.f; // how close mouse should be to enable behaviour
        
        // create a rect on each edge and each corner to check
        sf::FloatRect tl = {texRect.left - tolerance, texRect.top - tolerance, tolerance*2, tolerance*2};
        sf::FloatRect tr = {texRect.left + texRect.width - tolerance, texRect.top - tolerance, tolerance*2, tolerance*2};
        sf::FloatRect bl = {texRect.left - tolerance, texRect.top + texRect.height - tolerance, tolerance*2, tolerance*2};
        sf::FloatRect br = {texRect.left + texRect.width - tolerance, texRect.top + texRect.height - tolerance, tolerance*2, tolerance*2};
        
        sf::FloatRect l = {tl.left, tl.top + tl.height, tl.width, bl.top - tl.top + tl.height};
        sf::FloatRect r = {tr.left, tr.top + tr.height, tr.width, br.top - tr.top + tr.height};
        sf::FloatRect t = {tl.left + tl.width, tl.top, tr.left - tl.left + tl.width, tl.height};
        sf::FloatRect b = {bl.left + bl.width, bl.top, br.left - bl.left + bl.width, bl.height};
        
        // Check if we're dragging the rect
        if (rectDragPos != RectDragPos::NONE)
        {
            auto io = ImGui::GetIO();
            if (io.MouseClicked[0])
            {
                draggingRect = true;
            }
        }
        
        
        if (!draggingRect)
        {
            if (tl.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                rectDragPos = RectDragPos::TL;
            }
            else if(br.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                rectDragPos = RectDragPos::BR;
            }
            else if (tr.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                rectDragPos = RectDragPos::TR;
            }
            else if(bl.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                rectDragPos = RectDragPos::BL;
            }
            else if(l.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                rectDragPos = RectDragPos::L;
            }
            else if(r.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                rectDragPos = RectDragPos::R;
            }
            else if(t.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                rectDragPos = RectDragPos::T;
            }
            else if(b.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                rectDragPos = RectDragPos::B;
            }
            else if (texRect.contains(mousePos))
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                rectDragPos = RectDragPos::MOVE;
            }
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                rectDragPos = RectDragPos::NONE;
            }
        }
        else
        {
            
            auto md = ImGui::GetIO().MouseDelta;
            switch(rectDragPos)
            {
                case RectDragPos::TL:
                {
                    texRect.top += md.y;
                    texRect.left += md.x;
                    texRect.width -= md.x;
                    texRect.height -= md.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                    break;
                }
                case RectDragPos::TR:
                {
                    texRect.top += md.y;
                    texRect.width += md.x;
                    texRect.height -= md.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                    break;
                }
                case RectDragPos::BR:
                {
                    texRect.width += md.x;
                    texRect.height += md.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                    break;
                }
                case RectDragPos::BL:
                {
                    texRect.left += md.x;
                    texRect.width -= md.x;
                    texRect.height -= md.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                    break;
                }
                case RectDragPos::L:
                {
                    texRect.left += md.x;
                    texRect.width -= md.x;
                    break;
                }
                case RectDragPos::T:
                {
                    texRect.top += md.y;
                    texRect.height -= md.y;
                    break;
                }
                case RectDragPos::R:
                {
                    texRect.width += md.x;
                    break;
                }
                case RectDragPos::B:
                {
                    texRect.height += md.y;
                    break;
                }
                case RectDragPos::MOVE:
                {
                    texRect.left += md.x;
                    texRect.top += md.y;
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                    break;
                }
            }
            // Snap it
            auto snap = Editor::getPixelSnap();
            auto snapThatVar = [snap](float var)
            {
                return std::round(var/snap) * snap;
            };
            
            newRect.left = snapThatVar(texRect.left);
            newRect.top = snapThatVar(texRect.top);
            newRect.width = snapThatVar(texRect.width);
            newRect.height = snapThatVar(texRect.height);
            
            // Check for mouse release
            auto io = ImGui::GetIO();
            if (io.MouseReleased[0])
            {
                draggingRect = false;
                texRect = newRect;
            }
            
            sprite.setTextureRect(texRect);
        }
        
        ImGui::Image(*sprite.getTexture());
        
        // draw the rect on the image, semi-transparent
        sf::Color col = ImGui::GetStyle().Colors[ImGuiCol_TextSelectedBg];
        col.a = 128;
        
        // Because otherwise the rect is just shown AFTER the image
        auto transformedRect = newRect;
        auto curPos = ImGui::GetCursorScreenPos();
        transformedRect.left -= curPos.x - origin.x;
        transformedRect.top -= curPos.y - origin.y;
        ImGui::DrawRectFilled(sf::FloatRect(transformedRect), col);
        
        if (ImGui::Button(ICON_FA_CHECK))
        {
            ImGui::CloseCurrentPopup();
            sprite.setTextureRect(newRect);
        }
        
        // Show current rect
        ImGui::SameLine();
        ImGui::Text(("Top = " + std::to_string(newRect.top) + ", Left = " + std::to_string(newRect.left) + ", Width = " + std::to_string(newRect.width) + ", Height = " + std::to_string(newRect.height)).c_str());
        ImGui::EndPopup();
    }
}
    
    
    void editSprite(xy::Sprite& spr)
    {
        // Texture Rect
        auto texRect = static_cast<sf::IntRect>(spr.getTextureRect());
        if (ImGui::InputInt4("Texture Rect", (int*)&texRect))
        {
            spr.setTextureRect(static_cast<sf::FloatRect>(texRect));
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PENCIL_ALT))
        {
            ImGui::OpenPopup("Texture Rect Selector");
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
        
        // double meh...
        if (selectedAnim != "Select an animation")
        {
            
            for (auto i = 0 ; i < spr.getAnimationCount(); i++)
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
                        anim.frameCount = fc;
                    }
                    static int currentFrame(0);
                    ImGui::SliderInt("Frames", &currentFrame, 0, anim.frameCount);
                    
                    // Tex rect of current frame
                    auto& frame = anim.frames[currentFrame];
                    auto texRect = static_cast<sf::IntRect>(frame);
                    if (ImGui::InputInt4("Texture Rect##anim", (int*)&texRect))
                    {
                        frame = (static_cast<sf::FloatRect>(texRect));
                    }
                }
            }
        }
        
        // Show texture selector modal popup, if open
        showTextureRectSelector(spr);
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
            sheet.setSprite("New Sprite", Sprite(*sheet.getSprites().begin()->second.getTexture())); // much stupid
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
            static std::array<char, MAX_INPUT> input = {{0}};
            selectedSprite.copy(input.data(), selectedSprite.length());
            
            auto spr = sheet.getSprite(selectedSprite);
            
            if (ImGui::InputText("Name", input.data(), MAX_INPUT))
            {
                // much inefficient
                sheet.removeSprite(selectedSprite);
                selectedSprite = std::string(input.data());
                sheet.setSprite(selectedSprite, spr);
                m_dirty = true;
            }
            
            ImGui::Separator();
            
            SpriteEditor::editSprite(spr);
            
            // Add/Remove animations
            /*if (ImGui::Button("+##anim"))
            {
                spr.getAnimations()[spr.getAnimationCount()] = Sprite::Animation();
                selectedAnim = "New Anim";
                selectedAnim.copy(spr.getAnimations()[spr.getAnimationCount()].id.data(), selectedAnim.length());
                spr.setAnimationCount(spr.getAnimationCount()+1);
                sheet.setSprite(selectedSprite, spr);
                dirty = true;
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
                dirty = true;
            }*/
            
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
