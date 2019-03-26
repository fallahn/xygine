# Getting started with xygine - Part 4

### Before we begin
In this part of the tutorial we'll be looking at how to implement collision in the breakout 
game we've been creating with xygine. Collision tends to be one of the more complex parts 
of game development and, as such, this part of the tutorial may become a bit wordy. 
Creating collisions requires only two xygine systems, one of which already comes with 
xygine, but they require some explanation. Because of this I've tried to cut back on 
explanation of other code, such as adding new graphical entities, as these are already 
covered in the previous parts of the tutorial. The collision process is based on a blog 
post I wrote some time back, which you can read here: 
https://trederia.blogspot.com/2016/02/2d-physics-101-pong.html 
I'll try to refrain from reiterating anything too much and stick to explaining the actual 
implementation - so I highly recommend reading the blog post first to give you some idea of 
what we're trying to do.

### Setting up
In the previous paragraph I mentioned that there are only two systems needed for collision. 
Strictly speaking there's only one which is really necessary, but xygine includes a spatial 
partitioning system which can come in handy for optimising collision detection. The 
`DynamicTreeSystem`, as it's known, is an implementation of the balanced AABB tree based on 
the source of Box2D (and, to some extent, bullet physics. 
See [here](https://github.com/fallahn/xygine/blob/master/xyginext/include/xyginext/ecs/systems/DynamicTreeSystem.hpp#L68)
for more info). While it may be a slight 'over-optimisation' for our use case I'm including 
it because it's worth knowing about when scaling up a xygine project to something more 
complex.

Firstly, then, add the include directive for the `DynamicTreeSystem` to `GameState`, and 
add it to the list of `Scene` systems in `createScene()`

    xyginext/ecs/systems/DynamicTreeSystem.hpp

The matching component for this system is called the `BroadphaseComponent` as it is 
generally used in the 'broad phase' or first-pass of culling items from the list of 
potential collisions.

    xyginext/ecs/components/BroadphaseComponent.hpp

This component has one property in particular which can also be useful in the soon-to-be 
created `CollisionSystem`, namely an AABB (axis aligned bounding box) in the form of an 
`sf::FloatRect`. This AABB is used by the dynamic tree to parition the `Scene`, and quickly 
retrieve only other AABBs that are nearby.

We'll come back to deploying the `BroadphaseComponents`, but before that let's start on the 
`CollisionSystem`.

### Collision System Overview
The `CollisionSystem` requires much the same initial set up as our `BallSystem`. We'll add 
`CollisionSystem.hpp` and `CollisionSystem.cpp` to the relevant directories, and update the 
CMake files. The header is used to declare the `Collider` component as well as the 
`CollisionSystem` which, as before, inherits `xy::System` and implements the `process()` 
function.

The collider component is a simple struct with two members:

    struct Collider final
    {
        bool dynamic = false;
        std::function<void(xy::Entity, xy::Entity, Manifold)> callback;
    };

The dynamic bool is used to indicate if the collidable moves, or is static geometry. In the 
game we're creating the only moving object is the ball - while the paddle is moved by the 
mouse it's not considered 'dynamic' as it doesn't react to collisions.

The `std::function` is a little more complicated. Here we optionally add a callback 
function, which is called at the end of a collision, if it exists. For example once a 
collision has been resolved and the objects are no longer overlapping, calling this on the 
ball should execute a function updating the velocity so that it bounces off of solid 
objects. The callback takes three parameters - the entity whose collision was just 
resolved, the entity with which the first entity just collided, and a copy of the 
collision manifold. For an explanation of the manifold I recommend reading 
[this](https://trederia.blogspot.com/2016/02/2d-physics-101-pong.html) blog post about 
collisions. The declaration of the manifold struct is also in CollisionSystem.hpp

As with the `BallSystem` the `CollisionSystem` has two parts to its definition. In the 
constructor the filter is set up to declare which entities the `CollisionSystem` is 
interested in

    CollisionSystem::CollisionSystem(xy::MessageBus& mb)
        : xy::System(mb, typeid(CollisionSystem))
    {
        requireComponent<xy::Transform>();
        requireComponent<xy::BroadphaseComponent>();
        requireComponent<Collider>();
    }

Notice that we're requiring the `BroadphaseComponent` be present on the entities. The 
`process()` function is quite a bit lengthier than that of the `BallSystem`, so rather 
than list the entire code (the source of which you can find in the repository) I'll outline 
the steps it performs:

First, as with most systems, get the list of active entities with `getEntities()` and then 
start iterating over it.

For each entity get the `Collider` component and check if it's dynamic. We can ignore 
static `Colliders` as they won't move or create new collisions with other static geometry. 
If it is dynamic, however, then let's check to see if it collided with anything.

This is where the `BroadphaseComponent` comes in (in fact we're performing the broad phase 
right here!) - taking the AABB of the dynamic object's `BroadphaseComponent` we can query 
the Scene's `DynamicTreeSystem`, which returns a list of entities that are *close by* the 
dynamic object, saving us from having to check every single object in the `Scene`.

    auto others = getScene()->getSystem<xy::DynamicTreeSystem>().query(bounds);

With this list of entities in hand, next check the AABB of each one to see if it intersects 
with the dynamic object's AABB. If it does, insert it into a 
`std::set<std::pair<xy::Entity, xy::Entity>> m_collisions`, which is a member of `CollisionSystem`.

    m_collisions.insert(std::minmax(entity, other));

I'm using `std::set` as it will make sure that each collision pair (when sorted with 
`minmax()`) is inserted only once. This way the same collision isn't calculated twice, 
once from the perspective of each entity in the collision.

Once the broad phase is complete and the entire list of entities has been iterated over 
we're left with a set of colliding pairs. These are now operated on by iterating over the 
set in a 'narrow phase'. For each pair in `m_collisions` the overlap is calculated and used to create the 
manifold. If either of the pair are dynamic then the position of the dynamic object is corrected so that neither 
of the objects are overlapping. Finally, if there is a callback attached, the callback is 
executed for each of the colliders.

Phew.

### Deploying the CollisionSystem
With the system complete it's time to start hooking it up to the `Scene`. Include the header 
file in GameState.cpp and add an instance to the `Scene` in `createScene()`. When adding the 
`CollisionSystem` to the `Scene` it's probably worth making sure that it's added right 
after the `DynamicTreeSystem`, so that when the `CollisionSystem` is processed and it 
queries the `DynamicTreeSystem` we're sure the `DynamicTreeSystem` is up to date.

For the paddle and ball to collide we need to add both a `BroadphaseComponent` and 
`Collider` to each of them. Starting with the paddle add one of each:

    entity.addComponent<xy::BroadphaseComponent>(paddleBounds);
    entity.addComponent<Collider>();

The AABB is set on the `BroadphaseComponent` using the size of the `Sprite`, which we 
handily already have. In `spawnBall()` do the same, again using the `Sprite` size for the 
AABB bounds. Crucially here, however, we add a callback to the collider so that the `Ball` 
knows how to behave once it has collided.

    paddle.ball.addComponent<Collider>().callback = 
        [](xy::Entity e, xy::Entity other, Manifold man)
    {
        //if we hit the paddle change the velocity angle
        if (other.hasComponent<Paddle>())
        {
            auto newVel = e.getComponent<xy::Transform>().getPosition() - other.getComponent<xy::Transform>().getPosition();
            e.getComponent<Ball>().velocity = xy::Util::Vector::normalise(newVel);
        }
        else
        {
            //reflect the ball's velocity around the collision normal
            auto vel = e.getComponent<Ball>().velocity;
            vel = xy::Util::Vector::reflect(vel, man.normal);
            e.getComponent<Ball>().velocity = vel;
        }
    };

The callback checks to see if the ball collided with a paddle by testing the second entity 
for a `Paddle` component. If it finds one then the ball's velocity is set to that of the 
collision angle. This gives us the classic behaviour of skewing the bounce angle the 
further the ball is from the centre of the paddle. If the ball has collided with something 
else, say a wall or block, then the collision manifold is used to reflect the ball's 
velocity. The vector functions used here are found in xygine's `Utility` namespace, and are 
documented on the wiki page.

NOTE: The ball is not immediately set as a dynamic collider, as it's not considered dynamic 
while is sits on the paddle. In the event handler, which launches the ball on a button 
press, the `Collider` is also updated and its 'dynamic' property set to true.

Currently we can't really test the collision while the ball is free to fly out of 
the arena. In `createScene()`, below where the paddle entity is created, create three new 
entities, one for the top wall, and two for the left and right sides. These entities need 
only a `Transform` component along with the `Broadphase` and `Collider` components to 
function. The `xy::DefaultSceneSize` constant also comes in handy here when calculating the 
AABBs for the three entities. Of course it would be nice to also see these entities, which 
can be done by adding sprites with new textures added to the `TextureID` namespace. 
Alternatively the 'extras' directory in the xygine repository contains a header file called 
`ShapeUtils.hpp`. Adding this to your project will provide functions for quickly creating 
rectangle or circle shapes which are useful for prototyping. The wall entities  need only a 
`Drawable` component added for this:

    Shape::setRectangle(entity.addComponent<xy::Drawable>(), { wallBounds.width, wallBounds.height });

Now we're finally in a position to build and run the project to see the fruits of our labour. 
Running the game will give us the paddle and ball from before, but now when clicking the 
mouse the ball should move until it hits the top wall, and bounce off. Hitting the ball 
with different parts of the paddle will change its velocity angle. Awesome!


### Adding some polish with Messages
By now you've probably noticed that it's possible to spawn many balls at once by repeatedly 
clicking the mouse button. To stop this is pretty simple: go to `handleEvent()` and in the 
mouse button handler remove

    else
    {
        spawnBall();
    }

This doesn't help, however, when we lose the ball off the screen. To spawn a new 
ball as soon as the old one is lost, we can use xygine's `MessageBus`. The `MessageBus` is 
an application-wide list of messages which behave very much like Events, but can have 
arbitrary data placed on it from any class which can see it. This is why a reference to the 
`MessageBus` is passed to all `System` classes when they are created.

More detail of the `MessageBus` can be found in the xygine documentation, and on the wiki 
page, but all we need to know right now is that every message type needs a unique ID to 
identify it, and a struct declaration to state what kind of data the message holds. It is 
IMPORTANT that messages be kept small, however, and data be kept to POD such as numeric 
values and pointers. The maximum size of a message is 128 bytes.

We'll create a new message type used to notify the rest of the game when a ball is spawned 
and despawned. Any part of the game listening to these messages can then perform actions 
such as playing a sound or reducing the player's ball count. We can also use it to spawn a 
new ball.

Create a new header file called `MessageIDs.hpp` and add a reference to it to the relevant 
CMake file. Include the xygine Messages header

    xygine/core/Message.hpp

and add a new namespace to contain the message IDs

    namespace MessageID
    {
        enum
        {
            BallMessage = xy::Message::Count
        };
    }

Note that the first ID actually has the value of `xy::Message::Count`. This is very 
important as xygine has its own built in IDs, and overwriting any of them will cause no end 
of confusing bugs.

Underneath the namespace add a new struct to hold the message data

    struct BallEvent final
    {
        enum
        {
            Spawned, Despawned
        }action;
        sf::Vector2f position;
    }

This should contain everything we'll need elsewhere in the game when the ball is spawned or 
removed from the area.

In `BallSystem::process()` find the line which checks to see if the ball has left the play 
area. Underneath raise a new message:

    auto* msg = postMessage<BallEvent>(MessageID::BallMessage);
    msg->action = BallEvent::Despawned;
    msg->position = tx.getPosition();

That's all it takes to create a new message. Note that `postMessage()` exists in all 
`System` classes, and takes the event data type (`BallEvent` in this case) as the template 
parameter. `MessageID::BallMessage` is passed as a parameter so that the data can be 
correctly identified by any message handler that may receive it. The function returns a 
pointer to the newly created message so that any information about the event can be filled 
out.

Back in `GameState.cpp` find the `handleMessage()` function, and update it with a handler 
for our new message type.

    if(msg.id == MessageID::BallMessage)
    {
        const auto& data = msg.getData<BallEvent>();
        if(data.action == BallEvent::Despawned)
        {
            spawnBall();
        }
    }

It's important to first check that the message received is what you're looking for. Calling 
`Message::getData()` with the incorrect template parameter will cause anything from subtle 
bugs to a horrible crash. Once you have a reference to the data any of the properties can 
be read out, in this case checking the action and spawning a new ball should the existing 
ball be despawned.

That's it! Messages are quite a powerful concept in xygine and you should get used to 
raising them for all pertinent events such as collisions, spawning, despawning as it makes 
tracking the state of the game much easier.

### Final Step
As I stated at the beginning, this tutorial section is somewhat long so, as an exercise, 
I'm not going to cover creating the blocks explicitly. By now you should know enough about 
creating entities, adding sprites, collision data and even the mechanism for destroying 
blocks (hint: Collider callback) to be able to implement them on your own. Also try raising
 messages when blocks are destroyed as these events will be useful later to track the state 
of game, such as the number of remaining blocks or what score to award the player. Good 
luck! In the next part of the tutorial I'll be covering the game rules, and drawing the UI.