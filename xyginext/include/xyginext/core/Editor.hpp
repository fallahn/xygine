//
//  Editor.hpp
//
//  Created by Jonny Paton on 12/03/2018.
//

#ifndef XY_EDITOR_HPP_
#define XY_EDITOR_HPP_

#include <vector>
#include <SFML/Graphics/RenderTexture.hpp>

namespace xy
{
    class Editable
    {
    public:
        Editable();
        ~Editable();
        
    private:
        friend class Editor;
        virtual void editorDraw() = 0;
        
    protected:
        const int m_id;
    };
    
    class Editor
    {
    public:
        
        static void init();
        
        static bool isEnabled();
        
        template<typename T>
        static void registerEditable(const std::string& name, T* editable);
        
    private:
        static void toggle();
        static void draw();
        
        static void showStyleEditor();
        static void showVideoSettings();
        static void showAudioSettings();
        
        friend class App;
        friend class Editable;
    };
}

#endif /* XY_EDITOR_HPP_ */
