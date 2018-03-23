//
//  Editor.hpp
//
//  Created by Jonny Paton on 12/03/2018.
//

#ifndef XY_EDITOR_HPP_
#define XY_EDITOR_HPP_

#include <vector>
#include <SFML/Graphics/RenderTexture.hpp>

#include <xyginext/ecs/System.hpp>

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
        
        static bool isEnabled();
        
    private:
        static void toggle();
        static void draw();
        
        static void showStyleEditor();
        static void showVideoSettings();
        static void showAudioSettings();
        static void showAssetBrowser();
        static void showSpriteEditor();
        
        friend class App;
        friend class Editable;
    };
}

#endif /* XY_EDITOR_HPP_ */
