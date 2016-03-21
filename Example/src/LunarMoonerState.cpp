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

#include <LunarMoonerState.hpp>
#include <LMAlienController.hpp>
#include <LMGameController.hpp>
#include <CommandIds.hpp>

#include <xygine/App.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/Command.hpp>
#include <xygine/Assert.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Keyboard::Key keyStart = sf::Keyboard::Num1;
    const sf::Keyboard::Key keyLeft = sf::Keyboard::A;
    const sf::Keyboard::Key keyRight = sf::Keyboard::D;
    const sf::Keyboard::Key keyThrust = sf::Keyboard::W;
    const sf::Keyboard::Key keyFire = sf::Keyboard::Space;
}

LunarMoonerState::LunarMoonerState(xy::StateStack& stack, Context context)
    : xy::State         (stack, context),
    m_scene             (context.appInstance.getMessageBus()),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_inputFlags        (0),
    m_prevInputFlags    (0)
{
    launchLoadingScreen();
    
    m_scene.setView(context.defaultView);
    m_scene.drawDebug(true);

    createAliens();
    createTerrain();

    auto gameController = xy::Component::create<lm::GameController>(m_messageBus, m_scene);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    quitLoadingScreen();
}

//public
bool LunarMoonerState::handleEvent(const sf::Event& evt)
{
    switch(evt.type)
    {
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case keyStart:
            m_inputFlags &= ~LMInputFlags::Start;
            break;
        case keyLeft:
            m_inputFlags &= ~LMInputFlags::SteerLeft;
            break;
        case keyRight:
            m_inputFlags &= ~LMInputFlags::SteerRight;
            break;
        case keyThrust:
            m_inputFlags &= ~LMInputFlags::Thrust;
            break;
        case keyFire:
            m_inputFlags &= ~LMInputFlags::Shoot;
            break;
        default:break;
        }
        break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case keyStart:
            m_inputFlags |= LMInputFlags::Start;
            break;
        case keyLeft:
            m_inputFlags |= LMInputFlags::SteerLeft;
            break;
        case keyRight:
            m_inputFlags |= LMInputFlags::SteerRight;
            break;
        case keyThrust:
            m_inputFlags |= LMInputFlags::Thrust;
            break;
        case keyFire:
            m_inputFlags |= LMInputFlags::Shoot;
            break;
        default:break;
        }
        break;
    default: break;
    }
    return true;
}

void LunarMoonerState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

bool LunarMoonerState::update(float dt)
{
    if (m_inputFlags != m_prevInputFlags)
    {
        m_prevInputFlags = m_inputFlags;
        xy::Command cmd;
        cmd.category = LMCommandID::GameController;
        cmd.action = [this](xy::Entity& entity, float)
        {
            XY_ASSERT(entity.getComponent<lm::GameController>(), "");
            entity.getComponent<lm::GameController>()->setInput(m_inputFlags);
        };
        m_scene.sendCommand(cmd);
    }
    
    m_scene.update(dt);
    return true;
}

void LunarMoonerState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_scene);
}

//private
namespace
{
    const sf::Uint8 alientCount = 8;
    const sf::FloatRect alienArea(386.f, 200.f, 1148.f, 480.f);
    const std::array<sf::FloatRect, 4u> alienSizes = 
    {
        sf::FloatRect(0.f, 0.f, 20.f, 16.f),
        {0.f, 0.f, 24.f, 22.f},
        {0.f, 0.f, 14.f, 16.f},
        {0.f, 0.f, 18.f, 26.f}
    };
}

void LunarMoonerState::createAliens()
{
    for (auto i = 0; i < alientCount; ++i)
    {
        auto size = alienSizes[xy::Util::Random::value(0, alienSizes.size() - 1)];
        auto position = sf::Vector2f(xy::Util::Random::value(alienArea.left, alienArea.left + alienArea.width),
                                    xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height));

        auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
        drawable->getDrawable().setFillColor(sf::Color::Red);
        drawable->getDrawable().setSize({ size.width, size.height });

        auto controller = xy::Component::create<lm::AlienController>(m_messageBus, alienArea);

        auto entity = xy::Entity::create(m_messageBus);
        entity->addComponent(drawable);
        entity->addComponent(controller);
        entity->setPosition(position);

        m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
    }
}

void LunarMoonerState::createTerrain()
{
    auto backgroundDrawable = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(m_messageBus);
    backgroundDrawable->getDrawable().setTexture(m_textureResource.get("assets/images/lunarmooner/background.png"));

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(alienArea.left, 0.f);
    entity->addComponent(backgroundDrawable);

    m_scene.addEntity(entity, xy::Scene::BackRear);


    //death zone at bottom
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    drawable->getDrawable().setFillColor(sf::Color::Red);
    drawable->getDrawable().setSize({ alienArea.width, 40.f });

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(drawable);
    entity->setPosition(alienArea.left, 1040.f);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);


    //towers to land on
    std::array<std::pair<sf::Vector2f, sf::Vector2f>, 3u> positions = 
    {
        std::make_pair(sf::Vector2f(180.f, 210.f), sf::Vector2f(20.f, 1080.f - 210.f)),
        std::make_pair(sf::Vector2f(150.f, 290.f), sf::Vector2f(520.f, 1080.f - 290.f)),
        std::make_pair(sf::Vector2f(220.f, 60.f), sf::Vector2f(900.f, 1080.f - 60.f))
    };

    for (const auto& p : positions)
    {
        drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
        drawable->getDrawable().setFillColor(sf::Color::Green);
        drawable->getDrawable().setSize(p.first);

        entity = xy::Entity::create(m_messageBus);
        entity->addComponent(drawable);
        entity->setPosition(alienArea.left + p.second.x, p.second.y);

        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    }
}
