# Getting started with xygine - Part 5

### Overview
In part four of the tutorial I left the task of creating block entities as an exercise 
for the reader, so I'll not be covering the code here. If you haven't added the blocks 
yet the source code for this part of the tutorial includes the code in 
`GameState::createBlocks()`. From here I'm going to assume the block code is functioning 
as we concentrate on tidying up the assets with a `SpriteSheet` and implementing a game 
UI overlay.

### Sprite Sheets
In part three we refactored asset management so that we are now using an instance of 
`xy::ResourceManager` to book keep the textures in use. However, rather than loading 
multiple textures it is possible, using `xy::SpriteSheet`, to merge the textures into a 
single file, a texture atlas, and load a text-based configuration file describing the 
`Sprite`s contained within. The assets directory for this part of the tutorial now 
contains a single image, composed of the `Paddle` and `Ball` textures and also a 
texture for the blocks. In the `assets/spritesheets` directory there is also a text file 
named `sprites.spt`. The `.spt` extension is used to denote sprite sheets used with 
xygine, although the format of the text file is the same `xy::ConfigFile` format used by 
all xygine description files, such as particle systems. More about the `xy::ConfigFile` 
format can be found in the documentation, but for now know that the `.spt` file contains 
information about the image source, sprite names from which it is composed, sub-regions 
describing the area of the atlas used by a specific sprite, as well as having the option
to describe animation data used by animated sprites. This system allows easy editing of 
sprite data externally from xygine, without having to recompile the game each time an 
animation is changed or sprite atlas modified.

To use the `SpriteSheet` in the game first rename the `TextureID` namespace to 
`SpriteID` in Resources.hpp. We're going to use the same technique as managing each 
texture, but instead apply it to the sprites. Add a new member to the enum for the 
`Block` sprite, and modify the handles array so that it is an array of `xy::Sprite` 
rather than `std::size_t`.

In `GameState::loadAssets()` where we previously loaded the textures, modify the code to 
use an instance of `xy::SpriteSheet`. You'll have to include the header

    xyginext/graphics/SpriteSheet.hpp

The `SpriteSheet` class can be used to load multiple `.spt` files from disk as we'll be 
copying the sprite data out of it and letting it drop out of scope when we're done.

    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/sprites.spt", m_resources);

Note that a reference to the resource manager is passed to the function, so that any 
loaded textures are still kept by the `ResourceManager` as usual.

When the sprite sheet is loaded (`xy::SpriteSheet::loadFromFile()` returns true on 
success) we can grab the loaded sprite data and store it in the `SpriteID` array

    SpriteID::sprites[SpriteID::Ball] = spriteSheet.getSprite("Ball");
    SpriteID::sprites[SpriteID::Paddle] = spriteSheet.getSprite("Paddle");
    SpriteID::sprites[SpriteID::Block] = spriteSheet.getSprite("Block");

That's pretty much it. Whenever you need to create a new entity with a sprite component 
preloaded sprites can easily be fetched from the array. For example the `Paddle` and 
`Ball` entities will now need to be modified:

    paddle.ball.addComponent<xy::Sprite>() = SpriteID::sprites[SpriteID::Ball];

For more information on the SpriteSheet class and how to handle animated sprites, see 
the xygine documentation, or review the source of the demo game included in the xygine 
repository.

---

### Adding A UI
By now the game should be somewhat playable, with the `Ball` respawning when it goes off 
screen, and breakable `Blocks` to aim at. To flesh out the gameplay ideally we want to 
track some stats such as the score and the player's lives, and display them on the 
screen. In a small project such as this it would be perfectly acceptable to render a UI 
using entities added to the game scene, but for demonstration purposes we'll create a 
second `Scene` in the `GameState` called `m_uiScene`. In a larger project, particularly 
one with a moving camera, it is often easier to manage items which remain constant in 
position with their own `Scene`. For example following a player around the world 
shouldn't modify the appearance of an inventory or health bars, which should remain 
static relative to the camera.

Adding another `Scene` is as simple as adding a new `xy::Scene` member to `GameState`, 
right below `m_gameScene`. Then the new `Scene` (`m_uiScene`) needs to forward events, 
forward messages, receive updates and be drawn, in exactly the same way as the game 
scene. It will also need to be properly constructed in the the initialiser list of 
`GameState`, and the current view applied to its active camera.

At the bottom of `createScene()` the new UI scene needs to have the necessary systems 
added to it. Right now those are `xy::TextSystem`, `xy::RenderSystem` and 
`xy::CommandSystem`. The command system will be used when the text in the UI needs to be 
updated. To do this also update the `CommandID` namespace with two new enum members and 
give them the correct values

    namespace CommandID
    {
        enum
        {
            Paddle = 0x1,
            Score = 0x2,
            Lives = 0x4
        };
    }

Remember that these values need to double each time as `CommandID`'s allow OR'ing 
together target IDs.

Then create two new entities below where we added the systems to the UI scene, 
remembering to use the `Entity` factory function from `m_uiScene`!

    entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(20.f, 20.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(50);
    entity.getComponent<xy::Text>().setString("Score: 0");
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Score;

    entity = m_uiScene.createEntity();
    entity.addComponent<xy::Transform>().setPosition(20.f, 60.f);
    entity.addComponent<xy::Text>(m_resources.get<sf::Font>(FontID::handles[FontID::ScoreFont]));
    entity.getComponent<xy::Text>().setCharacterSize(50);
    entity.getComponent<xy::Text>().setString("Lives: 3");
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::CommandTarget>().ID = CommandID::Lives;

Notice that the resource manager now also handles the font we want to use. Refer back 
to `MyFirstState` to see how fonts are loaded if you modified it to use a 
`ResourceManager` in part three. It is also the same technique used when loading 
textures and storing their resource handles, only within a `FontID` namespace instead of 
a `TextureID`.

Build and run the game to make sure everything is configured correctly, and that the 
`Score` and `Lives` text appears in the top left corner of the screen. If not, check 
that the UI scene is correctly added to `handleEvent()`, `handleMessage()`, `update()` 
and `draw()`.

### Updating the UI
In part four we covered using the `MessageBus` to raise messages when the `Ball` went 
out of play, and using the message handler to spawn a new `Ball` when it did. Hopefully, 
when creating the `Block` entities, you employed this technique to raise a 
`BlockEvent` message when a `Block` is destroyed. If not, reviewing the source in the 
repository if necessary, add that now. We want to be able to recieve a message each time 
a new `Block` is created, and each time one is destroyed.

With the messages in place add two new members to `GameState`

    std::size_t m_score;
    std::size_t m_lives;

Initialise these to 0 and 3 respectively. Now in `handleMessage()` these can be updated 
appropriately: if we get a message saying the `Ball` despawned, reduce the player's 
lives by one. Only respawn the `Ball` if the lives are not at 0. If we get a message 
saying a `Block` was broken increase the score. For the example I've incremented the 
score by 100 each time, but feel free to experiment with the scoring system. For example 
the amount of points awarded could increase with each block broken before the `Ball` 
hits the `Paddle` again.

Updating the score display from the message handler is simple, using the `CommandSystem` 
we added to `m_uiScene`.

    xy::Command cmd;
    cmd.targetFlags = CommandID::Score;
    cmd.action = [&](xy::Entity e, float)
    {
        e.getComponent<xy::Text>().setString("Score: " + std::to_string(m_score));
    };
    m_uiScene.getSystem<xy::CommandSystem>().sendCommand(cmd);

Use the same technique to update the `Lives` display. Remember to send the command via 
`m_uiSystem`, not `m_gameSystem`!

---

### Summary
From here develop your own game rules. Track the lives value and when it reaches zero, 
display a game over message. Track the count of active blocks and when it reaches zero 
spawn a new set. Try creating blocks in a different pattern.

For an even more fully fleshed out experience refer to part two of the tutorial where we 
created a new `State`, and use it to create a `PauseState`. Add a menu to the `PauseState`, 
using part one of the tutorial as a guide, so that the player can quit or restart the 
game. Remember you only have to push the `PauseState` on to the `StateStack` to pause the 
game and pop it again to resume play.

Maybe even try spawning special entities when random `Block`s break, and award bonuses 
if the player catches them with the `Paddle`.

If you made it this far, thank you for staying with me! In the last part we'll cover the 
`Director` class and use it to create special effects with sounds and particles.