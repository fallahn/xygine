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

#ifndef DEMO_ENDING_DIRECTOR_HPP_
#define DEMO_ENDING_DIRECTOR_HPP_

#include <xyginext/ecs/Director.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/graphics/SpriteSheet.hpp>

namespace xy
{
    class SoundResource;
    class TextureResource;
}

class EndingDirector final : public xy::Director
{
public:
    EndingDirector(xy::SoundResource&, xy::TextureResource&, xy::MessageBus&);

    void handleMessage(const xy::Message&) override;
    void handleEvent(const sf::Event&) override;
    void process(float) override;

private:
    xy::SoundResource& m_soundResource;
    xy::TextureResource& m_textureResource;
    xy::MessageBus& m_messageBus;
    xy::SpriteSheet m_spriteSheet;
    xy::Sprite m_playerSprite;

    void spawnBubble(sf::Uint32);
    void spawnFood(sf::Uint32);
    void spawnPause();
    void spawnWideShot();

    enum SoundID
    {
        Collect, Pop, Land,
        Q1, Q2, Q3, Shout, Angry
    };
    void playSound(sf::Uint32, xy::Entity);
};
#endif //DEMO_ENDING_DIRECOTR_HPP_