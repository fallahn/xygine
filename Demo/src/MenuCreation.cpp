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
    entity.addComponent<xy::Text>(font).setString("Single Player");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
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
    entity.addComponent<xy::Text>(font).setString("Multiplayer");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
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
    entity.addComponent<xy::Text>(font).setString("QUIT");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(136.f, 15.f);

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
    entity.addComponent<xy::Text>(font).setString("Local");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
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
    entity.addComponent<xy::Text>(font).setString("Network");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
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
    entity.addComponent<xy::Text>(font).setString("Back");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(140.f, 15.f);

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
    entity.addComponent<xy::Text>(font).setString("Host");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx = entity.addComponent<xy::Transform>();
    tx.setOrigin(50.f, 45.f);

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
    entity.addComponent<xy::Text>(font).setString("Join");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    auto& tx2 = entity.addComponent<xy::Transform>();
    tx2.setOrigin(50.f, 45.f);

    //ip text
    entity = m_scene.createEntity();
    entity.addComponent<xy::Text>(font).setString(m_sharedStateData.remoteIP);
    entity.getComponent<xy::Text>().setCharacterSize(65);
    bounds.width -= 72.f;
    entity.getComponent<xy::Text>().setCroppingArea(bounds);
    entity.addComponent<xy::CommandTarget>().ID = CommandID::MenuText;
    auto& tx3 = entity.addComponent<xy::Transform>();
    tx3.setPosition(44.f, 146.f);

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
    entity.addComponent<xy::Text>(font).setString("Back");
    entity.getComponent<xy::Text>().setCharacterSize(60);
    entity.getComponent<xy::Text>().setFillColour(sf::Color::Black);
    bounds = entity.getComponent<xy::Text>().getLocalBounds();
    auto& tx4 = entity.addComponent<xy::Transform>();
    tx4.setPosition(140.f, 15.f);

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