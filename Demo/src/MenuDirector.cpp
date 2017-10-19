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

#include "MenuDirector.hpp"

#include <xyginext/ecs/Entity.hpp>
#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/Callback.hpp>
#include <xyginext/ecs/components/SpriteAnimation.hpp>
#include <xyginext/ecs/components/Drawable.hpp>
#include <xyginext/ecs/components/QuadTreeItem.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>
#include <xyginext/resources/Resource.hpp>

#include <xyginext/util/Random.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    class Runner final
    {
    private:
        float m_direction;
        xy::Scene& m_scene;
        static constexpr float Speed = 400.f;

    public:
        Runner(float direction, xy::Scene& scene)
            : m_direction(direction),
            m_scene(scene)
        {

        }

        void operator()(xy::Entity entity, float dt)
        {
            auto& tx = entity.getComponent<xy::Transform>();
            tx.move(m_direction * Speed * dt, 0.f);

            if ((m_direction > 0 && tx.getPosition().x > xy::DefaultSceneSize.x + 128)
                || (m_direction < 0 && tx.getPosition().x < -128)) //magic number is sprite width
            {
                m_scene.destroyEntity(entity);
            }
        }
    };

    class BubbleBehaviour final
    {
    private:
        static constexpr float LifeTime = 2.f;
    public:
        explicit BubbleBehaviour(xy::Scene& scene) : m_scene(scene), m_lifetime(LifeTime) {}

        void operator()(xy::Entity entity, float dt)
        {
            m_lifetime -= dt;
            float ratio = std::max(0.f, m_lifetime / LifeTime);
            float scale = 1.f - ratio;

            entity.getComponent<xy::Transform>().setScale(scale, scale);
            entity.getComponent<xy::Transform>().move(0.f, -300.f * dt * ratio);

            if (m_lifetime < 0)
            {
                m_scene.destroyEntity(entity);
            }
        }

    private:
        xy::Scene& m_scene;
        float m_lifetime;
    };

    class BubbleSpawner final
    {
    private:
        static constexpr float spawnTime = 0.6f;

    public:
        BubbleSpawner(const xy::Sprite& sprite, xy::Scene& scene)
            : m_sprite(sprite),
            m_scene(scene),
            m_timer(0.f){}

        void operator()(xy::Entity entity, float dt)
        {
            m_timer += dt;
            if (m_timer > spawnTime)
            {
                m_timer = 0.f;
                auto bubbleEnt = m_scene.createEntity();
                bubbleEnt.addComponent<xy::Sprite>() = m_sprite;
                bubbleEnt.addComponent<xy::Drawable>();
                bubbleEnt.addComponent<xy::SpriteAnimation>().play(0);
                bubbleEnt.addComponent<xy::Transform>().setPosition(entity.getComponent<xy::Transform>().getWorldPosition());
                bubbleEnt.getComponent<xy::Transform>().setScale(0.f, 0.f);
                bubbleEnt.addComponent<xy::Callback>().active = true;
                bubbleEnt.getComponent<xy::Callback>().function = BubbleBehaviour(m_scene);
            }
        }

    private:
        const xy::Sprite& m_sprite;
        xy::Scene& m_scene;
        float m_timer;
    };
}

MenuDirector::MenuDirector(xy::TextureResource& tr)
    : m_currentAct  (0),
    m_timer         (5.f),
    m_acts          (19)
{
    m_particleSettings.loadFromFile("assets/particles/panic.xyp", tr);
    
    xy::SpriteSheet spriteSheet;
    spriteSheet.loadFromFile("assets/sprites/menu_sprites.spt", tr);

    m_sprites[MenuSprite::Balldock].sprite = spriteSheet.getSprite("balldock");
    m_sprites[MenuSprite::Clocksy].sprite = spriteSheet.getSprite("clocksy");
    m_sprites[MenuSprite::Goobly].sprite = spriteSheet.getSprite("goobly");
    m_sprites[MenuSprite::PlayerOne].sprite = spriteSheet.getSprite("player_one");
    m_sprites[MenuSprite::PlayerTwo].sprite = spriteSheet.getSprite("player_two");
    m_sprites[MenuSprite::Princess].sprite = spriteSheet.getSprite("princess");
    m_sprites[MenuSprite::Whirlybob].sprite = spriteSheet.getSprite("whirlybob");
    m_sprites[MenuSprite::Bubble].sprite = spriteSheet.getSprite("bubble");

    m_sprites[MenuSprite::Goobly].verticalOffset = 128.f;
    m_sprites[MenuSprite::Whirlybob].verticalOffset = 192.f;


    //set up actor spawn times
    m_acts[0].sprite = MenuSprite::Balldock;
    m_acts[0].nextTime = 10.f; //time to next spawn

    m_acts[1].sprite = MenuSprite::Clocksy;
    m_acts[1].nextTime = 0.5f;
    
    m_acts[2].sprite = MenuSprite::Clocksy;
    m_acts[2].nextTime = 0.2f;

    m_acts[3].sprite = MenuSprite::Whirlybob;
    m_acts[3].nextTime = 0.6f;

    m_acts[4].sprite = MenuSprite::PlayerOne;
    m_acts[4].nextTime = 1.6f;

    m_acts[5].sprite = MenuSprite::PlayerTwo;
    m_acts[5].nextTime = 0.1f;


    //-----
    m_acts[6].sprite = MenuSprite::PlayerTwo;
    m_acts[6].nextTime = 10.f;
    m_acts[6].direction = -1.f;

    m_acts[7].sprite = MenuSprite::PlayerOne;
    m_acts[7].nextTime = 0.16f;
    m_acts[7].direction = -1.f;

    m_acts[8].sprite = MenuSprite::Goobly;
    m_acts[8].nextTime = 0.9f;
    m_acts[8].direction = -1.f;

    m_acts[9].sprite = MenuSprite::Clocksy;
    m_acts[9].nextTime = 0.8f;
    m_acts[9].direction = -1.f;

    m_acts[10].sprite = MenuSprite::Whirlybob;
    m_acts[10].nextTime = 0.4f;
    m_acts[10].direction = -1.f;

    m_acts[11].sprite = MenuSprite::Balldock;
    m_acts[11].nextTime = 0.05f;
    m_acts[11].direction = -1.f;

    m_acts[12].sprite = MenuSprite::Clocksy;
    m_acts[12].nextTime = 0.6f;
    m_acts[12].direction = -1.f;

    //-----
    m_acts[13].sprite = MenuSprite::Clocksy;
    m_acts[13].nextTime = 10.f;

    m_acts[14].sprite = MenuSprite::Whirlybob;
    m_acts[14].nextTime = 0.1f;

    m_acts[15].sprite = MenuSprite::Clocksy;
    m_acts[15].nextTime = 0.4f;

    m_acts[16].sprite = MenuSprite::Balldock;
    m_acts[16].nextTime = 0.4f;

    m_acts[17].sprite = MenuSprite::Goobly;
    m_acts[17].nextTime = 0.9f;

    m_acts[18].sprite = MenuSprite::Princess;
    m_acts[18].nextTime = 1.6f;
}

//public
void MenuDirector::handleEvent(const sf::Event&)
{

}

void MenuDirector::process(float dt)
{
    m_timer += dt;
    if (m_timer > m_acts[m_currentAct].nextTime)
    {
        m_timer = 0;
        spawnSprite(m_acts[m_currentAct]);
        m_currentAct = (m_currentAct + 1) % m_acts.size();
    }
}

//private
void MenuDirector::spawnSprite(const Act& act)
{
    auto& scene = getScene();

    auto entity = scene.createEntity();
    entity.addComponent<xy::Sprite>() = m_sprites[act.sprite].sprite;
    entity.addComponent<xy::Drawable>();
    entity.addComponent<xy::SpriteAnimation>().play(0);

    auto bounds = m_sprites[act.sprite].sprite.getTextureBounds();
    entity.addComponent<xy::Transform>().setPosition(act.direction > 0 ? -bounds.width : xy::DefaultSceneSize.x,
        (xy::DefaultSceneSize.y - bounds.height) - m_sprites[act.sprite].verticalOffset);
    entity.getComponent<xy::Transform>().setScale(-act.direction, 1.f);
    //entity.getComponent<xy::Transform>().setOrigin(bounds.width / 2.f, 0.f);

    entity.addComponent<xy::QuadTreeItem>().setArea(bounds);

    entity.addComponent<xy::Callback>().active = true;
    entity.getComponent<xy::Callback>().function = Runner(act.direction, scene);

    if ((act.sprite == MenuSprite::PlayerOne || act.sprite == MenuSprite::PlayerTwo) && act.direction < 0)
    {
        /*auto spawnEnt = scene.createEntity();
        entity.getComponent<xy::Transform>().addChild(spawnEnt.addComponent<xy::Transform>());
        spawnEnt.addComponent<xy::Callback>().active = true;
        spawnEnt.getComponent<xy::Callback>().function = BubbleSpawner(m_sprites[MenuSprite::Bubble].sprite, scene);*/
        entity.addComponent<xy::ParticleEmitter>().settings = m_particleSettings;
        entity.getComponent<xy::ParticleEmitter>().start();
    }
    else if (act.sprite == MenuSprite::Goobly && act.direction > 0)
    {
        entity.addComponent<xy::ParticleEmitter>().settings = m_particleSettings;
        entity.getComponent<xy::ParticleEmitter>().start();
    }
    else if (act.sprite == MenuSprite::Whirlybob)
    {
        entity.getComponent<xy::Drawable>().setDepth(-2);
    }
}