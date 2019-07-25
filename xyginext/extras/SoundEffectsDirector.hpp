/*********************************************************************
(c) Matt Marchant 2017 - 2019
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

#include <xyginext/ecs/Director.hpp>
#include <xyginext/ecs/Entity.hpp>
#include <xyginext/resources/ResourceHandler.hpp>

#include <SFML/System/Vector2.hpp>

#include <vector>

namespace xy
{
    class AudioEmitter;
}

/*
The sound effects director inherits the xy::Director interface and can
be used by adding it to a Scene with xy::Scene::addDirector<T>();

The director pools a series of entities with AudioEmitter components
attached and can be used to dispatch sound effects based on messages
recieved on the message bus. See SoundEffectsDirector.cpp for more
detailed examples.
*/

class SFXDirector final : public xy::Director
{
public:

    SFXDirector();

    void handleEvent(const sf::Event&) override {}
    void handleMessage(const xy::Message&) override;
    void process(float) override;

private:

    xy::ResourceHandler m_resources;

    std::vector<xy::Entity> m_entities;
    std::size_t m_nextFreeEntity;

    xy::Entity getNextFreeEntity();
    void resizeEntities(std::size_t);
    xy::AudioEmitter& playSound(std::int32_t, sf::Vector2f = {});
};
