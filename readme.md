xygineXT
--------

[![Build Status](https://travis-ci.org/fallahn/xygine.svg?branch=master)](https://travis-ci.org/fallahn/xygine)
[![Build status](https://ci.appveyor.com/api/projects/status/0g672m1laa8mutdw/branch/master?svg=true)](https://ci.appveyor.com/project/fallahn/xygine)


The next iteration of xygine....

![Logo by Baard](xyginext/xygine.png?raw=true)

2D Game Engine Framework built around SFML. xygine implements a scene
graph along with an Entity Component System. I use this as the starting
point for new projects, as it evolves slowly over time. The source for
xygine is released under the zlib license in the hope that it may be
useful for other people - but offers no warranty that it is fit for any
particular purpose, even those for which it was specifically written...

This version provides an overhauled API and, as such, is not compatible 
with existing versions of xygine - hence the slightly different naming.
The biggest changes are to the Entity Component System for flexible, modular
development of systems with a data-driven approach, as well as the
removal of 3D support. If you're interested in a 3D, crossplatform, mobile
compatible framework with a very similar API, check out [crogine](https://github.com/fallahn/crogine).

xygine uses C++17 features so an up to date compiler is required, for
example XCode 10 on macOS, Visual Studio 2017 on windows or recent versions
of gcc and clang.

###### What's new?
All new modular ECS providing a flexible and performant API for implementing
custom components and systems, which compliment the systems included in the library

Reworked CMake build system, courtesy of Jonny 'The Machine' Paton, using
modern CMake syntax as well as being Visual Studio compatible.

All new reliable network connection classes dedicated to creating
gameplay oriented connections based upon the indomitable [Enet](http://enet.bespin.org/)

There is also a collection of utilites which may be useful for development
of projects in the 'extras' directory.

###### Contributions
Contributions should be acknowledged and are gratefully accepted from:  
Jonny Paton (Build system and CI setup and many usability improvements)  
Jean-Sebastien Fauteux (macOS Testing)  
Baard (Awesome Logo)


#### FAQ
###### Where's the 3D stuff gone?
While it had the potential to look very pretty the 3D API did not mesh
(pun intended ;) ) well with the 2D rendering. Mixing custom OpenGL, particularly
non-core 2.1 level, with SFML meant that it did not perform optimally, as
well as not working on macOS which doesn't support compatibility
profiles. If you really want 3D there are many better engines/frameworks out
there, including [crogine](https://github.com/fallahn/crogine), another
framework by myself which is optimised for 3D, particularly on mobile devices.
It features a very similar API to xygine too.

###### What about physics stuff?
The physics binding of Box2D has been removed, mostly because I wasn't using it.
It may return in the future, but thanks to the new ECS API implementing your
own binding to any physics engine of choice, or even implementing your own
collision detection is super easy. There's an example of a Chipmunk2D binding
in the 'extras' directory.

###### ...and the Tiled map support?
This had already been spun out into [tmxlite](https://github.com/fallahn/tmxlite)
and maintaining two versions of it seemed an unnecessary amount of work.
Tmxlite works easily with any project, and is even used in the demo project
included in this repository.

###### So I can't use the old version any more?
Sure you can. From now on the old revision is referred to in the documentation
as 'legacy', and [can be found in its own branch](https://github.com/fallahn/xygine/tree/legacy).
The wiki pages remain available, although the doxy generated content is
offline and will need to be generated from the docs directory using doxygen.

###### Why xygine?
The name is simply derived from the fact that this is a 2D framework -
hence xy, followed by the gine part of engine (although technically
xygine isn't really an engine).


xygine uses native file dialogue  
https://github.com/mlabbe/nativefiledialog  
and dear imgui  
https://github.com/ocornut/imgui  
https://github.com/eliasdaler/imgui-sfml  


/*********************************************************************  
(c) Matt Marchant & contributors 2017 - 2019  
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
