# Getting started with xygine - Part 2

### Recap

In the first part of the tutorial we created a basic menu using the `MyFirstState` 
project template. Continuing on from that we'll add a new state, `GameState`, to host 
the game play logic, before taking a look at xygine's `CommandSystem` class.

### The Game State

Recalling the last tutorial you should be familiar with the concept of the `StateStack` 
which lives within the `Game` class. The stack is used to control the active state of 
the game, but before we can do that we need to create and register a new `GameState` 
class. All states inherit `xy::State` but for simplicity's sake rather than look at the 
specific `State` interface (which can be seen in detail in the documentation at 
https://github.com/fallahn/xygine/wiki) let's start by duplicating MyFirstState.hpp and 
MyFirstState.cpp and renaming them to GameState.hpp and GameState.cpp. In GameState.hpp 
remove the include line for `sf::Font`, and remove the `m_font` member as we won't be 
using it. Then rename `m_scene` to `m_gameScene` - this is important because later on 
there'll be another scene added to contain the user interface. Rename the class from 
`MyFirstState` to `GameState`. In GameState.cpp the process is similar: rename all 
`m_scene` references to `m_gameScene` and all `MyFirstState` references to `GameState`. 
This includes changing the name of the included header file "MyFirstState.hpp" to 
"GameState.hpp" at the top of the document. There's no need to include the headers for 
the Text component or TextSystem either, but leave the remaining includes as they will 
be used later on. The contents of `createScene()` can be removed too, although you can 
leave the lines where the `RenderSystem` is added, as that'll be used when drawing the 
game. The line 

    m_scene.getSystem<xy::UISystem>().handleEvent(evt);

needs to be removed from `GameState::handleEvent()` as there is no `UISystem` in use in 
the GameState.

A little book keeping is required if you're using the CMake files to build the tutorial.
In the include directory edit the `CMakeLists.txt` file and add a new line referencing 
GameState.hpp. In the src directory repeat the edit for GameState.cpp.

To register the state with the game the `GameState` needs a unique ID to return from the
function `GameState::stateID()`. Open the States.hpp file and add a new member to the 
State enum called GameState. Modify the `GameState::stateID()` function to return 
`States::GameState`. This is important because the `StateStack` needs to be able to 
identify individual states. With this done open Game.cpp and below the include line 
for "MyFirstState.hpp" add an include directive for "GameState.hpp". Then in the 
function body of `Game::registerStates()` add

    m_stateStack.registerState<GameState>(States::GameState);

This will associate the specific state type (GameState) with its enum value. This way 
requesting a new state on the stack can be done with a single ID. Finally, to launch 
this new state, we need to modify `MyFirstState`.

Open MyFirstState.cpp and go to the definition of `createScene()`. The process of adding
 a new callback is the same as adding the quit button callback, only this time it should
 be added to the entity with the text component that says 'Play'. The lambda expression 
used in the callback needs to be modified, however, else clicking Play will close the 
game! The callback should look like this:

    auto callbackID = 
    m_scene.getSystem<xy::UISystem>().addMouseButtonCallback(
        [&](xy::Entity e, sf::Uint64 flags)
        {
            if(flags & xy::UISystem::LeftMouse)
            {
                requestStackClear();
                requestStackPush(States::GameState);
            }
        });

Note that there are two stack request functions here. `requestStackClear()` will remove 
ALL active states from the state stack. There is a similar function `requestStackPop()` 
which will remove only the topmost state from the stack that would also work here as 
there is only one state currently active, but requesting that the stack be cleared 
better displays intent. The next line `requestStackPush()` takes a stateID as a 
parameter, so that the `StateStack` knows which state is being requested of it. If 
you're wondering why the function names are prefixed with 'request' this is because 
state operations are delayed - they are 'requested' and the `StateStack` will only 
perform these requests at the beginning of the next frame if it is safe to do so.

With this done the new `GameState` should be ready to use. Build and run the source and 
when the menu is displayed click 'Play'. The menu should disappear and be replaced with 
a blank screen. This is because we've not yet added anything to the `GameState` - but if
you want some confirmation of the state change, edit the constructor of `GameState` and
add

    xy::Logger::log("Open game state!", xy::Logger::Type::Info);

Build and run the game again, this time when Play is clicked and the blank screen 
appears the "Open game state!" message should appear in the system console. If you don't
have a console window open press F1 to open the xygine console, where the timestamped 
message will be displayed.

---

