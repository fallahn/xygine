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

#pragma once

#include <string>

// Enum of asset types
enum class AssetType
{
    Texture, // Any raw texture: png, jpeg, etc.
    Spritesheet, // An xy::Spritesheet (.spt)
    Sound, // Sound file: ogg, wav, etc.
    Scene, // An xy::Scene (.something...)
    ParticleEmitter, // An xy::ParticleEmitter (or rather, it's settings)
    Font, // A font file (.ttf or .otf)
};

// Base class for anything representing an "Asset" in the editor
class EditorAsset
{
public:
    virtual void edit() = 0; // Shows imgui edit controls
    virtual AssetType getType() const = 0; // The type of this asset
    
    bool m_open = false; // If the asset is open in the editor
    bool m_dirty = false; // If the asset has unsaved changes in the editor
    std::string m_path; // Path to the asset (relative to assets folder - bad idea?)
};