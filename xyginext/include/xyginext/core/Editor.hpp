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

#ifndef XY_EDITOR_HPP_
#define XY_EDITOR_HPP_

#include <vector>
#include <SFML/Graphics/RenderTexture.hpp>

#include "xyginext/ecs/System.hpp"
#include "xyginext/graphics/SpriteSheet.hpp"
#include "xyginext/ecs/components/Camera.hpp"

namespace xy
{
    // Editable component, attach this to entities for the editor system to see them
    class Editable final
    {
        // hrm...
    };
    
    // Editor system, add it to scenes you want to modify
    class EditorSystem : public xy::System
    {
    public:
        EditorSystem(xy::MessageBus& mb, const std::string& sceneName = "");
        ~EditorSystem();
        
    protected:
        void onCreate();
        
    private:
        std::string m_sceneName;
        
        friend class Editor;
    };
    
    // Basically just a load of static/global stuff I haven't decided on a decent structure for yet
    class Editor
    {
    public:
        
        static void init();
        static void shutdown();
        
        static bool isEnabled();
        
    private:
        static void toggle();
        static void draw();
        static bool handleEvent(sf::Event& ev);
        
        static void showStyleEditor();
        static void showVideoSettings();
        static void showAudioSettings();
        static void showAssetBrowser();
        static void showSpriteEditor();
        static void showSceneEditor();
        
        static void showModalPopups();
        
        friend class App;
        friend class Editable;
        friend class Scene;
    };
    
    // These will probably eventually be moved to their own source files
    
    struct SpriteSheetAsset
    {
        SpriteSheet sheet; // The spritesheet asset itself
        bool        open = false; // True if the asset is currently open for editing
        bool        unsavedChanges = false;
        std::string path;
    };
    
    struct SceneAsset
    {
        Scene* scene;
        xy::Entity cam;
    };
}

#endif /* XY_EDITOR_HPP_ */
