/*********************************************************************
(c) Matt Marchant 2017 - 2020
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

#include "xyginext/Config.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <array>
#include <string>

namespace sf
{
    class Texture;
}

namespace xy
{
    class TextureResource;
    class ResourceHandler;

    /*!
    \brief Represents a single particle in a system
    */
    struct Particle final
    {
        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f gravity;
        float lifetime = 0.f;
        float maxLifetime = 1.f;
        sf::Color colour;
        float rotation = 0.f;
        float scale = 1.f;
        float acceleration = 0.f;
    };

    /*!
    \brief Settings used by a particle emitter to initialise new particles
    */
    struct XY_EXPORT_API EmitterSettings final
    {
        sf::BlendMode blendmode = sf::BlendAlpha;
        sf::Vector2f gravity;
        sf::Vector2f initialVelocity = { 0.f, -100.f };
        float spread = 0.f;
        std::array<sf::Vector2f, 4> forces{};
        float lifetime = 1.f;
        float lifetimeVariance = 0.f;
        sf::Color colour = sf::Color::White;
        float rotationSpeed = 0.f;
        bool randomInitialRotation = true;
        float scaleModifier = 0.f;
        float acceleration = 1.f;
        float size = 10.f; //! <diameter of particle
        float emitRate = 10.f; //! <particles per second
        sf::Uint32 emitCount = 1; //! <amount released at once
        float spawnRadius = 10.f;
        sf::Vector2f spawnOffset; //! <initial spawn position is offset this much
        sf::Int32 releaseCount = 0; //! <number of particles released before stopping (0 for infinite)
        sf::Texture* texture = nullptr;
        std::string texturePath;
        bool loadFromFile(const std::string&, TextureResource&);
        bool loadFromFile(const std::string&, ResourceHandler&);
        bool saveToFile(const std::string&); //! <saves the current settings to a config file
    };

    /*!
    \brief Particle Emitter.
    Component struct used to emit particles from an entity
    */
    class XY_EXPORT_API ParticleEmitter final
    {
    public:
        ParticleEmitter();

        /*!
        \brief Starts the emitter
        */
        void start();
        /*!
        \brief Stops the emitter
        */
        void stop();

        /*!
        \brief Returns true if emitter has stopped
        */
        bool stopped() const { return (!m_running && m_nextFreeParticle == 0); }

        /*!
        \brief Settings used by this emitter when creating a new particle
        */
        EmitterSettings settings;

        static constexpr sf::Uint32 MaxParticles = 1000u;
        
    private:

        sf::Uint32 m_arrayIndex;

        std::array<Particle, MaxParticles> m_particles;
        std::size_t m_nextFreeParticle;

        bool m_running;
        sf::Clock m_emissionClock;

        sf::FloatRect m_bounds;

        sf::Int32 m_releaseCount;

        friend class ParticleSystem;
    };
}
