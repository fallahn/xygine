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
#include "xyginext/core/editor/TextureEditor.hpp"
#include "xyginext/core/editor/ParticleEditor.hpp"

#include "../../imgui/imgui.h"

namespace xy
{
namespace ParticleEditor
{
    
}
    
namespace
{
    // Can't use unordered/map bc blendmode doesn't have requisite operators
    std::vector<std::pair<sf::BlendMode, std::string>> blendModes
    {
        {sf::BlendAdd, "add"},
        {sf::BlendMultiply, "multiply"},
        {sf::BlendAlpha, "alpha"}
    };
}
    
void ParticleEmitterAsset::edit()
{
    // Select texture
    auto texPath = settings.texturePath.length() ? settings.texturePath : "Select a texture";
    if (ImGui::BeginCombo("Texture", texPath.c_str()))
    {
        for (auto& asset : Editor::getAssets())
        {
            if (asset->getType() == AssetType::Texture)
            {
                auto texAsset = dynamic_cast<TextureAsset*>(asset.get());
                if (ImGui::Selectable(asset->m_path.c_str()))
                {
                    settings.texture = &dynamic_cast<TextureAsset*>(asset.get())->texture;
                    settings.texturePath = texAsset->m_path;
                }
            }
        }
        ImGui::EndCombo();
    }
    
    // Blend mode
    auto currentbm = std::find_if(blendModes.begin(), blendModes.end(), [this](std::pair<sf::BlendMode, std::string> bm)
                                  {
                                      return bm.first == settings.blendmode;
                                  });
    if (ImGui::BeginCombo("Blend Mode", currentbm->second.c_str()))
    {
        for (auto& bm : blendModes)
        {
            if (ImGui::Selectable(bm.second.c_str()))
            {
                settings.blendmode = bm.first;
            }
        }
        ImGui::EndCombo();
    }
    
    // Emit rate
    ImGui::InputFloat("Emit rate", &settings.emitRate);
    int ec = settings.emitCount;
    if (ImGui::InputInt("Emit count", &ec))
    {
        settings.emitCount = ec;
    }
    
    // Initial velocity
    ImGui::InputFloat2("Initial velocity", (float*)&settings.initialVelocity);
    
    // Colour
    ImVec4 col = settings.colour;
    if (ImGui::ColorEdit3("Colour", (float*)&col))
    {
        settings.colour = col;
    }
    
    // Size
    ImGui::InputFloat("Size", &settings.size);
    
    // Save
    if (ImGui::Button("Save"))
    {
        settings.saveToFile(m_path);
    }
}
}// namespace xy
