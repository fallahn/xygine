Shader Editor
-------------

Real time editing and hot-loading of shaders written in glsl. This is aimed at writing shaders for SFML so only 2D previews are available (for now...), and vertex shader editing is not supported. The main reason for this is that SFML 'optimises' geometry with fewer than 5 vertices by pre-multiplying their positions into world space. This means that the gl_ModelViewMatrix supplied to vertex shaders is an identity matrix and therefore effectively useless for creating other matrices such as a normal matrix. It is possible to supply ones own matrices via uniforms, of course, but attaching those via a simplified user interface is beyond the scope of this editor. For most fragment shaders and post-process effects this utility should be sufficient.

The editor uses the [ImGui text editor widget](https://github.com/BalazsJako/ImGuiColorTextEdit) by BalazsJako and [tiny file dialogues](https://sourceforge.net/projects/tinyfiledialogs/) by vareille.

/*********************************************************************  
(c) Matt Marchant & contributors 2019  
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
