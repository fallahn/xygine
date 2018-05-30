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

#include "xyginext/core/editor/TextureEditor.hpp"
#include "xyginext/core/Editor.hpp"
#include "xyginext/core/editor/IconFontAwesome5.hpp"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui-SFML.h"

#include <cmath>

namespace xy
{
    namespace
    {
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
    }
    
    void TextureAsset::edit()
    {
        
    }
    
    bool TextureAsset::selectRect(sf::FloatRect& texRect)
    {
        ImGui::SetNextWindowPosCenter();
        if (ImGui::Begin("Texture Rect Selector", nullptr, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
        {
            auto newRect = texRect;
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
                        newRect.top += md.y;
                        newRect.left += md.x;
                        newRect.width -= md.x;
                        newRect.height -= md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                        break;
                    }
                    case RectDragPos::TR:
                    {
                        newRect.top += md.y;
                        newRect.width += md.x;
                        newRect.height -= md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                        break;
                    }
                    case RectDragPos::BR:
                    {
                        newRect.width += md.x;
                        newRect.height += md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                        break;
                    }
                    case RectDragPos::BL:
                    {
                        newRect.left += md.x;
                        newRect.width -= md.x;
                        newRect.height -= md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
                        break;
                    }
                    case RectDragPos::L:
                    {
                        newRect.left += md.x;
                        newRect.width -= md.x;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                        break;
                    }
                    case RectDragPos::T:
                    {
                        newRect.top += md.y;
                        newRect.height -= md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                        break;
                    }
                    case RectDragPos::R:
                    {
                        newRect.width += md.x;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                        break;
                    }
                    case RectDragPos::B:
                    {
                        newRect.height += md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
                        break;
                    }
                    case RectDragPos::MOVE:
                    {
                        newRect.left += md.x;
                        newRect.top += md.y;
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                        break;
                    }
                    default:
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                        break;
                }
                // Snap it
                auto snap = Editor::getPixelSnap();
                auto snapThatVar = [snap](float var)
                {
                    return std::round(var/snap) * snap;
                };
                
                newRect.left = snapThatVar(newRect.left);
                newRect.top = snapThatVar(newRect.top);
                newRect.width = snapThatVar(newRect.width);
                newRect.height = snapThatVar(newRect.height);
                
                // Check for mouse release
                auto io = ImGui::GetIO();
                if (io.MouseReleased[0])
                {
                    draggingRect = false;
                }
            }
            
            ImGui::Image(texture);
            
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
                ImGui::End();
                return true;
            }
            
            // Show current rect
            ImGui::SameLine();
            ImGui::TextUnformatted(("Top = " + std::to_string(newRect.top) + ", Left = " + std::to_string(newRect.left) + ", Width = " + std::to_string(newRect.width) + ", Height = " + std::to_string(newRect.height)).c_str());
            ImGui::End();
            texRect = newRect;
            return false;
        }
    }
}// namespace xy
