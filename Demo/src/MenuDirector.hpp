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

#ifndef DEMO_MENU_DIRECTOR_HPP_
#define DEMO_MENU_DIRECTOR_HPP_

#include <xyginext/ecs/Director.hpp>
#include <xyginext/ecs/components/Sprite.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>

#include <vector>

namespace xy
{
    class TextureResource;
}

class MenuDirector final : public xy::Director
{
public:
    explicit MenuDirector(xy::TextureResource&);

    void handleMessage(const xy::Message&) override {}

    void handleEvent(const sf::Event&) override;

    void process(float) override;


private:

    xy::EmitterSettings m_panicParticleSettings;
    xy::EmitterSettings m_leafParticleSettings;

    enum MenuSprite
    {
        PlayerOne,
        PlayerTwo,
        Clocksy,
        Goobly,
        Whirlybob,
        Balldock,
        Princess,
        Bubble,
        Count
    };

    struct Spritem final
    {
        xy::Sprite sprite;
        float verticalOffset = 0.f;
    };

    std::array<Spritem, MenuSprite::Count> m_sprites;

    struct Act final
    {
        float direction = 1.f;
        MenuSprite sprite = MenuSprite::PlayerOne;
        float nextTime = 1.f;
    };

    std::size_t m_currentAct;
    float m_timer;
    std::vector<Act> m_acts;

    void spawnSprite(const Act&);
};

#endif //DEMO_MENU_DIRECTOR_HPP_