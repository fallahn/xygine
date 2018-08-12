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

#include "MenuState.hpp"
#include "CommandIDs.hpp"
#include "KeyMapping.hpp"
#include "MessageIDs.hpp"
#include "KeyMapDirector.hpp"
#include "Localisation.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/UIHitBox.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/CommandTarget.hpp>

#include <xyginext/ecs/systems/UISystem.hpp>

void MenuState::createFirstMenu(xy::Transform& parentTx, sf::Uint32 selectedID, sf::Uint32 unselectedID, sf::Font& font)
{
    float xPosition = (xy::DefaultSceneSize.x / 2.f);
    
    //one player text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::SinglePlayer]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(128.f, 45.f);

    //one player button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -64.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 1;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 1;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 1;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //two player text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Multiplayer]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(108.f, -19.f);

    //multiplayer button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 64.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, -64.f);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //quit text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Quit]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(142.f, 15.f);

    //quit button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx4);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 192.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            requestStackClear();
            xy::App::quit();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            requestStackClear();
            xy::App::quit();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            requestStackClear();
            xy::App::quit();
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());
}

void MenuState::createSecondMenu(xy::Transform& parentTx, sf::Uint32 selectedID, sf::Uint32 unselectedID, sf::Font& font)
{
    float xPosition = (xy::DefaultSceneSize.x / 2.f) + xy::DefaultSceneSize.x;
    
    //local text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Local]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(52.f, 45.f);

    //local button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -64.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 2;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 2;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            m_sharedStateData.playerCount = 2;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //network text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Network]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(84.f, -19.f);

    //network button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 64.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, -64.f);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
            m_sharedStateData.playerCount = 1;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
            m_sharedStateData.playerCount = 1;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_menuTarget.x -= xy::DefaultSceneSize.x;
            m_sharedStateData.playerCount = 1;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //back text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Back]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(144.f, 15.f);

    //back button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx4);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 192.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());
}

void MenuState::createThirdMenu(xy::Transform& parentTx, sf::Uint32 selectedID, sf::Uint32 unselectedID, sf::Font& font)
{
    float xPosition = (xy::DefaultSceneSize.x / 2.f) + (xy::DefaultSceneSize.x * 2.f);

    //host text
    auto entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Host]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(44.f, 45.f);

    //host button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, -128.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_sharedStateData.hostState = SharedStateData::Host;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //join text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Join]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(46.f, 45.f);

    //ip text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(m_sharedStateData.remoteIP);
    entity.getComponent<xy::Text>().setCharacterSize(58);
    entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
    entity.addComponent<xy::Drawable>().setDepth(2);
    bounds.width -= 72.f;
    bounds.left -= 148.f;
    entity.getComponent<xy::Drawable>().setCroppingArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MenuText;
    auto& tx3 = entity.addComponent<xy::Transform>();
    tx3.setPosition(192.f, 146.f);

    //join button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 2.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx2);
    entity.getComponent<xy::Transform>().addChild(tx3);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 64.f);
    tx2.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    tx2.move(0.f, -64.f);
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_sharedStateData.hostState = SharedStateData::Client;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_sharedStateData.hostState = SharedStateData::Client;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_sharedStateData.hostState = SharedStateData::Client;
            requestStackClear();
            requestStackPush(StateID::Game);
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());

    //back text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(Locale::Strings[Locale::Back]);
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    entity.addComponent<xy::Drawable>().setDepth(2);
    bounds = entity.getComponent<xy::Drawable>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(144.f, 15.f);

    //back button
    entity = m_scene.createEntity();
    entity.addComponent<xy::Sprite>().setTexture(m_textureResource.get("assets/images/button.png"));
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds();
    entity.getComponent<xy::Sprite>().setTextureRect({ 0.f, 0.f, bounds.width, bounds.height / 4.f });
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::Transform>().setOrigin(entity.getComponent<xy::Sprite>().getSize() / 2.f);
    entity.getComponent<xy::Transform>().addChild(tx4);
    entity.getComponent<xy::Transform>().setPosition(xPosition, xy::DefaultSceneSize.y / 2.f);
    entity.getComponent<xy::Transform>().move(0.f, 256.f);
    tx.setPosition(entity.getComponent<xy::Transform>().getOrigin());
    bounds = entity.getComponent<xy::Sprite>().getTextureBounds(); //these have been updated by setTextureRect
    entity.addComponent<xy::UIHitBox>().area = bounds;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::MouseUp] =
        m_scene.getSystem<xy::UISystem>().addMouseButtonCallback([this](xy::Entity, sf::Uint64 flags)
    {
        if (flags & xy::UISystem::LeftMouse)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::KeyUp] =
        m_scene.getSystem<xy::UISystem>().addKeyCallback([this](xy::Entity, sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space || key == sf::Keyboard::Return)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::ControllerButtonUp] =
        m_scene.getSystem<xy::UISystem>().addControllerCallback([this](xy::Entity, sf::Uint32, sf::Uint32 button)
    {
        if (button == 0)
        {
            m_menuTarget.x += xy::DefaultSceneSize.x;
        }
    });
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Selected] = selectedID;
    entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::CallbackID::Unselected] = unselectedID;
    parentTx.addChild(entity.getComponent<xy::Transform>());
}

void MenuState::createKeybindInputs(xy::Entity towerEnt, sf::Uint8 player)
{
    static const std::array<sf::Vector2f, 4u> buttonPositions =
    {{
        sf::Vector2f(64.f, 192.f),
        sf::Vector2f(64.f, 448.f),
        sf::Vector2f(64.f, 704.f),
        sf::Vector2f(64.f, 960.f)
    }};

    static const std::array<sf::Vector2f, 4u> textPositions =
    {{
        sf::Vector2f(160.f, 194.f),
        sf::Vector2f(160.f, 450.f),
        sf::Vector2f(160.f, 706.f),
        sf::Vector2f(160.f, 962.f)
    }};

    auto& towerTx = towerEnt.getComponent<xy::Transform>();
    auto& font = m_fontResource.get("assets/fonts/Cave-Story.ttf");

    for (auto i = 0u; i < 4u; ++i)
    {
        auto entity = m_helpScene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(buttonPositions[i]);
        towerTx.addChild(entity.getComponent<xy::Transform>());
        entity.addComponent<xy::UIHitBox>().area = { 0.f, 0.f, 192.f, 64.f };
        entity.getComponent<xy::UIHitBox>().callbacks[xy::UIHitBox::MouseUp] =
            m_helpScene.getSystem<xy::UISystem>().addMouseButtonCallback([&, i, player](
                xy::Entity buttonEnt, sf::Uint64 flags)
        {
            auto* msg = getContext().appInstance.getMessageBus().post<MenuEvent>(MessageID::MenuMessage);
            msg->action = MenuEvent::KeybindClicked;
            msg->index = i;
            msg->player = player;
        });

        entity = m_helpScene.createEntity();
        entity.addComponent<xy::Transform>().setPosition(textPositions[i]);
        towerTx.addChild(entity.getComponent<xy::Transform>());
        entity.addComponent<xy::Text>(font).setCharacterSize(36);
        entity.getComponent<xy::Text>().setAlignment(xy::Text::Alignment::Centre);
        
        switch (i)
        {
        default: break;
        case 0:
            entity.getComponent<xy::Text>().setString(KeyMap.at(m_sharedStateData.inputBindings[player].keys[InputBinding::Jump]));
            break;
        case 1:
            entity.getComponent<xy::Text>().setString(KeyMap.at(m_sharedStateData.inputBindings[player].keys[InputBinding::Shoot]));
            break;
        case 2:
            entity.getComponent<xy::Text>().setString(KeyMap.at(m_sharedStateData.inputBindings[player].keys[InputBinding::Left]));
            break;
        case 3:
            entity.getComponent<xy::Text>().setString(KeyMap.at(m_sharedStateData.inputBindings[player].keys[InputBinding::Right]));
            break;
        }
        entity.addComponent<xy::Drawable>().setDepth(2);
        entity.addComponent<xy::CommandTarget>().ID = CommandID::KeybindInput;
        entity.addComponent<KeyMapInput>().player = player;
        entity.getComponent<KeyMapInput>().index = i;
    }
}
