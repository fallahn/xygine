# Getting started with xygine - Part 3

### Overview

So far in the last two parts we've covered creating a new project, learned how to create 
entities which display text and sprites, and started making a breakout style game. If 
you run the project you should see a menu, be able to click play, and then be presented 
with a paddle on the screen which is controllable with a mouse. If not then review the 
first two parts of the tutorial before carrying on.

In this part we'll look at how resource management works in xygine, before creating 
custom components and systems to add a ball which collides with the arena.

### Resources

In the last part to get the paddle rendering on the screen we added an `sf::Texture` 
member to the `GameState` `m_paddleTexture` which is used to draw the paddle. As we'll 
be progressively be adding more textures from now on, it's time to do a small amount of 
refactoring. xygine has a `ResourceManager` class which provides an interface for 
loading multiple types of SFML resources such as fonts, textures, images and even sound 
buffers. Shaders are, however, a special case and won't be covered in this tutorial. The
`ResourceManager` is designed to ensure that only a single instance of any resource 
exists at one time and provides fast lookup of loaded resources via an integer ID. It 
also provides a fallback mechanism allowing you to handle any errors which may occur 
should loading a resource fail. For now we'll be using it to load all of the textures 
required for the game.

When a resource is loaded the `ResourceManager` returns a unique ID for that resource, 
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

        static std::array<std::int32_t, TextureID::Count> handles = {};
    }

Note the `std::array` named `handles`. This will hold the IDs returned from the resource 
manager, and will be indexed by the enum values above it. From now on whenever we add a 
new texture to the game we'll create a new member in the enum.

With this created open GameState.hpp and include the header for the `ResourceManager`

    xyginext/resources/ResourceManager.hpp

and remove the include directive for `sf::Texture`. Below remove the member 
`m_paddleTexture` and replace it with

    xy::ResourceManager m_resources;

Underneath `createScene()` add a new function, `loadResources()`. In GameState.cpp 
go to the definition of `createScene()` and find the line where the paddle texture is 
loaded. Remove it and replace it with a call to `loadResources()`.

Create a definition for `loadResources()` below. Here we'll load all of our textures and 
any other resources up front, and store the ID assigned to them by the resource manager.
Right now we're only using the paddle texture so add the line

    TextureID::handles[TextureID::Paddle] = m_resources.load<sf::Texture>("assets/images/paddle.png");

The `ResourceManager::load()` function is templated and can be used to load different 
kinds of resources. When loading fonts or images the same pattern of namespacing and 
handle array can be used.

Back in `createScene()`, above, modify the line where the `Sprite` component is added to
the paddle entity so that it uses the new resource manager.

    entity.addComponent<xy::Sprite>(m_resources.get<sf::Texture>(TextureID::handles[TextureID::Paddle]));

Build and run the project to check that every thing is OK, you should see the same 
paddle sprite as it was before. If you're feeling bold refactor `MyFirstState` in the 
same way, replacing m_font with a `ResourceManager`, and use it curate the font used in 
the menu.

---

