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

#pragma once

#include <xyginext/core/State.hpp>
#include <xyginext/core/ConfigFile.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/resources/Resource.hpp>
#include <xyginext/resources/ResourceHandler.hpp>

#include <SFML/Graphics/Font.hpp>

#include "StateIDs.hpp"
#include "SharedStateData.hpp"

namespace xy
{
    class PostBlur;
    class Transform;
}

namespace MenuID
{
    enum
    {
        Main, Player, Network, Keybind
    };
}

class LoadingScreen;

class MenuState final : public xy::State
{
public:
    MenuState(xy::StateStack&, xy::State::Context, SharedStateData&, LoadingScreen&);

    xy::StateID stateID() const override { return StateID::MainMenu; }

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

private:
    xy::Scene m_scene;
    xy::Scene m_helpScene;

    xy::AudioResource m_audioResource;
    xy::ResourceHandler m_resource;
    
    // Handles to resources we'll use
    xy::ResourceHandle  m_menuBGRes,
                        m_grassRes,
                        m_flowerRes,
                        m_helpSignRes,
                        m_caveStoryRes,
                        m_helpRes,
                        m_keyBindsRes,
                        m_buttonRes;

    SharedStateData& m_sharedStateData;
    LoadingScreen& m_loadingScreen;

    bool m_helpShown;
    xy::PostBlur* m_blurEffect;

    xy::ConfigFile m_keyBinds;

    sf::Vector2f m_menuTarget;
    sf::Vector2f m_leftMenuTarget;
    sf::Vector2f m_rightMenuTarget;
    sf::Vector2f m_helpTextTarget;

    void loadKeybinds();
    void createScene();
    void createMenu();
    void createHelp();
    void showHelpMenu();

    void createFirstMenu(xy::Transform&, std::uint32_t, std::uint32_t, sf::Font&);
    void createSecondMenu(xy::Transform&, std::uint32_t, std::uint32_t, sf::Font&);
    void createThirdMenu(xy::Transform&, std::uint32_t, std::uint32_t, sf::Font&);

    void createKeybindInputs(xy::Entity, std::uint8_t);

    void updateLoadingScreen(float, sf::RenderWindow&) override;
};
