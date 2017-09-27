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

#ifndef DEMO_PARTICLE_DIRECTOR_HPP_
#define DEMO_APRTICLE_DIRECTOR_HPP_

#include <xyginext/ecs/Director.hpp>
#include <xyginext/ecs/Entity.hpp>

#include <xyginext/ecs/components/ParticleEmitter.hpp>

#include <vector>
#include <array>

namespace xy
{
    class TextureResource;
}

class ParticleDirector final : public xy::Director 
{
public:
    explicit ParticleDirector(xy::TextureResource&);

    void handleMessage(const xy::Message&) override;

    void handleEvent(const sf::Event&) override;

    void process(float) override;

private:

    enum SettingsID
    {
        BubblePop,
        SpawnNPC,
        Count
    };

    std::array<xy::EmitterSettings, SettingsID::Count> m_settings;

    std::size_t m_nextFreeEmitter;
    std::vector<xy::Entity> m_emitters;

    void resizeEmitters();
    xy::Entity getNextEntity();
};


#endif //DEMO_PARTICLE_DIRECTOR_HPP_