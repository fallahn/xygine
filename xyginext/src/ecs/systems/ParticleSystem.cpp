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

#include <xyginext/ecs/systems/ParticleSystem.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/components/ParticleEmitter.hpp>
#include <xyginext/core/App.hpp>
#include <xyginext/util/Const.hpp>
#include <xyginext/util/Random.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "../../detail/GLCheck.hpp"

#include <limits>

#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE 34370
#define GL_POINT_SPRITE 34913
#endif

using namespace xy;

namespace
{
    const std::string VertexShader = R"(
        #version 120    
        
        varying mat2 v_rotation;
        
        void main()
        {
            vec4 rotScale = gl_TextureMatrix[0] * gl_MultiTexCoord0; //we've shanghaid the text coords to hold rotation and scale

            vec2 rot = vec2(sin(rotScale.x), cos(rotScale.x));
            v_rotation[0] = vec2(rot.y, -rot.x);
            v_rotation[1] = rot;

            gl_FrontColor = gl_Color;
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
            gl_PointSize = 15.0;//rotScale.y;
        })";

    const std::string FragmentShader = R"(
        #version 120

        uniform sampler2D u_texture;
        
        varying mat2 v_rotation;

        void main()
        {
            vec2 texCoord = v_rotation * (gl_PointCoord - vec2(0.5));
            gl_FragColor = gl_Color * texture2D(u_texture, texCoord + vec2(0.5));
        })";

    const std::size_t MaxParticleSystems = 64; //max VBOs, must be divisible by min count
    const std::size_t MinParticleSystems = 4; //min amount before resizing. This many are added on resize
}

ParticleSystem::ParticleSystem(xy::MessageBus& mb)
    : xy::System        (mb, typeid(ParticleSystem)),
    m_arrayCount        (0),
    m_activeArrayCount  (0)
{
    requireComponent<ParticleEmitter>();
    requireComponent<Transform>();

    if (!m_shader.loadFromMemory(VertexShader, FragmentShader))
    {
        Logger::log("Failed creating particle shader", Logger::Type::Error);
    }

    m_emitterArrays.reserve(MaxParticleSystems);
    m_emitterArrays.resize(MinParticleSystems);

    m_dummyTexture.create(1, 1);
}

ParticleSystem::~ParticleSystem()
{

}

//public
void ParticleSystem::process(float dt)
{
    m_activeArrayCount = 0;
    auto view = App::getRenderWindow().getView();
    sf::FloatRect viewableArea(view.getCenter() - (view.getSize() / 2.f), view.getSize());

    auto& entities = getEntities();
    for (auto& entity : entities)
    {
        auto& emitter = entity.getComponent<ParticleEmitter>();
        if (emitter.m_running &&
            emitter.m_emissionClock.getElapsedTime().asSeconds() > (1.f / emitter.settings.emitRate))
        {
            //time to emit a particle
            emitter.m_emissionClock.restart();
            static const float epsilon = 0.0001f;
            if (emitter.m_nextFreeParticle < emitter.m_particles.size() - 1)
            {
                auto& tx = entity.getComponent<Transform>();
                auto rotation = tx.getRotation() * Util::Const::degToRad;

                const auto& settings = emitter.settings;
                XY_ASSERT(settings.emitRate > 0, "Emit rate must be grater than 0");
                XY_ASSERT(settings.lifetime > 0, "Lifetime must be greater than 0");
                auto& p = emitter.m_particles[emitter.m_nextFreeParticle];
                p.colour = settings.colour;
                p.gravity = settings.gravity;
                p.lifetime = settings.lifetime + xy::Util::Random::value(-settings.lifetimeVariance, settings.lifetimeVariance + epsilon);
                p.maxLifetime = p.lifetime;
                p.velocity = /*rotation **/ settings.initialVelocity; //this should rotate the velocity
                p.rotation = Util::Random::value(-Util::Const::TAU, Util::Const::TAU);
                p.scale = 1.f;

                //spawn particle in world position
                p.position = tx.getWorldTransform().transformPoint({});

                //add random radius placement - TODO how to do with a position table? CAN'T HAVE +- 0!!
                p.position.x += Util::Random::value(-settings.spawnRadius, settings.spawnRadius + epsilon);
                p.position.y += Util::Random::value(-settings.spawnRadius, settings.spawnRadius + epsilon);

                emitter.m_nextFreeParticle++;
            }
        }

        //update each particle
        sf::Vector2f minBounds(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        sf::Vector2f maxBounds;
        for (auto i = 0u; i < emitter.m_nextFreeParticle; ++i)
        {
            auto& p = emitter.m_particles[i];

            p.velocity += p.gravity * dt;
            for (auto f : emitter.settings.forces) p.velocity += f * dt;
            p.position += p.velocity * dt;

            p.lifetime -= dt;
            p.colour.a = static_cast<sf::Uint8>(255.f * (std::max(p.lifetime / p.maxLifetime, 0.f)));

            p.rotation += emitter.settings.rotationSpeed * dt;
            p.scale += ((p.scale * emitter.settings.scaleModifier) * dt);

            //update bounds for culling
            if (p.position.x < minBounds.x) minBounds.x = p.position.x;
            if (p.position.y < minBounds.y) minBounds.y = p.position.y;

            if (p.position.x > maxBounds.x) maxBounds.x = p.position.x;
            if (p.position.y > maxBounds.y) maxBounds.y = p.position.y;
        }
        emitter.m_bounds = { minBounds, maxBounds - minBounds };

        //go over again and remove dead particles with pop/swap
        for (auto i = 0u; i < emitter.m_nextFreeParticle; ++i)
        {
            if (emitter.m_particles[i].lifetime < 0)
            {
                emitter.m_nextFreeParticle--;
                std::swap(emitter.m_particles[i], emitter.m_particles[emitter.m_nextFreeParticle]);
            }
        }

        //check if visible and create a vertex array for it
        if (viewableArea.intersects(emitter.m_bounds)) 
        {
            auto& vertArray = m_emitterArrays[m_activeArrayCount++];
            vertArray.count = 0;
            vertArray.texture = emitter.settings.texture;

            for (auto i = 0u; i < emitter.m_nextFreeParticle; ++i)
            {
                vertArray.vertices[vertArray.count++] = 
                {
                    emitter.m_particles[i].position,
                    emitter.m_particles[i].colour,
                    sf::Vector2f(emitter.m_particles[i].rotation, emitter.m_particles[i].scale)
                    //TODO wrangle rotation/scale into UV
                };
            }
        }
        //DPRINT("Position", std::to_string(emitter.m_particles[0].position.y));
    }
    //DPRINT("Emitter Count", std::to_string(m_activeArrayCount));
}

//private
void ParticleSystem::onEntityAdded(xy::Entity entity)
{
    m_arrayCount++;

    if (m_arrayCount == m_emitterArrays.size() && m_emitterArrays.size() < MaxParticleSystems)
    {
        m_emitterArrays.resize(m_emitterArrays.size() + MinParticleSystems);
    }
}

void ParticleSystem::onEntityRemoved(xy::Entity entity)
{
    m_arrayCount--; //if this is right it should never go less than 0...
    if (m_arrayCount < (m_emitterArrays.size() - MinParticleSystems))
    {
        m_emitterArrays.resize(m_emitterArrays.size() - MinParticleSystems);
    }
}


void ParticleSystem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.shader = &m_shader;
    states.texture = &m_dummyTexture;
    
    glCheck(glEnable(GL_PROGRAM_POINT_SIZE));
    glCheck(glEnable(GL_POINT_SPRITE));
    for (auto i = 0u; i < m_activeArrayCount; ++i)
    {
        if (m_emitterArrays[i].texture)
        {
            m_shader.setUniform("u_texture", *m_emitterArrays[i].texture);
        }
        
        rt.draw(m_emitterArrays[i].vertices.data(), m_emitterArrays[i].count, sf::Points, states);
        //DPRINT("Particle Count", std::to_string(m_emitterArrays[i].count));
    }
    glCheck(glDisable(GL_PROGRAM_POINT_SIZE));
    glCheck(glDisable(GL_POINT_SPRITE));
}