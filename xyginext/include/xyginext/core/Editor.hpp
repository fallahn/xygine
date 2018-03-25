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
    };
    
    class Editor
    {
    public:
        
        static void init();
        static void shutdown();
        
        static bool isEnabled();
        
    private:
        static void toggle();
        static void draw();
        
        static void showStyleEditor();
        static void showVideoSettings();
        static void showAudioSettings();
        static void showAssetBrowser();
        static void showSpriteEditor();
        static void showSceneEditor();
        
        // Called in the scene constructor
        // So the editor can keep track of active scenes
        static void registerScene(Scene* scene, const std::string& name);
        static void deregisterScene(Scene* scene);
        
        friend class App;
        friend class Editable;
        friend class Scene;
    };
}

#endif /* XY_EDITOR_HPP_ */
