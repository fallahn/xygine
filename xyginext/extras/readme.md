## xygine extras

This directory contains the source for various utilities and xygine compatible systems
which may be useful in a project but don't warrant inclusion in the main library. To use
these classes open the relavant *.hpp file and follow the intstructions. These mostly
consist of copying the source files to your project and adding them to your build path
although some maybe require linking external libraries. These files may or may not be
'feature complete' and are added / updated only when I find I have a need for them myself.
Contributions via pull requests are welcome, however. Currently available are:

- ShapeUtils.hpp Creates basic geometric shapes from `xy::Drawable` components, useful
for drawaing debug output.
- PhysicsSystem.hpp Acts as a binding for the Chipmunk2D physics simulation library
allowing it to be used with xygine's ECS. Requires linking to [chipmunk2D](https://chipmunk-physics.net/)


/*********************************************************************  
(c) Matt Marchant 2017 - 2018  
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
