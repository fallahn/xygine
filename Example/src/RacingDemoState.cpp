/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#include <RacingDemoState.hpp>
#include <RacingDemoTrack.hpp>
#include <RacingDemoPlayer.hpp>
#include <CommandIds.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/Util.hpp>

#include <xygine/PostBloom.hpp>
#include <xygine/PostChromeAb.hpp>

#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Keyboard::Key upKey = sf::Keyboard::W;
    const sf::Keyboard::Key downKey = sf::Keyboard::S;
    const sf::Keyboard::Key leftKey = sf::Keyboard::A;
    const sf::Keyboard::Key rightKey = sf::Keyboard::D;
    const sf::Keyboard::Key fireKey = sf::Keyboard::Space;

    const float joyDeadZone = 25.f;
    const float joyMaxAxis = 100.f;
}

RacingDemoState::RacingDemoState(xy::StateStack& stateStack, Context context)
    : State(stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene(m_messageBus)
{
    launchLoadingScreen();

    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);
    //xy::PostProcess::Ptr pp = xy::PostChromeAb::create();
    //m_scene.addPostProcess(pp);
    //pp = xy::PostBloom::create();
    //m_scene.addPostProcess(pp);
    //m_scene.setClearColour({ 0u, 100u, 220u });

    m_reportText.setFont(m_fontResource.get("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    buildScene();

    quitLoadingScreen();
}

bool RacingDemoState::update(float dt)
{    
    m_scene.update(dt);

    m_reportText.setString(xy::StatsReporter::reporter.getString());

    return true;
}

void RacingDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    rw.draw(m_reportText);
}

bool RacingDemoState::handleEvent(const sf::Event& evt)
{   
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        //const auto& rw = getContext().renderWindow;
        //auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    }
    break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case upKey:

            break;
        case downKey:

            break;
        case leftKey:

            break;
        case rightKey:

            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case sf::Keyboard::Escape:
        case sf::Keyboard::BackSpace:
            requestStackPop();
            requestStackPush(States::ID::MenuMain);
            break;
        case sf::Keyboard::P:
            //requestStackPush(States::ID::MenuPaused);
            break;
        case upKey:

            break;
        case downKey:

            break;
        case leftKey:

            break;
        case rightKey:

            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonPressed:
        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        }
        break;
    case sf::Event::JoystickButtonReleased:

        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        case 7:

            break;
        default: break;
        }
        break;
    }

    return true;
}

void RacingDemoState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void RacingDemoState::buildScene()
{
    auto trackEnt = xy::Entity::create(m_messageBus);
    trackEnt->addCommandCategories(RacingCommandIDs::TrackEnt);
    auto tc = std::make_unique<RaceDemo::Track>(m_messageBus);
    auto trackComponent = trackEnt->addComponent<RaceDemo::Track>(tc);
    m_scene.addEntity(trackEnt, xy::Scene::Layer::BackMiddle);

    auto trackTex = &m_textureResource.get("assets/images/racing demo/paper_tex.png");
    trackTex->setRepeated(true);
    trackComponent->setTexture(trackTex);

    auto playerEnt = xy::Entity::create(m_messageBus);
    playerEnt->addCommandCategories(RacingCommandIDs::PlayerEnt);
    auto drawable = std::make_unique<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    auto playerSprite = playerEnt->addComponent<xy::SfDrawableComponent<sf::RectangleShape>>(drawable);
    auto& rectangle = playerSprite->getDrawable();
    rectangle.setSize({ 640.f, 300.f });
    rectangle.setFillColor(sf::Color::Red);
    xy::Util::Position::centreOrigin(rectangle);
    playerEnt->setWorldPosition({ 960.f, 930.f });

    auto pc = std::make_unique<RaceDemo::PlayerController>(m_messageBus);
    auto playerController = playerEnt->addComponent<RaceDemo::PlayerController>(pc);
    playerController->setDepth(trackComponent->getCameraDepth());

    m_scene.addEntity(playerEnt, xy::Scene::Layer::FrontRear);
}