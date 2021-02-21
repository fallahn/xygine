/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "xyginext/ecs/systems/ParticleSystem.hpp"
#include "xyginext/ecs/components/Transform.hpp"
#include "xyginext/ecs/components/ParticleEmitter.hpp"
#include "xyginext/core/App.hpp"
#include "xyginext/util/Const.hpp"
#include "xyginext/util/Random.hpp"
#include "xyginext/util/Vector.hpp"

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
    //convert an sf::BlendMode::Factor constant to the corresponding OpenGL constant.
    //taken from sf::RenderTarget source
    std::uint32_t factorToGlConstant(sf::BlendMode::Factor blendFactor)
    {
        switch (blendFactor)
        {
        case sf::BlendMode::Zero:             return GL_ZERO;
        case sf::BlendMode::One:              return GL_ONE;
        case sf::BlendMode::SrcColor:         return GL_SRC_COLOR;
        case sf::BlendMode::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
        case sf::BlendMode::DstColor:         return GL_DST_COLOR;
        case sf::BlendMode::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
        case sf::BlendMode::SrcAlpha:         return GL_SRC_ALPHA;
        case sf::BlendMode::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case sf::BlendMode::DstAlpha:         return GL_DST_ALPHA;
        case sf::BlendMode::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
        }

        return GL_ZERO;
    }


    //convert an sf::BlendMode::BlendEquation constant to the corresponding OpenGL constant.
    //taken from sf::RenderTarget source
    std::uint32_t equationToGlConstant(sf::BlendMode::Equation blendEquation)
    {
        switch (blendEquation)
        {
        case sf::BlendMode::Add:             return GL_FUNC_ADD;
        case sf::BlendMode::Subtract:        return GL_FUNC_SUBTRACT;
        case sf::BlendMode::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        }
        return GL_FUNC_ADD;
    }

    const std::string VertexShader = R"(
        #version 120    
        
        uniform float u_screenScale;

        varying mat2 v_rotation;
        varying float v_currentFrame;
        
        void main()
        {
            vec4 rotScale = gl_TextureMatrix[0] * gl_MultiTexCoord0; //we've shanghaid the text coords to hold rotation and scale

            vec2 rot = vec2(sin(rotScale.x), cos(rotScale.x));
            v_rotation[0] = vec2(rot.y, -rot.x);
            v_rotation[1] = rot;

            //current frame is stored in Z coord.
            v_currentFrame = gl_Vertex.z;

            gl_FrontColor = gl_Color;
            vec4 position = vec4(gl_Vertex.xy, 0.0, 1.0);
            gl_Position = gl_ModelViewProjectionMatrix * position;
            gl_PointSize = rotScale.y * u_screenScale;
        })";

    const std::string FragmentShader = R"(
        #version 120

        uniform sampler2D u_texture;

        uniform float u_frameCount;
        uniform vec2 u_textureSize;
        
        varying mat2 v_rotation;
        varying float v_currentFrame;

        void main()
        {
            float frameWidth = 1.0 / u_frameCount;

            vec2 coord = gl_PointCoord;
            coord.x *= frameWidth;
            coord.x += v_currentFrame * frameWidth;

            vec2 centreOffset = vec2((v_currentFrame * frameWidth) + (frameWidth / 2.f), 0.5);

            //convert to texture space
            coord *= u_textureSize;
            centreOffset *= u_textureSize;

            //rotate
            coord = v_rotation * (coord - centreOffset);
            coord += centreOffset;

            //and back to UV space
            coord /= u_textureSize;

            gl_FragColor = gl_Color * texture2D(u_texture, coord);
        })";

    const std::size_t MaxParticleSystems = 64; //max VBOs, must be divisible by min count
    const std::size_t MinParticleSystems = 4; //min amount before resizing. This many are added on resize
}

ParticleSystem::ParticleSystem(xy::MessageBus& mb)
    : xy::System        (mb, typeid(ParticleSystem)),
    m_visible           (true),
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

    sf::Image img;
    img.create(16, 16, sf::Color::White);
    m_fallbackTexture.loadFromImage(img);
}

ParticleSystem::~ParticleSystem()
{

}

//public
void ParticleSystem::process(float dt)
{
    m_activeArrayCount = 0;

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
            auto emitCount = emitter.settings.emitCount;
            while (emitCount--)
            {
                if (emitter.m_nextFreeParticle < emitter.m_particles.size() - 1)
                {
                    auto& tx = entity.getComponent<Transform>();
                    auto rotation = emitter.settings.inheritRotation ? tx.getWorldRotation() : 0.f;

                    const auto& settings = emitter.settings;
                    XY_ASSERT(settings.emitRate > 0, "Emit rate must be grater than 0");
                    XY_ASSERT(settings.lifetime > 0, "Lifetime must be greater than 0");
                    auto& p = emitter.m_particles[emitter.m_nextFreeParticle];
                    p.colour = settings.colour;
                    p.gravity = settings.gravity;
                    p.lifetime = settings.lifetime + xy::Util::Random::value(-settings.lifetimeVariance, settings.lifetimeVariance + epsilon);
                    p.maxLifetime = p.lifetime;
                    p.velocity = Util::Vector::rotate(settings.initialVelocity, rotation + Util::Random::value(-settings.spread, (settings.spread + epsilon)));
                    p.rotation = (settings.randomInitialRotation) ?  Util::Random::value(-Util::Const::TAU, Util::Const::TAU) : rotation * xy::Util::Const::degToRad;
                    p.scale = settings.size;
                    p.acceleration = settings.acceleration;
                    p.frameID = settings.useRandomFrame ? xy::Util::Random::value(0, static_cast<std::int32_t>(settings.frameCount) - 1) : 0;
                    p.frameTime = 0.f;

                    //spawn particle in world position
                    p.position = tx.getWorldTransform().transformPoint(tx.getOrigin());

                    //add random radius placement - TODO how to do with a position table? CAN'T HAVE +- 0!!
                    p.position.x += Util::Random::value(-settings.spawnRadius, settings.spawnRadius + epsilon);
                    p.position.y += Util::Random::value(-settings.spawnRadius, settings.spawnRadius + epsilon);

                    auto offset = settings.spawnOffset;
                    offset.x *= tx.getScale().x;
                    offset.y *= tx.getScale().y;
                    p.position += offset;

                    emitter.m_nextFreeParticle++;
                    if(emitter.m_releaseCount > 0) emitter.m_releaseCount--;
                }
            }
        }
        if (emitter.m_releaseCount == 0) emitter.stop();

        //update each particle
        sf::Vector2f minBounds(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        sf::Vector2f maxBounds;

        float framerate = 1.f / emitter.settings.framerate;
        for (auto i = 0u; i < emitter.m_nextFreeParticle; ++i)
        {
            auto& p = emitter.m_particles[i];

            p.velocity += p.gravity * dt;
            for (auto f : emitter.settings.forces)
            {
                p.velocity += f * dt;
            }
            p.velocity *= p.acceleration;
            p.position += p.velocity * dt;

            p.lifetime -= dt;
            p.colour.a = static_cast<std::uint8_t>(255.f * (std::max(p.lifetime / p.maxLifetime, 0.f)));

            p.rotation += emitter.settings.rotationSpeed * dt;
            p.scale += ((p.scale * emitter.settings.scaleModifier) * dt);

            if (emitter.settings.animate)
            {
                p.frameTime += dt;
                if (p.frameTime > framerate)
                {
                    p.frameID++;
                    p.frameTime -= framerate;
                }
            }

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
            if (emitter.m_particles[i].lifetime < 0
                || emitter.m_particles[i].frameID == emitter.settings.frameCount)
            {
                emitter.m_nextFreeParticle--;
                std::swap(emitter.m_particles[i], emitter.m_particles[emitter.m_nextFreeParticle]);
            }
        }

        //limit max number of active systems and generate actual vert array
        if (m_activeArrayCount < MaxParticleSystems) 
        {
            auto& vertArray = m_emitterArrays[m_activeArrayCount++];
            vertArray.count = 0;
            vertArray.texture = (emitter.settings.texture) ? emitter.settings.texture : &m_fallbackTexture;
            vertArray.bounds = emitter.m_bounds;
            vertArray.blendMode = emitter.settings.blendmode;
            vertArray.frameCount = emitter.settings.frameCount;

            for (auto i = 0u; i < emitter.m_nextFreeParticle; ++i)
            {
                auto& vert = vertArray.vertices[vertArray.count++];
                const auto& particle = emitter.m_particles[i];
                vert.position.x = particle.position.x;
                vert.position.y = particle.position.y;
                vert.position.currentFrame = static_cast<float>(particle.frameID);

                vert.colour.r = particle.colour.r;
                vert.colour.g = particle.colour.g;
                vert.colour.b = particle.colour.b;
                vert.colour.a = particle.colour.a;

                vert.uv.rotation = particle.rotation;
                vert.uv.scale = particle.scale;
            }
        }
    }
}

//private
void ParticleSystem::onEntityAdded(xy::Entity)
{
    m_arrayCount++;

    if (m_arrayCount == m_emitterArrays.size()/* && m_emitterArrays.size() < MaxParticleSystems*/)
    {
        m_emitterArrays.resize(m_emitterArrays.size() + MinParticleSystems);
    }
}

void ParticleSystem::onEntityRemoved(xy::Entity)
{
    m_arrayCount--; //if this is right it should never go less than 0...
    if (m_arrayCount < (m_emitterArrays.size() - MinParticleSystems))
    {
        m_emitterArrays.resize(m_emitterArrays.size() - MinParticleSystems);
    }
}


void ParticleSystem::draw(sf::RenderTarget& rt, sf::RenderStates) const
{
    //TODO lookup and cache shader uniforms - however this will need special
    //care when setting custom shaders
    if (m_visible)
    {
        rt.pushGLStates();

        const auto& view = rt.getView();
        sf::FloatRect viewableArea(view.getCenter() - (view.getSize() / 2.f), view.getSize());

        //apply the view as SFML would
        auto viewport = rt.getViewport(view);
        auto top = rt.getSize().y - (viewport.top + viewport.height);
        glCheck(glViewport(viewport.left, top, viewport.width, viewport.height));

        //set the projection matrix
        glCheck(glMatrixMode(GL_PROJECTION));
        glCheck(glLoadMatrixf(view.getTransform().getMatrix()));

        //go back to model-view mode
        glCheck(glMatrixMode(GL_MODELVIEW));


        //apply the shader
        sf::Shader::bind(&m_shader);

        //scale particles to match screen size
        float ratio = static_cast<float>(rt.getSize().x) / viewableArea.width;
        m_shader.setUniform("u_screenScale", ratio);

        //we shouldn't need to apply the view as SFML ought to have
        //set it up before the RenderTarget calls this function

        glCheck(glEnable(GL_PROGRAM_POINT_SIZE));
        glCheck(glEnable(GL_POINT_SPRITE));

        //TODO can we ask SFML if this is already enabled?
        glCheck(glEnableClientState(GL_TEXTURE_COORD_ARRAY));

        //set up the model matrix
        //particles are always emitted in world space so we use an identity matrix
        //and load it just once before rendering the particle arrays
        glCheck(glLoadIdentity());

        for (auto i = 0u; i < m_activeArrayCount; ++i)
        {
            if (m_emitterArrays[i].bounds.intersects(viewableArea))
            {
                //TODO can we query SFML to find out if this texture is already bound?
                //bind() doesn't track this for us.
                sf::Texture::bind(m_emitterArrays[i].texture);
                m_shader.setUniform("u_frameCount", static_cast<float>(m_emitterArrays[i].frameCount));
                m_shader.setUniform("u_texture", sf::Shader::CurrentTexture);
                m_shader.setUniform("u_textureSize", sf::Glsl::Vec2(m_emitterArrays[i].texture->getSize()));

                //blend mode
                glCheck(glBlendFunc(factorToGlConstant(m_emitterArrays[i].blendMode.colorSrcFactor), factorToGlConstant(m_emitterArrays[i].blendMode.colorDstFactor)));

                const auto* data = reinterpret_cast<const char*>(m_emitterArrays[i].vertices.data());
                glCheck(glVertexPointer(3, GL_FLOAT, sizeof(Vertex), data + Vertex::PositionOffset));
                glCheck(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), data + Vertex::ColourOffset));
                glCheck(glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), data + Vertex::UVOffset));

                glCheck(glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_emitterArrays[i].count)));
            }
        }

        sf::Shader::bind(nullptr);

        glCheck(glDisable(GL_PROGRAM_POINT_SIZE));
        glCheck(glDisable(GL_POINT_SPRITE));

        rt.popGLStates();
    }
}
