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

#pragma once

#include "xyginext/Config.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/Rect.hpp>

#include <array>
#include <string>
#include <any>

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
        sf::Color colour;
        std::uint32_t frameID = 0;

        sf::Vector2f position;
        sf::Vector2f velocity;
        sf::Vector2f gravity;
        float lifetime = 0.f;
        float maxLifetime = 1.f;
        float frameTime = 0.f;
        float rotation = 0.f;
        float scale = 1.f;
        float acceleration = 0.f;
    };

    /*!
    \brief Settings used by a particle emitter to initialise new particles
    */
    struct XY_API EmitterSettings final
    {
        std::string texturePath;

        sf::BlendMode blendmode = sf::BlendAlpha;
        sf::Vector2f gravity;
        sf::Vector2f initialVelocity = { 0.f, -100.f };
        sf::Vector2f spawnOffset; //!< initial spawn position is offset this much

        std::array<sf::Vector2f, 4u> forces{};
        sf::Color colour = sf::Color::White;

        std::uint32_t emitCount = 1u; //!< amount released at once
        std::int32_t releaseCount = 0; //!< number of particles released before stopping (0 for infinite)
        std::uint32_t frameCount = 1u; //!< must be at least one. Texture width is divided by this to calc frame coords

        float lifetime = 1.f;
        float lifetimeVariance = 0.f;
        float spread = 0.f;
        float scaleModifier = 0.f;
        float acceleration = 1.f;
        float size = 10.f; //!< diameter of particle
        float emitRate = 10.f; //!< particles per second
        float rotationSpeed = 0.f;
        float spawnRadius = 10.f;
        float framerate = 1.f; //!< must be greater than zero

        sf::Texture* texture = nullptr; //!< Animated textues should have all frames in a single row.

        bool randomInitialRotation = true;
        bool inheritRotation = true;

        bool animate = false; //!< If true the particle will attempt to play through all animation frames once.
        bool useRandomFrame = false; //!< If true the particle will pick a frame at random when spawning

        bool loadFromFile(const std::string&, TextureResource&);
        bool loadFromFile(const std::string&, ResourceHandler&);
        bool saveToFile(const std::string&); //! <saves the current settings to a config file

    private:
        struct ResourcePointer final
        {
            TextureResource* tr = nullptr;
            ResourceHandler* rh = nullptr;
        };
        bool loadFromFile(const std::string&, ResourcePointer);
    };

    /*!
    \brief Particle Emitter.
    Component struct used to emit particles from an entity
    */
    class XY_API ParticleEmitter final
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

        static constexpr std::uint32_t MaxParticles = 1000u;
        
    private:

        std::uint32_t m_arrayIndex;

        std::array<Particle, MaxParticles> m_particles;
        std::size_t m_nextFreeParticle;

        bool m_running;
        sf::Clock m_emissionClock;

        sf::FloatRect m_bounds;

        std::int32_t m_releaseCount;

        friend class ParticleSystem;
    };
}
