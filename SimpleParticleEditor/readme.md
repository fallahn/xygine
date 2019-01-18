Simple Particle Editor
----------------------

This project uses the cmake template as a basis for creating a simple particle system
editor which can load and save xygine's *.xyp files, used by the particle components.
Particle systems can be edited with the on-screen gui (created with xy::Gui / ImGUI)
and previewed in real time. Created particle systems can then be loaded by any xygine
project which uses the ParticleEmitter components and ParticleSystem.


When first launching the editor remember to set the working directory via the on screen
menu to that of your project. The editor will still work without it, but the exported
*.xyp files will need to be manually edited for the correct texture paths.


Improvements and suggestions are welcome via github.

/*********************************************************************  
(c) Matt Marchant 2019  
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