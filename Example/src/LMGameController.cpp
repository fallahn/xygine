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

#include <LMGameController.hpp>
#include <LMPlayerController.hpp>
#include <LMMothershipController.hpp>
#include <CommandIds.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/CircleShape.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f playerSize(32.f, 42.f);
}

GameController::GameController(xy::MessageBus& mb, xy::Scene& scene)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_inputFlags    (0),
    m_spawnReady    (true),
    m_player        (nullptr),
    m_mothership    (nullptr)
{
    createMothership();

    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::LMMessage;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMEvent::PlayerDied:
        {
            m_delayedEvents.emplace_back();
            auto& de = m_delayedEvents.back();
            de.time = 2.f;
            de.action = [this]()
            {
                m_spawnReady = true;

                auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
                dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
                dropshipDrawable->getDrawable().setSize(playerSize);
                xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

                auto entity = xy::Entity::create(getMessageBus());
                auto bounds = m_mothership->getComponent<xy::SfDrawableComponent<sf::CircleShape>>()->getDrawable().getGlobalBounds();
                entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
                entity->addComponent(dropshipDrawable);
                m_mothership->addChild(entity);
            };

            m_player = nullptr;
        }
            break;
        }
    };
    addMessageHandler(handler);
}

//public
void GameController::entityUpdate(xy::Entity&, float dt)
{
    for (auto& de : m_delayedEvents) de.time -= dt;
    
    //execute then remove expired events
    m_delayedEvents.erase(std::remove_if(m_delayedEvents.begin(), m_delayedEvents.end(),
        [](const DelayedEvent& de)
    {
        if (de.time <= 0)
        {
            de.action();
            return true;
        }
        return false;
    }),
        m_delayedEvents.end());
}

void GameController::setInput(sf::Uint8 input)
{
    if ((input & LMInputFlags::Start) == 0 
        && (m_inputFlags & LMInputFlags::Start) != 0)
    {
        //start was released, try spawning
        spawnPlayer();
    }

    if (m_player)
    {
        //set player input
        m_player->setInput(input);

        //TODO hook player shoot even here
        //as bullets are technically entities in their own right

        //TODO hook particle effects for thrusts here too
    }

    m_inputFlags = input;
}

//private
void GameController::spawnPlayer()
{
    if (!m_player && m_spawnReady)
    {
        const auto& children = m_mothership->getChildren();
        auto spawnPos = children[0]->getWorldPosition();
        children[0]->destroy();

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);
        xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

        auto playerController = xy::Component::create<lm::PlayerController>(getMessageBus());

        auto entity = xy::Entity::create(getMessageBus());
        entity->setPosition(spawnPos);
        entity->addComponent(dropshipDrawable);
        m_player = entity->addComponent(playerController);

        m_scene.addEntity(entity, xy::Scene::BackFront);

        m_spawnReady = false;
    }
}

void GameController::createMothership()
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(getMessageBus());
    drawable->getDrawable().setRadius(10.f);
    drawable->getDrawable().setScale(12.f, 4.f);
    drawable->getDrawable().setFillColor(sf::Color::Yellow);

    auto controller = xy::Component::create<lm::MothershipController>(getMessageBus(), sf::Vector2f(386.f, 1534.f));

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->setPosition(386.f, 26.f);
    entity->addCommandCategories(LMCommandID::Mothership);

    m_mothership = m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);

    auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
    dropshipDrawable->getDrawable().setSize(playerSize);
    xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

    entity = xy::Entity::create(getMessageBus());
    auto bounds = m_mothership->getComponent<xy::SfDrawableComponent<sf::CircleShape>>()->getDrawable().getGlobalBounds();
    entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
    entity->addComponent(dropshipDrawable);
    m_mothership->addChild(entity);
}
