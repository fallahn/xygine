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

#include "ParticleDirector.hpp"
#include "MessageIDs.hpp"

#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>

namespace
{
    const std::size_t MinEmitters = 4;
}

ParticleDirector::ParticleDirector()
    :m_nextFreeEmitter(0),
    m_emitters(MinEmitters)
{
    //TODO load particle presets
}

//public
void ParticleDirector::handleMessage(const xy::Message& msg)
{
    //TODO fire a particle system based on event
}

void ParticleDirector::process(float dt)
{
    //check for finished systems then free up by swapping
    for (auto i = 0u; i < m_nextFreeEmitter; ++i)
    {
        if (m_emitters[i].getComponent<xy::ParticleEmitter>().stopped())
        {
            auto entity = m_emitters[i];
            m_nextFreeEmitter--;
            m_emitters[i] = m_emitters[m_nextFreeEmitter];
            m_emitters[m_nextFreeEmitter] = entity;
            i--;
        }
    }
}