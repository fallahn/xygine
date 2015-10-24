xygine
------

![Logo by Baard](xygine/xygine.png?raw=true)

2D Game Engine Framework built around SFML. I use this as the starting  
point for new projects, as it evolves slowly over time. The source for  
xygine is released under the zlib license in the hope that it may be  
useful for other people - but offers no warranty that it is fit for any  
particular purpose, even those for which it was specifically written...  


#####Basic usage:

xygine compiles to a static library by default when using the included  
CMake file. Linux / Mac maybe be able to compile a shared library (I  
haven't tried it), but MSVC currently does not export any functions,  
and probably won't until I see some reason to add the option. The  
included solution / project file is for Visual Studio 2013, and mainly  
there for my own convenience. Once you have compiled the library with  
the compiler of your choice, simply link to your new project, along  
with the SFML libraries. xygine requires all five components of SFML -  
system, windows, graphics, audio and networking.  

To use the library create a new class which inherits xy::App and  
implement the virtual functions:

    void handleEvent(const sf::Event&);
    void handleMessage(const Message&);

    void registerStates();
    void updateApp(float dt);
    void pauseApp(float dt);
    void draw();

and, optionally, override finalise(); if you have any code which  
requires clean up on program exit.

    handleEvent()

and  

    handleMessage()

pass on any window event and system messages respectively. Handle these  
however you will.  

    registerStates()

is needed when using the state stack implementation of xygine. When  
creating new game states (deriving from xy::State) they need to be  
registered with the stateStack instance of your game. See the Example  
directory for a demo project which shows how to create a basic state  
which uses the UI controls to create a menu.  

Both

    updateApp()

and

    pauseApp()

pass on the current elapsed game time, fixed at 1/60 second. These are  
used to update logic when xygine is either running or paused, for  
example you may wish to make menus updatable while the game itself is  
paused.

    draw()

is the top level draw function. getRenderWindow().clear() and  
getRenderWindow().display() should both be called here, with your  
drawing code called in between.

Once you have created your app class, instanciate it in your main()  
function and call run()

    int main()
    {
    	MyApp game;
    	game.run();
    	return 0;
    }


#####Custom Components

Custom components can easily be created by inheriting from xy::Component  
and implementing its pure virtual functions. Two functions of note are  

    type() const
    
and

    uniqueType() const
    
which return IDs used internally by xygine. type() must return either  
xy::Component::Type::drawable or xy::Component::Type::script (or possibly  
xy::Component::Type::physics, although this is not implemented by default)  
to indicated the general use of the component. Returning a drawable type  
allows the engine to add the component to the rendering list, whereas  
script type components are used to define some kind of logic or behviour.  
Unique type IDs are used when accessing components attached to an entity.  
Custom components require a unique ID (defined internally as sf::Int32)  
which can be defined via an enum. xygine contains a few default components  
whose IDs are found in xy::Component::UniqueId. To extend these the  
recommended method is to create a new enum, where the first value is that  
of xy::Component::UniqueId::count. This ensures custom IDs are not only  
unique, but will remain so should more default components be added to  
xygine in the future.

    enum MyComponentID
    {
        TriangleComponent = xy::Component::UniqueId::count,
        LogicComponent,
        AIComponent //etc...
    }


For a detailed example of xygine take a look at  
[Pseuthe](https://github.com/fallahn/pseuthe) which was built using an  
early version of xygine.


######Why xygine?

The name is simply derived from the fact that this is a 2D framework -   
hence xy, followed by the gine part of engine (although technically  
xygine isn't really an engine).


-----------------------------------------------------------------------

Matt Marchant 2014 - 2015  
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
