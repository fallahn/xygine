xygine demo game
----------------

This directory contains the project for a small, networked,
multiplayer demo based on a popular 80s arcade game. It is
intended as an example of what xygine is capable of, but is also
quite playable. There is also a dedicated server project
included in the ServerTest directory of the repository.

Controls are via keyboard using the Doom keys (WASD on standard
US layouts) or with the arrow keys. The space bar is used to fire.
Controllers are also supported with xbox style controllers in
mind. The default buttons are A to jump and B to fire, and the
D-Pad and left analogue stick will move.

Escape, P and Pause keys all bring up the the quit menu in game.

F1 will open the console window, from there the Video and Audio
options can be accessed to change the window size and alter the
volume.

When hosting a game servers expect incoming connections on port 40003

##### Maps
Maps are simple [Tiled](http://www.mapeditor.org) tmx format maps
which obey a few simple rules:

A tile layer. Tiles are 64px square, and the total map size is 16x17
tiles in size. There are two rows of empty tiles top and bottom to allow
for teleporting nodes. Multiple tile layers may exist, but will be rendered
to a single layer when the map is loaded and will always appear behind
other game sprites.

One object layer named 'solid'. Rectangular objects placed on this layer
mark out solid map geometry which cannot be passed through. Other shapes
are ignored.

One object layer named 'teleport'. This doesn't necessarily have to have
any objects placed on it, but when there are they should be rectangular
and be evenly spaced top and bottom. Maps with teleport objects in them
also have bonus power ups enabled.

One object layer named 'platform'. This layer should contain only
rectangular objects (other types are ignored) which become one-way
platforms.

One object layer named 'spawn'. This contains objects which define the
points where enemies are spawned after the map is loaded. The objects
should be named for the enemy to spawn, currently these are:

* whirlybob
* clocksy

Other names are ignored.

Finally, maps have an optional property `round_time` which is a floating
point value, in seconds, that defines how long the map will run before the
'Hurry Up!' warning is given. Maps without a value default to 39 seconds.

Maps will appear in the playlist once they are copied to the assets/maps
directory, and the corresponding entry added to the mapcycle.txt file.

Pull requests for custom maps will be gratefully considered!




/*********************************************************************  
(c) Matt Marchant 2017  
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