# Getting started with xygine - Part 6

### Pop & Sparkle - That Magic Touch
In the final part of the tutorial let's look at xygine's `Director` class, and see how 
it can be used to add some of the finer details to the game with the `AudioEmitter` and 
`ParticleEmitter` components.

### The Director
The `Director` class in xygine differs slightly from the other ECS based classes, in 
that while it is a member of the `Scene`, it doesn't interact directly with it.
The `Director` class instead monitors the `Scene` via the event queue and 
`MessageBus`, and dispatches commands or creates new entities based on what it has 
observed - hence the title `Director`.

The class itself is actually an abstract one - to use it we'll need to create our own 
class which inherits it. In this case we will use the `Director` to observe `Block`s 
being destroyed by using the `Director`'s message handler, and dispatch entities with 
an `AudioEmitter` and `ParticleEmitter` components attached to create a special effect. 
This class we'll call the `FXDirector`.

### Set up
Before diving into creating the new class I'll briefly cover two new components we'll be 
using. First is the `AudioEmitter`. `AudioEmitter` components require an audio resource 
to play, similarly to `Text` needing a font or `Sprite` requiring a `Texture`. 
`AudioEmitter`s can play two different types of resources, however. Short sounds can be 
buffered in memory or stored in an `sf::SoundBuffer`. `sf::SoundBuffer`s will happily be 
managed by the `xy::ResourceHandler` so that they can be shared between multiple 
`AudioEmitter` components. Before an `AudioEmitter` can play a sound it needs to have 
the source of the audio set with `AudioEmitter::setSource()`. This takes a reference to 
an `sf::SoundBuffer`, but it can also be passed a string. This string should contain a 
path to a resource on disk, in which case the the `AudioEmitter` will attempt to stream 
the sound directly from the file. This is useful for larger audio files such as music, 
and supports compressed formats like .ogg. When using an `AudioEmitter` it's worth 
considering what kind of audio it will be playing before you decide what to pass to 
`setSource()`.

To use the `AudioEmitter` in a `Scene` we'll need to add a `xy::AudioSystem`. The 
`AudioSystem` takes care of positional audio for any mono sounds, as well as making sure 
that the volume levels are correctly updated by the mixer displayed in the Audio tab, 
when opening the console with F1. It only ever needs to be added to a single active 
`Scene` as it will, by its nature, affect all audio output. In this case we'll add it to 
`m_gameScene`.

The `ParticleEmitter` component, as its name suggests, emits particles. The 
configuration of the emitter can be controlled via the `xy::EmitterSettings` struct 
which has properties for forces, velocities, textures... all of which control the 
appearance and behaviour of the particles emitted. A complete run down of these settings 
is beyond the scope of the tutorial, but they are described in detail in the xygine 
documentation. In the xygine repository there is a project named `SimpleParticleEditor` 
which can be used to adjust the settings of a particle emitter using a UI and provides 
real time feedback, useful for designing new effects. It also allows saving the settings 
to a .xyp file, which is not disimilar to the `SpriteSheet` .spt format. Using this tool 
I have created a particle settings file for this tutorial, which can be loaded with 
`xy::EmitterSettings::loadFromFile()` at runtime.

The `ParticleSystem` class is used to render the `ParticleEmitter` component. Particles 
are unique as renderables in that they do not require a `Drawable` component, and are 
therefore not drawn by the `RenderSystem`. This is due to the fact that they use a 
special GPU assisted render path. When adding the `ParticleSystem` to `m_gameScene` it 
needs to be added *after* the `xy::RenderSystem`, so that particles are drawn on top of 
everything else. While this does provide the benefit of GPU accelerated particles, it has 
the drawback of not being depth sorted with other `Drawable` entities, unfortunately.

---

### Creating the FXDirector
So now we're familiar with the components we want to use and the `AudioSystem` and 
`ParticleSystem` have been added to the `Scene`, we can start crafting our `FXDirector`. 
The idea is this: listen for `BlockEvent` messages in the message handler, then when we 
see a `Block` being destroyed dispatch an entity with an `AudioEmitter` and 
`ParticleEmitter` attached to it to play their effects at the position held in the the 
`BlockEvent` message.

Create a new header file in the include directory and call it `FXDirector.hpp`. Update 
the CMake file if necessary. Include
 
    xyginext/ecs/Director.hpp

so our class can inherit the `Director` interface. The declaration of `FXDirector` looks 
like this:

    class FXDirector final : public xy::Director
    {
    public:
        explicit FXDirector(xy::ResourceHandler&);

        void handleEvent(const sf::Event&) override {}
        void handleMessage(const xy::Message&) override;
        void process(float) override;

    private:
        xy::ResourceHandler& m_resources;
        xy::EmitterSettings m_particleSettings;

        std::vector<xy::Entity> m_entities;
        std::size_t m_nextFreeEntity;

        xy::Entity getNextFreeEntity();
        void resizeEntities(std::size_t);
        void doEffect(sf::Vector2f);
    };

The class is going to be constructed with a reference to a `ResourceHandler`, which 
we'll pass in from the `GameState`. This is because we'll load our resources for the 
sound and particles in the `FXDirector` constructor.

The three public functions implement the `Director` interface. We're not using the event 
handler, so this has a default empty body.

Rather than create a new entity every time the `FXDirector` dispatches one we'll take a 
pooling approach, so we have a `std::vector<xy::Entity>` to act as our pool and some 
book keeping functions to track free entities.

Add a new file to your project called FXDirector.cpp and update the CMake file. At the top 
include FXDirector.hpp plus any other headers required.

    #include "MessageIDs.hpp"

    #include <xyginext/resources/ResourceHandler.hpp>
    #include <xyginext/ecs/components/Transform.hpp>
    #include <xyginext/ecs/components/AudioEmitter.hpp>
    #include <xyginext/ecs/Scene.hpp>

We'll need to be able to identify incoming messages, as well as set up the relevant 
components and entities.

In the constructor let's load the required resources.

    FXDirector::FXDirector(xy::ResourceHandler& rh)
        : m_resources    (rh),
        m_nextFreeEntity (0)
    {
        m_particleSettings.loadFromFile("assets/particles/impact.xyp", rh);

        AudioBufferHandle = rh.load<sf::SoundBuffer>("assets/sound/boop.wav");
    }

The particle settings struct can load the settings file directly from disk. It also 
takes a reference to the `ResourceHandler`, as it will be needed to load any textures 
required by the particle system. `AudioBufferHandle` is used to store the handle we get 
from the `ResourceHandler` and is placed in an anonymous namespace at the top of the 
.cpp file. We'll be using it to assign the `sf::SoundBuffer` to the `AudioEmitter` 
components.

    void FXDirector::handleMessage(const xy::Message& msg)
    {
        if (msg.id == MessageID::BlockMessage)
        {
            const auto& data = msg.getData<BlockEvent>();
            if (data.action == BlockEvent::Destroyed)
            {
                doEffect(data.position);
            }
        }
    }

The message handler picks up any incoming `BlockEvent` messages, and when a `Block` is 
destroyed uses `doEffect()` to dispatch an entity to the correct position. As an 
exercise I encourage you to expand on this later with messages raised by the ball 
bouncing off the walls to play a different sound.

    void FXDirector::process(float)
    {
        //check all entities and free any which have finished playing all the effects
        for (auto i = 0u; i < m_nextFreeEntity; ++i)
        {
            if (m_entities[i].getComponent<xy::AudioEmitter>().getStatus() == xy::AudioEmitter::Stopped
                && m_entities[i].getComponent<xy::ParticleEmitter>().stopped())
            {
                //swaps the expired entity with the last active entity
                //and decrements i so we're up to date in the loop
                auto entity = m_entities[i];
                m_nextFreeEntity--;
                m_entities[i] = m_entities[m_nextFreeEntity];
                m_entities[m_nextFreeEntity] = entity;
                i--;
            }
        }
    }

The `process()` function loops over the entity pool and checks the status of any playing 
effects. If an effect has completely stopped the inactive entity is re-pooled and the 
active count updated.

    xy::Entity FXDirector::getNextFreeEntity()
    {
        if (m_nextFreeEntity == m_entities.size())
        {
            resizeEntities(m_entities.size() + MinEntities);
        }
        return m_entities[m_nextFreeEntity++];
    }

    void FXDirector::resizeEntities(std::size_t size)
    {
        auto currSize = m_entities.size();
        m_entities.resize(size);

        for (auto i = currSize; i < size; ++i)
        {
            m_entities[i] = getScene().createEntity();
            m_entities[i].addComponent<xy::Transform>();
            m_entities[i].addComponent<xy::AudioEmitter>().setSource(m_resources.get<sf::SoundBuffer>(AudioBufferHandle));
            m_entities[i].addComponent<xy::ParticleEmitter>().settings = m_particleSettings;
        }
    }

These two functions are used for book keeping of the entity pool. The first finds the 
next available entity and returns it. If the pool currently has no free entities 
then the function below is called to resize the pool. This is where entities are 
actually created, adding the `Transform`, `AudioEmitter` and `ParticleEmitter` 
components. Note that here the audio source is applied to the `AudioEmitter` and the 
particle settings are applied to the `ParticleEmitter`.

    void FXDirector::doEffect(sf::Vector2f position)
    {
        auto entity = getNextFreeEntity();
        entity.getComponent<xy::Transform>().setPosition(position);
        entity.getComponent<xy::AudioEmitter>().play();
        entity.getComponent<xy::ParticleEmitter>().start();
    }

Finally `doEffect()` requests the next free entity from the pool, applies the position 
and starts the effects.

To use the `FXDirector` in the `Scene` it needs to be added similarly to a `System` 
class. In `GameState::createScene()` underneath where the last System is added to 
`m_gameScene` add

    m_gameScene.addDirector<FXDirector>(m_resources);

Don't forget that `m_resources` needs to be passed to the function so that it is 
correctly forwaded to the `FXDirector` constructor.

That's it! Build and run the game, and when a `Block` is destroyed you should now see a 
small shower of sparks and hear a sound play!

---

### Taking it further
While this is the end of the tutorial, it is by no means the end of what can be 
achieved with xygine. The documentation https://github.com/fallahn/xygine/wiki covers 
many more features that are available, a few of which are:

 - AudioScapes. Analogous to `SpriteSheets` these are text based configuration files 
   used to define `AudioEmitter` settings outside of xygine. For an example of how 
   these are used see the Demo application in the xygine repository.

 - Post Process effects. Shaders haven't been covered at all in this tutorial but they 
   are available in xygine. If they are compatible with SFML then they can be applied 
   any `xy::Drawable` component, and Scene-wide via the `PostProcess` class. xygine 
   contains a few built in `PostProcess` effects that can be added to the `Scene` via 
   `Scene::addPostProcess<T>()`, and of course you can implement your own.

 - Animated Sprites. `xy::Sprite` fully supports multiframe animation via the 
   `SpriteAnimator` system, in combinations with the `SpriteAnimation` component and 
   metadata provided by the `SpriteSheet` format.

 - Networking. xygine has complete network support via the Enet game networking library.
   The Demo application in the xygine repository implements a 2 player online network 
   mode by way of example.

 - Utility functions for vector maths, strings and random numbers. The `xy::Utility` 
   namespace contains various functions for operating on 2D vectors such as normalise 
   and dot product, as well as string functions for UTF encoding and random number generators
   which can be seeded with custom values.

 - Optional extras. The extras directory in the xygine repository contains useful other 
   classes such as a `PhysicsSystem` to bind chipmunk2D to xygine for physics 
   simulation. It also has networking utilities and prototype drawing functions for 
   simple lines and shapes.

Thank you for making it to the end of this tutorial! I hope xygine is useful to anyone wanting to make games with C++ and SFML (and I'd love to see anything you make with it!). If you have any comments or feedback about this tutorial, or anything else about xygine, please feel free to post on the github repository page.

https://github.com/fallahn/xygine/