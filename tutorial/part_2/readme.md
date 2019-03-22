# Getting started with xygine - Part 2

### Recap

In the first part of the tutorial we created a basic menu using the `MyFirstState` 
project template. Continuing on from that we'll add a new state, `GameState`, to host 
the game play logic, before taking a look at xygine's `Sprite` component and the 
`CommandSystem` class.

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
the `Text` component or `TextSystem` either, but leave the remaining includes as they will 
be used later on. The contents of `createScene()` can be removed too, although you can 
leave the lines where the `RenderSystem` is added, as that'll be used when drawing the 
game. The line 

    m_scene.getSystem<xy::UISystem>().handleEvent(evt);

needs to be removed from `GameState::handleEvent()` as there is no `UISystem` in use in 
the `GameState`.

A little book keeping is required if you're using the CMake files to build the tutorial.
In the include directory edit the `CMakeLists.txt` file and add a new line referencing 
GameState.hpp. In the src directory repeat the edit for GameState.cpp.

To register the state with the game the `GameState` needs a unique ID to return from the
function `GameState::stateID()`. Open the States.hpp file and add a new member to the 
`State` enum called `GameState`. Modify the `GameState::stateID()` function to return 
`States::GameState`. This is important because the `StateStack` needs to be able to 
identify individual states. With this done open Game.cpp and below the include line 
for "MyFirstState.hpp" add an include directive for "GameState.hpp". Then in the 
function body of `Game::registerStates()` add

    m_stateStack.registerState<GameState>(States::GameState);

This will associate the specific state type (`GameState`) with its enum value. This way 
requesting a new state on the stack can be done with a single ID. Finally, to launch 
this new state, we need to modify `MyFirstState`.

Open MyFirstState.cpp and go to the definition of `createScene()`. The process of adding
a new callback is the same as adding the quit button callback, only this time it should
be added to the entity with the text component that says 'Play'. The lambda expression 
used in the callback needs to be modified, however, else clicking Play will close the 
game! The callback should look like this:

    auto callbackID = 
    m_scene.getSystem<xy::UISystem>().addMouseButtonCallback(
        [&](xy::Entity, sf::Uint64 flags)
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

### Scene Setup

Now that the `GameState` is created we have an arena to finally start implementing the 
game. Firstly, to ensure a consistent experience, the camera needs to be fixed to a set 
size across all states, and at all window resolutions. xygine uses a default size of 
1920x1080 world units, which is automatically applied to a view inside the `StateStack` 
when the window is resized. This way the same view is automatically letterboxed and 
mapped to the window. To apply the calculated view on startup, navigate to 
`Game::initialise()` in Game.cpp, and below the line which says 

    getRenderWindow()->setKeyRepeatEnabled(false);

add

    getRenderWindow()->setView(m_stateStack.updateView());

This line will force the stack to calculate an up-to-date view and immediately apply it 
to the RenderWindow when the game is initialised.

xygine also has a `Camera` component, used by a `Scene` to dictate what should be 
rendered to the screen. By default this component is initialised to the window size as
it makes no assumptions as for what it shall be used (for instance a `Camera` may only
show a small area in a `Scene` used to create a mini-map) so for our two states we'll need
to initialise it to the StateStack's calculated view. Handily this view is passed as one of
the properties of the `Context` struct which is given to the constructor of every state.
In the constructor of both `MyFirstState` and `GameState`, below the call to 
`createScene()`, update the state's `Scene` with the view stored in the context:

    m_scene.getActiveCamera().getComponent<xy::Camera>().setView(ctx.defaultView.getSize());
    m_scene.getActiveCamera().getComponent<xy::Camera>().setViewport(ctx.defaultView.getViewport());

adjusting the `Scene` variable name accordingly for each state.


### Adding a Paddle

Now that the camera is set up it's time to start on our paddle entity. Creating the 
entity will be very similar to creating the `Text` entities in `MyFirstState`, only the 
`Text` component will be replaced with a `Sprite` component. At the top of the file add 
two includes for the `Sprite` component and the `SpriteSystem`

    xyginext/ecs/components/Sprite.hpp
    xyginect/ecs/systems/SpriteSystem.hpp

The `Drawable` component and `RenderSystem` headers should already be there, but if not 
add those too

    xyginext/ecs/components/Drawable.hpp
    xyginext/ecs/systems/RenderSystem.hpp

In `createScene()` an instance of the `SpriteSystem` needs to be added to the `Scene` 
one line before the `RenderSystem` is added

    m_gameScene.addSystem<xy::SpriteSystem>(messageBus);

The rest of the `createScene()` function follows the same pattern as `MyFirstState`. As 
we're using a `Sprite` instead of a `Text`, however, the font from `MyFirstState` is 
replaced with an `sf::Texture`. For now add a new member to the `GameState` class 
`sf::Texture m_paddleTexture`. This will eventually be replaced with a resource holder, 
but for now it will do. Back in the definition of `createScene()` load the paddle image 
into the texture

    m_paddleTexture.loadFromFile("assets/images/paddle.png");

The image is included in the tutorial repository, but you will need to make sure the 
path is set correctly relative to your working directory, depending on your development 
environment.

Assuming the texture is now loaded correctly we can create a single entity to act as the
paddle.

    auto entity = m_gameScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(xy::DefaultSceneSize.x / 2.f, xy::DefaultSceneSize.y - 40.f);
    entity.addComponent<xy::Sprite>(m_texture);
    entity.addComponent<xy::Drawable>();

This looks very similar to creating a text entity in `MyFirstState`, with the difference
that the `Text` component / font combo is replaced with a `Sprite` component and 
texture. The paddle is positioned based on a built-in constant `xy::DefaultSceneSize`. 
This is the same size as the StateStack's calculated view, so we can be sure that the 
paddle will be aligned relative to the bottom centre of the screen as we expect, 
regardless of the size or resolution of the window.

To neaten up the layout of the paddle we can take the Sprite's texture bounds and use it
to align the origin to the centre of the `Sprite`

    auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Transform>().setOrigin(paddleBounds.width / 2.f, paddleBounds.height / 2.f);

The `Sprite` function `getTextureBounds()` differs from `getTextureRect()` in the fact 
that it returns the resulting size of the `Sprite` based on the current texture bounds, 
without the offset coordinates into the texture itself. In other words it is the same 
size as the current texture bounds, but the left and top properties are always zero.

Build and run the tutorial, then click Play on the menu. At the bottom centre of the 
screen you'll see our newly created paddle!

---

### Player Input

To make the paddle respond to mouse input we can use xygine's `CommandTarget` component 
with a `CommandSystem` added to the `Scene`. The `CommandTarget` merely holds an integer
ID used to identify which entities should be responding to specific commands. Add a new 
header file to the include directory and name it `CommandIDs.hpp`. Inside create a new 
namespace `CommandID` and add an anonymous enum. This enum will contain all the command 
IDs which we'll eventually create - to start with add a member named `Paddle` and give 
it the value 0x1. Each time a new member is added *its value should double* that of the 
previous member. This is so that multiple IDs can be OR'd together as flags. For more 
information on this see the `CommandTarget` documentation on the xygine wiki. When you 
have done this include the file at the top of GameState.cpp. If you're using the CMake 
files to build the project don't forget to add the CommandIDs.hpp entry to the 
CMakeLists.txt in  the include directory.

To assign the ID to our paddle include the header for the `CommandTarget` component

    xyginext/ecs/components/CommandTarget.hpp

and add the component to the paddle entity in `createScene()`. Because `addComponent()` 
returns a reference to the newly created component we can also assign the command ID 
immediately

    entity.addComponent<xy::CommandTarget>().ID = CommandID::Paddle;

To be able to process the commands we're going to send we also need an instance of the 
`CommandSystem` in the `Scene`. Include the `CommandSystem` header

    xyginext/ecs/systems/CommandSystem.hpp

and add it to the `Scene` before the `SpriteSystem` in `createScene()`

    m_gameScene.addSystem<xy::CommandSystem>(messageBus);

Now we're ready to start sending commands!

The `GameState` class has a function named `handleEvents()` in which we can pick up any 
mouse movement events passed down from the `Game` class. To identify events first 
include the SFML event header

    SFML/Window/Event.hpp

before moving to `handleEvent()`. At the top of the `handleEvent()` function add

    if(evt.type == sf::Event::MouseMoved)
    {
        auto worldMousePosition = xy::App::getRenderWindow()->mapPixelToCoords({evt.mouseMove.x, evt.mouseMove.y});
        xy::Command cmd;
        cmd.targetFlags = CommandID::Paddle;
        cmd.action = [worldMousePosition](xy::Entity entity, float)
        {
            //clamp the X position in the screen area minus the sprite width
            float posX = worldMousePosition.x;
            auto spriteWidth = entity.getComponent<xy::Sprite>().getTextureBounds().width / 2.f;
            posX = std::max(spriteWidth, worldMousePosition.x);
            posX = std::min(posX, xy::DefaultSceneSize.x - spriteWidth);
    
            auto& tx = entity.getComponent<xy::Transform>();
            auto currentPosition = tx.getPosition();
            currentPosition.x = posX;
            tx.setPosition(currentPosition);
        };
        m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }

This is simpler than it looks once we break it down:

First the event type is checked to make sure that it's a `MouseMoved` event. If it is, 
grab the mouse position contained in the event, which is in window coordinates. These 
are then converted to world coordinates using the `sf::RenderWindow` function 
`mapPixelToCoords()`. For more information on what this does see the SFML documentation, 
however for our purposes it converts the mouse coordinates into values which are relative 
to our `Scene` coordinates.

With the coordinates in hand we can now create our command

    xy::Command cmd;

and tell it to target our paddle entity by setting the target flags to the `CommandID` 
we gave the paddle.

    cmd.targetFlags = CommandID::Paddle;

The important part of the command is the lambda expression assigned to `cmd.action`. The 
signature takes an entity and a float as a parameter. When this command is executed the 
target entity, in this case the paddle, is passed in, along with the elapsed or delta 
time of the current frame. In this instance the delta time is not used so the parameter 
is omitted.

The mouse position is also copy-captured as it is needed to set the position of the 
paddle. In the body of the lambda the size of the paddle sprite is obtained and used to 
clamp the x position of the mouse coords. This prevents the paddle from leaving the 
screen area. Then the current paddle position is read, the x position updated, and the 
new position applied.

Once the lambda has been defined the command can be sent.

    m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);

Here it should be noted that commands are not executed immediately, rather they are 
placed on a stack, ready for the next frame. At the beginning of the next frame the 
entire stack is parsed and all the commands are executed sequentially. This is important 
when creating a lambda expression for the action, should it capture any variables by 
reference - the lambda is not executed in the same scope as it is written!!

If you build and run the tutorial now you'll find that when you move the mouse left or 
right the paddle will follow it. As a final touch we can hide the mouse cursor in the 
`GameState`. In the `GameState` constructor add a line after initialising the camera

    ctx.appInstance.setMouseCursorVisible(false);

That's it! In the next part of the tutorial we'll be looking at creating custom 
components and systems to add a ball entity, and resource management for fonts and textures.