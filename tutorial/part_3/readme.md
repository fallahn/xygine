# Getting started with xygine - Part 3

### Overview

In the last two parts we've covered creating a new project, learned how to create 
entities which display text and sprites, and started making a breakout style game. If 
you run the project you should see a menu, be able to click play, and then be presented 
with a paddle on the screen which is controllable with a mouse. If not then you should 
review the first two parts of the tutorial before carrying on.

In this part we'll look at how resource management works in xygine, before learning how 
to create custom components and systems to add a ball.

### Resources

In the last part to get the paddle rendering on the screen we added an `sf::Texture` 
member to the `GameState`, `m_paddleTexture` which is used to draw the paddle. As we'll 
be progressively be adding more textures from now on, it's time to do a small amount of 
refactoring. xygine has a `ResourceHandler` class which provides an interface for 
loading multiple types of SFML resources such as fonts, textures, images and even sound 
buffers. Shaders are, however, a special case and won't be covered in this tutorial. The
`ResourceHandler` is designed to ensure that only a single instance of any resource 
exists at one time and provides fast lookup of loaded resources via an integer ID. It 
also provides a fallback mechanism allowing you to handle any errors which may occur 
should loading a resource fail. For now we'll be using it to load all of the textures 
required for the game.

When a resource is loaded the `ResourceHandler` returns a unique ID for that resource, 
which can be used to quickly retreive it when it is needed at run time. A little book 
keeping is needed then, to store these IDs.

Create a new file in the include directory named `ResourceIDs.hpp`. Make sure to add it 
to the CMakeLists.txt file if you're using the CMake build system. Inside the new file 
add a namespace named `TextureID` and an anonymous enum like so:

    namespace TextureID
    {
        enum
        {
            Paddle,
            Ball,
            Count
        };

        static std::array<std::size_t, TextureID::Count> handles = {};
    }

Note the `std::array` named `handles`. This will hold the IDs returned from the resource 
manager, and will be indexed by the enum values above it. From now on whenever we add a 
new texture to the game we'll create a new member in the enum.

With this created open GameState.hpp and include the header for the `ResourceHandler`

    xyginext/resources/ResourceHandler.hpp

and remove the include directive for `sf::Texture`. Below that remove the member 
`m_paddleTexture` and replace it with

    xy::ResourceHandler m_resources;

Underneath `createScene()` add a new function, `loadResources()`. In GameState.cpp 
go to the definition of `createScene()` and find the line where the paddle texture is 
loaded. Remove it and replace it with a call to `loadResources()`.

Create a definition for `loadResources()` below. Here we'll load all of our textures and 
any other resources up front, and store the ID assigned to them by the resource manager.
Right now we're only using the paddle texture so add the line

    TextureID::handles[TextureID::Paddle] = m_resources.load<sf::Texture>("assets/images/paddle.png");

The `ResourceHandler::load()` function is templated and can be used to load different 
kinds of resources. When loading fonts or images the same pattern of namespacing and 
handle array can be used.

Back in `createScene()`, above, modify the line where the `Sprite` component is added to
the paddle entity so that it uses the new resource manager.

    entity.addComponent<xy::Sprite>(m_resources.get<sf::Texture>(TextureID::handles[TextureID::Paddle]));

Build and run the project to check that every thing is OK, you should see the same 
paddle sprite as it was before. If you're feeling bold refactor `MyFirstState` in the 
same way, replacing `m_font` with a `ResourceHandler`, and use it curate the font used in 
the menu.

---

### Balls.

The essence of an ECS is to 'break apart' deeply inherited classes, isolating the data 
of an object into smaller groups such as transform data (position, rotation etc) and 
render data (texture IDs, render states) and storing it in sets of structs, away from 
the logic part of the entity. These structs become our components, providing the ability
to select which sets of data are associated with an entity, while the logic used for 
processing is placed within a system. Systems have a filter applied to them which states
the component requirements an entity must have for the system to be interested in it. An 
entity may have more components than a system requires - but not less.

xygine already provides the `Transform` and `Sprite` components needed to make a ball 
appear on screen, but a `Ball` needs other data such as velocity and state (is the ball 
active? is it waiting to be launched?) which we can encapsulate in a custom struct. To 
make the ball move as we expect it also requires some logic to act upon the data stored 
in the `Ball` struct, which can be implemented in a `BallSystem`. It may seem like a lot
of code for a single ball, but when you consider what it takes to add 1 extra ball, or 
100 extra balls, suddenly the ability to scale with no extra code becomes apparent. 
Creating any new entity behaviour in xygine more or less follows this procedure.


#### Implementing the ball

Add a new file to the include directory and call it `BallSystem.hpp`. Update the 
CMakeLists.txt file too if you're using it. We'll add both the component and the 
system declarations to the header file as the component declaration is trivial.

    struct Ball final
    {
        enum class State
        {
            Waiting, Active
        }state = State::Waiting;

        sf::Vector2f velocity = {0.f, -1.f};
        static constexpr float Speed = 800.f;
    };

This is the `Ball` component. It contains an enum class declaring its state - when a 
ball is sat on the paddle waiting to be launched it'll be in the `Waiting` state. Once 
launched it'll be `Active`. The velocity is a normalised vector indicating the ball's 
current direction. It's initialised so that when it launches it'll move straight up. 
Lastly the constexpr value `Speed` is used to define the base speed of all balls. It 
makes more semantic sense to list it here than anywhere else.

After the component add the declaration of the `BallSystem`. It needs to publicly 
inherit the xygine `System` base class, and implement some of its interface. At the 
very least we need to fill out the `process()` function, although there are other 
virtual functions in `System` which may be useful. For a full breakdown of these see 
the xygine documentation or wiki. The `System` base class also needs to be constructed 
with a reference to the active `MessageBus`, so the `BallSystem` will need to take that 
as a parameter to the constructor.

    class BallSystem final : public xy::System
    {
    public:
        explicit BallSystem(xy::MessageBus&);

        void process(float) override;

    private:

    };

With these declared, create BallSystem.cpp in the src directory and update the 
CMakeLists.txt file. In BallSystem.cpp include the `BallSystem` header, as well as the 
xygine headers for the `Transform` component and `Scene` class. These classes will both 
be referenced by the system's process function.

The constructor of the `BallSystem` requires a couple of special things. First the 
initialiser list needs to correctly initialise the base class by forwarding the 
reference to the `MessageBus` passed in through the constructor, as well as providing 
the typeid of the `BallSystem`. This is used internally by xygine when the `Scene` 
processes all of its systems. The body of the constructor is also used to set up the 
'filter' by which the system decides which entities it should operate on. In this case 
we want to make sure that an entity has a `Ball` component and a `Transform` component. 
This is because the system will be modifying the position of the ball.

    BallSystem::BallSystem(xy::MessageBus& mb)
        : xy::System(mb, typeid(BallSystem))
    {
        requireComponent<Ball>();
        requireComponent<xy::Transform>();
    }

Next the `process()` function needs to be defined. This function is called once per 
frame by the `Scene` on every active system. The current frame time, or delta time, is 
passed in as a float. We'll be using this when moving the ball to make sure it moves 
the correct amount each frame.

    void BallSystem::process(float dt)
    {
        auto& entities = getEntities();
        for(auto entity : entities)
        {
            auto& ball = entity.getComponent<Ball>();
            switch(ball.state)
            {
                default:
                case Ball::State::Waiting:             
                    break;
                case Ball::State::Active:
                {
                    auto& tx = entity.getComponent<xy::Transform>();
                    tx.move(ball.velocity * Ball::Speed * dt);

                    sf::FloatRect bounds(sf::Vector2f(), xy::DefaultSceneSize);
                    if(!bounds.contains(tx.getPosition()))
                    {
                       getScene()->destroyEntity(entity);
                    }
                }
                    break;
            }
        }
    }

The function `getEntities()` returns a vector of entities which are active in the 
current system. By taking this and iterating over it, we can update each entity one at 
a time. For each entity then, we find its `Ball` component and check its state. If it is 
currently `Waiting`... do nothing. The `Ball` is on the `Paddle`. If the `Ball` is 
`Active`, on the other hand, then move it by its velocity multiplied by the 
`Ball::Speed` constant multiplied by the frame time.

After the ball is moved there is a simple check which takes the `Scene` size and looks 
to see if the `Ball` is still within the `Scene` area. If it is not the ball entity is 
destroyed.

*A side note:* it is possible for a `System` to be made `Drawable` by inheriting `sf::Drawable.` 
The `Scene` will attempt to draw all `System`s in the order in which they are added to 
the `Scene`.

Before we can start testing though the `BallSystem` has to be added to the `Scene`. 
Open the GameState.cpp file, include BallSystem.hpp and add an instance of the system 
to the `Scene` in `createScene()`, on the line before adding the `SpriteSystem`.

    m_gameScene.addSystem<BallSystem>(messageBus);

Unlike the paddle the `Ball` isn't created once in `createScene()`. When the game runs 
we'll want to request multiple balls, so add a new function called `spawnBall()` to 
`GameState`. Call this immediately after creating the `Paddle` entity.

When the ball is spawned into the game it needs to be placed on the the paddle and 
follow the paddle around until it is launched. To do this we can create a `Paddle` 
component which references the active `Ball` entity. In BallSystem.hpp add a new struct

    struct Paddle final
    {
        xy::Entity ball;
    };

and add an instance of it as a component to the paddle entity in `createScene()`

    entity.addComponent<Paddle>();

As the `Entity` class is a handle for entities within the `Scene` it is also nullable and, 
in fact, is null by default. It has a function `isValid()` which returns true if the 
entity contains a valid handle. Using this we can tell whether or not the `Paddle` has 
a `Ball` entity waiting to be launched, and reset it as necessary with the `spawnBall()` 
function.

    void GameState::spawnBall()
    {
        xy::Command cmd;
        cmd.targetFlags = CommandID::Paddle;
        cmd.action = [&](xy::Entity entity, float)
        {
            auto& paddle = entity.getComponent<Paddle>();
            paddle.ball = m_gameScene.createEntity();
            paddle.ball.addComponent<xy::Transform>();
            paddle.ball.addComponent<xy::Sprite>(m_resources.get<sf::Texture>(TextureID::handles[TextureID::Ball]));
            paddle.ball.addComponent<xy::Drawable>();
            paddle.ball.addComponent<Ball>();

            auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
            auto paddleBounds = entity.getComponent<xy::Sprite>().getTextureBounds();
            paddle.ball.getComponent<xy::Transform>().setOrigin(ballBounds.width / 2.f, ballBounds.height / 2.f);
            paddle.ball.getComponent<xy::Transform>().setPosition(paddleBounds.width / 2.f, -ballBounds.height / 2.f);

            entity.getComponent<xy::Transform>().addChild(paddle.ball.getComponent<xy::Transform>());
        };
        m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
    }

The function works by sending a command to the `Paddle` entity. We set up the 
`CommandSystem` in the previous tutorial to receive player input from the mouse. We can 
also use this system to tell the `Paddle` we want a new `Ball`, and to immediately attach that 
`Ball` to the `Paddle`. The `Transform` component has the option to add 'child' 
transforms, which in this case will be the `Transform` of the `Ball`. While the `Ball` 
transform is parented to the `Paddle` transform it will follow the `Paddle` on screen, 
which is what we want while the `Ball` is waiting to be launched.

To test this we can modify `handleEvent()` in `GameState` so that when the left mouse 
button is pressed it checks the `Paddle` to see if the `Ball` property is valid. If not 
it spawns a new `Ball`, else if it *is* valid then set the state of the `Ball` to 
`Active`, and unparent it from the `Paddle`.
Add this under where we check the MouseMove event.

    else if(evt.type == sf::Event::MouseButtonReleased)
    {
        if(evt.mouseButton.button == sf::Mouse::Left)
        {
            //send a command to the paddle to launch the ball if it has one
            //else spawn a new ball
            xy::Command cmd;
            cmd.targetFlags = CommandID::Paddle;
            cmd.action = [&](xy::Entity entity, float)
            {
                auto& paddle = entity.getComponent<Paddle>();
                if(paddle.ball.isValid())
                {
                    paddle.ball.getComponent<Ball>().state = Ball::State::Active;
                    auto ballBounds = paddle.ball.getComponent<xy::Sprite>().getTextureBounds();
                    paddle.ball.getComponent<xy::Transform>().setPosition(entity.getComponent<xy::Transform>().getPosition() + sf::Vector2f(0.f, -ballBounds.height / 2.f));
                    entity.getComponent<xy::Transform>().removeChild(paddle.ball.getComponent<xy::Transform>());
                    paddle.ball = {};
                }
                else
                {
                    spawnBall();
                }
            };
            m_gameScene.getSystem<xy::CommandSystem>().sendCommand(cmd);
        }
    }

Again we're using the `CommandSystem` to address the `Paddle` directly. Note also that 
when unparenting a `Ball` from the `Paddle` that the ball's position needs to be 
updated. While parented a `Transform`'s coordinates are relative to the parent (in this 
case the `Paddle`), so when unparenting the `Ball` its position needs to be set in world 
space coordinates. Finally, when unparenting the `Ball`, the `Paddle`'s ball property is 
reset with the default constructor, effectively nullifying it.

If you build and run the project now you should find that first clicking the left mouse 
button places a `Ball` on the paddle, and a second click fires the `Ball` in a straight 
line across the screen.

Now that we have the `Ball` and `Paddle` in place we need some basic collision detection
and physics, which will be the topic of the next tutorial.