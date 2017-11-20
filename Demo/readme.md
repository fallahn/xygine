xygine demo game
----------------

This directory contains the project for a small, networked,
multiplayer demo inspired by popular 80s arcade games. It is
intended as an example of what xygine is capable, but is also
quite playable. There is also a dedicated server project
included in the ServerTest directory of the repository.

Controls are via keyboard using the Doom keys (WASD on standard
GB/US layouts) or with the arrow keys. The space bar is used to fire.
Controllers are also supported with xbox style controllers in
mind. The default buttons are A to jump and B to fire, and the
D-Pad and left analogue stick will move. The help menu can be
accessed by clicking the ? sign on the main menu. From here
keys may be rebound for both player one and two, by clicking on
the currently bound key underneath the appropriate action icon.

Escape, P and Pause keys all bring up the the pause/quit menu in game.

F1 will open the console window, from there the Video and Audio
options can be accessed to change the window size and alter the
volume.

When hosting a game, servers expect incoming connections on port 40003.
This is also true when running a local game, so Windows users may see
a warning from Windows firewall the first time the game is run. Click
allow connections on this port to continue.

##### Gameplay
The idea of the game is to clear all the levels required for the player
to reach the top of the tower. Enemies can be defeated by trapping them
in a bubble before bursting it, or by triggering a power up which then
strikes the enemy. Power ups are colour coded to each player and come
in two varieties - fire bombs and ninja stars. Some maps contain crates
which can normally be pushed around and used to crush enemies, and on
special maps can be picked up and dropped - at the cost of being able
to shoot bubbles. Players can also collect Bonus Bubbles - bubbles
that contain letters to spell out the word 'BONUS'. Collecting all five
letters propels the player forward by five levels.
    Plenty of extra points can be gained by collecting the food items
dropped by defeated enemies, or by finding and wearing the magic crown.
Each 10,000 points earned rewards the player with an extra life. The
crown will also defand the player against a single blow from an enemy.

##### Maps
Maps are [Tiled](http://www.mapeditor.org) tmx format maps
which obey a few simple rules:

A tile layer. Tiles are 64px square, and the total map size is 16x17
tiles in size. There are two rows of empty tiles top and bottom to allow
for teleporting nodes. Multiple tile layers may exist, but will be rendered
to a single layer when the map is loaded and will always appear behind
other game sprites.


One object layer named 'geometry'. Objects placed on this layer must
be rectangular, with the exception of crates (see below). Other shapes
will automatically be converted to rectangles using their bounding boxes.
The maps directory contains a file called objecttypes.xml which can be
loaded into Tiled to define the needed object types. These are:

* platform - One way platforms which can be passed through from below
* solid - used to create impassable geometry, and ought to be used around
the four edges of the map
* teleport - these are used to teleport game entities from the bottom to
the top of the map and vice versa. Attempting to place these closer together
than the top and bottom edges of the map will cause undesirable behaviour.
* crates - which can also be placed as object tiles to aid visual placement.
Crates have a boolean property 'explosive' which when set to 'true' causes
the crate to explode when it is destroyed, and 'respawn' which cause crates
that have been destroyed to respawn after a short time if this is true.
There may be no more than 6 crates on a map. Any more are ignored by the game.

Maps require at least one solid type object and one platform type and will
fail to load if these are not found. Teleport and crate types are optional.


One object layer named 'spawn'. This contains objects which define the
points where enemies are spawned after the map is loaded. The objects
should be named for the enemy to spawn, currently these are:

* whirlybob - the blue bird
* clocksy - a walking bush
* balldock - a gurning orange chum
* squatmo - also known as the Nightmare Booger.

Other names are ignored. The object type is irrelevant as the game only loads
the positions. Object tiles may be used to help aid in visual design.

Maps have the following available properties:

* use_bubbles - An optional boolean property. If this is explicitly set to
false then the player will not be able to fire bubbles. Use this on maps
which have respawnable crates that can be used as weapons instead, as when
bubbles are disabled players are able to pick up crates to move them around.
* colour_quad - This represents the quadrant of a colour wheel used to
define the colour of the background of the current map. This can be set
to compliment the colour of the map geometry. The following values are
available
  * 0 - Yellow
  * 1 - Green
  * 2 - Blue
  * 3 - Red
* round_time - a floating point value, in seconds, that defines how long 
the map will run before the 'Hurry Up!' warning is given. Maps without a
value default to 39 seconds.

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