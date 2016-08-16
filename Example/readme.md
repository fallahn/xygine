xygine examples
---------------

The example project provides a few code samples for getting started with
xygine. When using the included visual studio solution the example project
will be built by default. If using the CMake build configuration first
make sure to [build and install](https://github.com/fallahn/xygine/wiki/Building) xygine so that the CMake script can find it.
Then configure the example project in the same way. NOTE the link settings
for the example dependencies (SFML, xygine, Box2D) must all match. If
xygine is built as a shared library (and therefore not statically linked
to SFML or Box2D) then the example should be built in shared mode too.
Conversely if one library is statically linked, then all libraries must
be statically linked.


######What's included?
Currently the example provides a demonstration of how to create a xygine
game by inheriting the App class, how to use the state stack with custom
states, and includes a set of states each of which are designed to provide
an example of a particular xygine feature. It also demonstrates xygine's UI
components by using them to create a navigable menu system.

Particles and dynamic lighting.  
Demonstrates xygines particle system and the built in lighting shader. Also
uses some basic physics components.

Physics.  
A small pool/billiards style simulation which demonstrates the possibilities
of xygine's binding to box2D.

Networking.  
A pong clone, written specifically with network play in mind. Also used to
develop xygine's network components so may be buggy!

Deferred Rendering.  
A special SFML compatible MultiRenderTarget - not dissimilar to SFML's
render texture - is used to create a g-buffer for deferred rendering with
normal mapping and dynamic lighting.

3D Mesh rendering.
The platform example demonstratoes how to use xygine's MeshRenderer to
overlay 3D models on the curent scene.

Tilemap.
xygine includes a parser for Tiled TMX format tile maps, and the tilemap
demo included renders an orthogonal example map, and demonstrates conversion
from map objects into xygine's physics components.

######Contributions
Contributions should be acknowledged and are gratefully accepted from:  
Jonny Paton  
Jean-Sebastien Fauteux (OS X Testing) 

-----------------------------------------------------------------------

Matt Marchant 2014 - 2016  
http://trederia.blogspot.com  

xygine - Zlib license.  

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

-----------------------------------------------------------------------
