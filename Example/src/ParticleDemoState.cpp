/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <ParticleDemoState.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>
#include <xygine/Util.hpp>

#include <xygine/ParticleController.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
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

    enum ParticleType
    {
        Bubbles = 0,
        Explosion,
        FairyDust,
        Fire,
        Count
    };

    sf::Uint64 controllerId = 0;
}

ParticleDemoState::ParticleDemoState(xy::StateStack& stateStack, Context context)
    : State         (stateStack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus)
{
    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);
    m_scene.setPostEffects(xy::Scene::PostEffect::ChromaticAbberation);

    m_reportText.setFont(context.appInstance.getFont("assets/fonts/Console.ttf"));
    m_reportText.setPosition(1500.f, 30.f);

    setupParticles();
    context.renderWindow.setMouseCursorVisible(true);
}

bool ParticleDemoState::update(float dt)
{    
    m_scene.update(dt);

    m_reportText.setString(xy::StatsReporter::reporter.getString());

    return true;
}

void ParticleDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    rw.draw(m_reportText);
}

bool ParticleDemoState::handleEvent(const sf::Event& evt)
{
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        const auto& rw = getContext().renderWindow;
        auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));
        xy::Command cmd;
        cmd.entityID = controllerId;
        cmd.action = [mousePos](xy::Entity& entity, float)
        {
            entity.getComponent<xy::ParticleController>()->fire(xy::Util::Random::value(0, ParticleType::Count - 1), mousePos);
        };
        m_scene.sendCommand(cmd);
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

void ParticleDemoState::handleMessage(const xy::Message& msg)
{ 
    m_scene.handleMessage(msg);
}

//private
void ParticleDemoState::setupParticles()
{
    auto particleController = std::make_unique<xy::ParticleController>(m_messageBus);
    auto entity = std::make_unique<xy::Entity>(m_messageBus);

    auto pc = entity->addComponent<xy::ParticleController>(particleController);
    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/bubbles.xyp", getContext().appInstance);
    pc->addDefinition(ParticleType::Bubbles, pd);

    pd.loadFromFile("assets/particles/explosion.xyp", getContext().appInstance);
    pc->addDefinition(ParticleType::Explosion, pd);

    pd.loadFromFile("assets/particles/fairydust.xyp", getContext().appInstance);
    pc->addDefinition(ParticleType::FairyDust, pd);

    pd.loadFromFile("assets/particles/fire.xyp", getContext().appInstance);
    pc->addDefinition(ParticleType::Fire, pd);

    controllerId = entity->getUID();
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}