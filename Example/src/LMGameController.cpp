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
#include <LMCollisionWorld.hpp>
#include <LMHumanController.hpp>
#include <LMAlienController.hpp>
#include <LMBulletController.hpp>
#include <CommandIds.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/ParticleController.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f playerSize(32.f, 42.f);
    const sf::Uint8 humanCount = 8;
    const sf::Vector2f bulletSize(6.f, 10.f);
}

GameController::GameController(xy::MessageBus& mb, xy::Scene& scene, CollisionWorld& cw)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_collisionWorld(cw),
    m_inputFlags    (0),
    m_spawnReady    (true),
    m_player        (nullptr),
    m_mothership    (nullptr)
{
    createTerrain();
    createAliens();
    createMothership();
    spawnHumans();

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
        case LMEvent::PlayerLanded:
        {
            auto pos = m_player->getPosition();

            //get nearest human
            auto dist = xy::Util::Vector::lengthSquared(m_humans[0]->getPosition() - pos);
            std::size_t index = 0;
            if (m_humans.size() > 1)
            {
                for (auto i = 1u; i < m_humans.size(); ++i)
                {
                    auto newDist = xy::Util::Vector::lengthSquared(m_humans[i]->getPosition() - pos);
                    if (newDist < dist)
                    {
                        index = i;
                        dist = newDist;
                    }
                }
            }
            m_humans[index]->getComponent<HumanController>()->setDestination(pos);
        }
            break;
        case LMEvent::HumanRescued:
            addRescuedHuman();
            break;
        case LMEvent::AlienDied:
            spawnAlien();
            break;
        }
    };
    addMessageHandler(handler);

    //particle spawner
    auto particleManager = xy::Component::create<xy::ParticleController>(getMessageBus());
    handler.action = [](xy::Component* c, const xy::Message& msg)
    {
        auto component = dynamic_cast<xy::ParticleController*>(c);
        auto& msgData = msg.getData<LMEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMEvent::AlienDied:
        case LMEvent::PlayerDied:
            component->fire(LMParticleID::SmallExplosion, { msgData.posX, msgData.posY });
            break;
        }
    };
    particleManager->addMessageHandler(handler);

    auto entity = xy::Entity::create(getMessageBus());
    auto pc = entity->addComponent(particleManager);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/LunarMooner/small_explosion.xyp", m_textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);

    m_playerParticles[LMParticleID::Thruster].loadFromFile("assets/particles/LunarMooner/thrust.xyp", m_textureResource);
    m_playerParticles[LMParticleID::RcsLeft].loadFromFile("assets/particles/LunarMooner/rcs_left.xyp", m_textureResource);
    m_playerParticles[LMParticleID::RcsRight].loadFromFile("assets/particles/LunarMooner/rcs_right.xyp", m_textureResource);
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

    //remove humans which have reached the ship
    m_humans.erase(std::remove_if(m_humans.begin(), m_humans.end(),
        [](const xy::Entity* ent)
    {
        return ent->destroyed();
    }), m_humans.end());
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

        //hook player shoot event here
        //as bullets are technically entities in their own right
        if ((input & LMInputFlags::Shoot) != 0
            && (m_inputFlags & LMInputFlags::Shoot) == 0)
        {
            spawnBullet();
        }
    }

    m_inputFlags = input;
}

//private
void GameController::spawnPlayer()
{
    if (!m_player && m_spawnReady)
    {
        const auto& children = m_mothership->getChildren();
        auto spawnPos = children.back()->getWorldPosition();
        children.back()->destroy();

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);

        auto playerController = xy::Component::create<lm::PlayerController>(getMessageBus());

        auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, playerSize }, CollisionComponent::ID::Player);
        CollisionComponent::Callback cb = std::bind(&PlayerController::collisionCallback, playerController.get(), _1);
        collision->setCallback(cb);

        auto thrust = m_playerParticles[LMParticleID::Thruster].createSystem(getMessageBus());
        thrust->setName("thrust");

        auto rcsLeft = m_playerParticles[LMParticleID::RcsLeft].createSystem(getMessageBus());
        rcsLeft->setName("rcsLeft");

        auto rcsRight = m_playerParticles[LMParticleID::RcsRight].createSystem(getMessageBus());
        rcsRight->setName("rcsRight");

        auto entity = xy::Entity::create(getMessageBus());
        entity->setPosition(spawnPos);
        entity->setOrigin(playerSize / 2.f);
        entity->addComponent(dropshipDrawable);
        m_player = entity->addComponent(playerController);
        entity->addComponent(collision);
        entity->addComponent(thrust);
        entity->addComponent(rcsLeft);
        entity->addComponent(rcsRight);

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

    auto bounds = drawable->getDrawable().getGlobalBounds();
    auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, {bounds.width, bounds.height} }, CollisionComponent::ID::Mothership);

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->setPosition(386.f, 26.f);
    entity->addCommandCategories(LMCommandID::Mothership);

    m_mothership = m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);

    auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
    dropshipDrawable->getDrawable().setSize(playerSize);
    xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

    entity = xy::Entity::create(getMessageBus());    
    entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
    entity->addComponent(dropshipDrawable);
    m_mothership->addChild(entity);
}

namespace
{
    std::unique_ptr<xy::SfDrawableComponent<sf::CircleShape>> getHumanDrawable(xy::MessageBus& mb)
    {
        auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(mb);
        drawable->getDrawable().setRadius(10.f);
        drawable->getDrawable().setFillColor(sf::Color::Blue);
        drawable->getDrawable().setScale(1.f, 1.4f);
        return std::move(drawable);
    }

    const sf::Uint8 alienCount = 12;
    const sf::FloatRect alienArea(386.f, 200.f, 1148.f, 480.f);
    const std::array<sf::FloatRect, 4u> alienSizes =
    {
        sf::FloatRect(0.f, 0.f, 20.f, 16.f),
        { 0.f, 0.f, 28.f, 30.f },
        { 0.f, 0.f, 14.f, 16.f },
        { 0.f, 0.f, 18.f, 26.f }
    };
}

void GameController::spawnHumans()
{
    for (auto i = 0; i < humanCount; ++i)
    {
        auto drawable = getHumanDrawable(getMessageBus());

        auto controller = xy::Component::create<HumanController>(getMessageBus());

        auto entity = xy::Entity::create(getMessageBus());
        entity->addComponent(drawable);
        entity->addComponent(controller);
        entity->setPosition(xy::Util::Random::value(380.f, 1500.f), xy::Util::Random::value(1045.f, 1060.f));

        m_humans.push_back(m_scene.addEntity(entity, xy::Scene::Layer::FrontRear));
    }
}

void GameController::spawnAlien()
{
    auto size = alienSizes[xy::Util::Random::value(0, alienSizes.size() - 1)];
    auto position = sf::Vector2f(xy::Util::Random::value(alienArea.left, alienArea.left + alienArea.width),
        xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height));

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color::Red);
    drawable->getDrawable().setSize({ size.width, size.height });

    auto controller = xy::Component::create<AlienController>(getMessageBus(), alienArea);

    auto collision = m_collisionWorld.addComponent(getMessageBus(), size, lm::CollisionComponent::ID::Alien);
    lm::CollisionComponent::Callback cb = std::bind(&AlienController::collisionCallback, controller.get(), std::placeholders::_1);
    collision->setCallback(cb);

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->setPosition(position);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
}

void GameController::createAliens()
{
    for (auto i = 0; i < alienCount; ++i)
    {
        spawnAlien();
    }
}

void GameController::createTerrain()
{
    auto backgroundDrawable = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(getMessageBus());
    backgroundDrawable->getDrawable().setTexture(m_textureResource.get("assets/images/lunarmooner/background.png"));

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(alienArea.left, 0.f);
    entity->addComponent(backgroundDrawable);

    m_scene.addEntity(entity, xy::Scene::BackRear);


    //death zone at bottom
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color::Red);
    drawable->getDrawable().setSize({ alienArea.width, 40.f });

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ alienArea.width, 40.f } }, lm::CollisionComponent::ID::Alien);

    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(collision);
    entity->setPosition(alienArea.left, 1040.f);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);


    //walls
    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ 40.f, 1080.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left - 40.f, 0.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ 40.f, 1080.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left + alienArea.width, 0.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ alienArea.width, 40.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left, -40.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);


    //towers to land on
    std::array<std::pair<sf::Vector2f, sf::Vector2f>, 3u> positions =
    {
        std::make_pair(sf::Vector2f(180.f, 210.f), sf::Vector2f(20.f, 1080.f - 210.f)),
        std::make_pair(sf::Vector2f(150.f, 290.f), sf::Vector2f(520.f, 1080.f - 290.f)),
        std::make_pair(sf::Vector2f(220.f, 60.f), sf::Vector2f(900.f, 1080.f - 60.f))
    };

    for (const auto& p : positions)
    {
        drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        drawable->getDrawable().setFillColor(sf::Color::Green);
        drawable->getDrawable().setSize(p.first);

        collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f }, p.first }, lm::CollisionComponent::ID::Tower);

        entity = xy::Entity::create(getMessageBus());
        entity->addComponent(drawable);
        entity->addComponent(collision);
        entity->setPosition(alienArea.left + p.second.x, p.second.y);

        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    }
}

void GameController::addRescuedHuman()
{
    auto drawable = getHumanDrawable(getMessageBus());

    float offset = static_cast<float>(humanCount - (m_humans.size() + 1)) * 26.f;
    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(offset+4.f, 16.f);
    entity->addComponent(drawable);
    m_mothership->addChild(entity);
}

void GameController::spawnBullet()
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize(bulletSize);
    drawable->getDrawable().setFillColor(sf::Color::Cyan);

    auto controller = xy::Component::create<BulletController>(getMessageBus());

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, bulletSize }, CollisionComponent::ID::Bullet);
    CollisionComponent::Callback cb = std::bind(&BulletController::collisionCallback, controller.get(), _1);
    collision->setCallback(cb);

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(m_player->getPosition());
    entity->setOrigin(bulletSize / 2.f);
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
}