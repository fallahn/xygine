/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

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

#ifndef XY_PARTICLESYS_HPP_
#define XY_PARTICLESYS_HPP_

#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <xygine/ParticleAffectors.hpp>
#include <xygine/components/Component.hpp>
#include <xygine/Particle.hpp>

#include <deque>
#include <functional>
#include <vector>

namespace xy
{
    class TextureResource;
    /*!
    \brief Particle System Component

    Particle systems are flexible vertex array based drawable component
    useful for creating effects such as smoke, flames and sparks. While
    particle system parameters are manually adjustable The Definition
    sub-class is useful for creating particle presets and even includes
    a function for returning particle system components which are
    automatically configured based on the definition properties. Definitions
    can also be loaded from json files, created with apps such as the
    xygine particle editor found in the xygine repository.
    */
    class XY_EXPORT_API ParticleSystem final : public Component, public sf::Drawable
    {
    public:
        using Affector = std::function<void(Particle& p, float dt)>;
        using Ptr = std::unique_ptr<ParticleSystem>;

        /*!
        \brief Particle System preset

        Particle System Definitions allow creation of preset parameters
        for particle systems. This is particulary useful when loading 
        particle system configuration files from disk (which have the
        file extension *.xyp). Definitions can also be used directly with
        the Particle Controller component.
        */
        class XY_EXPORT_API Definition final
        {
        public:
            Definition();
            ~Definition() = default;
            /*Definition(const Definition&) = delete;
            Definition& operator = (const Definition) = delete;*/

            sf::Uint8 releaseCount = 1u;
            float delay = 0.f;
            float duration = 1.f;

            sf::Texture* texture = nullptr;
            sf::Color colour = sf::Color::White;
            sf::BlendMode blendMode;
            sf::Shader* shader = nullptr;

            sf::Vector2f particleSize = { 4.f, 4.f };
            sf::Vector2f particlePosition;
            bool followParent = false;

            float lifetime = 2.f;
            sf::Vector2f initialVelocity = { 1.f, 1.f };
            std::vector<sf::Vector2f> randomInitialVelocities;
            float emitRate = 30.f;
            std::vector<sf::Vector2f> randomInitialPositions;

            std::vector<Affector> affectors;

            /*!
            \brief Creates a Particle System Component

            \returns A new Particle System preconfigured with
            the definition paramters.
            */
            Ptr createSystem(MessageBus&) const;
            /*!
            \brief Loads a particle system configuration file

            Configuration files are a json formatted text file, usually
            with the file extension *.xyp. If file loading or parsing
            fails then the definition is left unmodified from its
            existing state.
            */
            void loadFromFile(const std::string&, TextureResource&);

        private:
            void reset();
        };

        explicit ParticleSystem(MessageBus&);
        ~ParticleSystem() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void onParentDestroyed(Entity&) override;

        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;

        /*!
        \brief Sets the optional texture to be used by the particle system
        \param t Valid SFML texture to use
        */
        void setTexture(const sf::Texture& t);
        /*!
        \brief Sets the colour with which the particle system is multiplied

        \param colour SFML colour with with to multiply the particle system.
        Default colour is white.
        */
        void setColour(const sf::Color& colour);
        /*!
        \brief Sets the blend mode used when drawing the particle system
        \param mode SFML Blend Mode. Defaults to BlendaAlpha
        */
        void setBlendMode(sf::BlendMode mode);
        /*!
        \brief Sets an optional shader to be used when drawing the particle
        system.

        \param shader Reference to a valid SFML shader. Note the particle
        system only keeps a pointer to the shader, so the shader itself must
        exist at least as long as the ParticleSystem
        \see ShaderResource
        */
        void setShader(sf::Shader& shader);

        /*!
        \brief Sets the initial size of the particles emitted by the particle
        system

        \param size An sf::Vector2f representing the width and height of the particle
        */
        void setParticleSize(const sf::Vector2f& size);
        /*!
        \brief Sets the initial position of emitted particles relative the
        component's parent entity's world position.

        \param position Relative position of the emitted particle
        */
        void setPosition(const sf::Vector2f& position);
        void move(const sf::Vector2f& amount);
        /*!
        \brief Sets whether the emitted particles should follow the parent
        entity once emitted.

        By default this is false, so that particles are left by moving entities
        as a trail, such as smoke or flames. In some cases it is desirable to
        have emitted particles to continue to follow their parent

        \param bool true to make particles follow their parent, false by default
        */
        void followParent(bool);

        /*!
        \brief Sets the particle lifetime

        The lifetime of a particle dictates how quickly it fades in to non-existence

        \param time Number of seconds the particle exists for before fading away
        */
        void setParticleLifetime(float time);
        /*!
        \brief Sets the amount by with particle lifetime should vary

        A positive ratio of the current lifetime value which is added
        or subtracted from a new particle's lifetime. For example a value
        of 0.5 will make a new particle's lifetime a value of +- 50% of
        that of the ParticleLifetime value.
        \param amount Ratio to vary the particle's lifetime by
        */
        void setLifetimeVariance(float amount);
        /*!
        \brief Set the initial velocity of emitted particles

        The initial velocity of a particle defines the speed and direction of
        a particle when it is first emitted. This parameter is overriden by any
        random initial velocities if they are set.

        \param vel sf::Vector2f representing the speed and direction of spawned
        particles
        */
        void setInitialVelocity(const sf::Vector2f& vel);
        /*!
        \brief Enable spawning particles with pseudo random initial velocities

        Supplying a selection of possible initial velocities allows the particle system
        to pick one at random when emitting a new particle. These values override
        any value set with setInitialVelocity.

        \param randValues a std::vector of sf::Vector2f representing possible initial
        speed and directions for newly emitted particles
        */
        void setRandomInitialVelocity(const std::vector<sf::Vector2f>& randValues);
        /*!
        \brief Sets the inertia of the particle system.

        If a particle system is attached to a moving entity it can offer a more realistic
        appearance to set the inertia of the system to that of the parent entity's velocity.
        The intertia is then added to the initial velocity of any particles which are spawned

        \param sf::Vector2f inertia value to add to the particle's initial velocity
        */
        void setInertia(const sf::Vector2f&);
        /*!
        \brief Set the particle emit rate

        The emit rate defines how many particles are released per second by the particle system
        \param rate Number of particles to release per second.
        */
        void setEmitRate(float rate);
        /*!
        \brief Tells the particle system to emit particles at a pseudo random position
        relative to the emitter position

        \param std::vector<sf::Vector2f> A std::vector of sf::Vector2f representing positions
        relative to the emitter. Positions are picked at random from the vector each time a
        new particle is emitted.
        */
        void setRandomInitialPosition(const std::vector<sf::Vector2f>&);

        /*!
        \brief Add a ParticleAffector to the particle system

        Affectors influence the behaviour of emitted particles over time, such as
        speed, direction or rotation.
        \see Affector
        */
        void addAffector(Affector& a);
        template <typename T>
        void addAffector(T& affector);

        /*!
        \brief Starts the particle system

        When the particle system is started it will emit particles based on the current
        parameters.

        \param releaseCount Number of particles to release at once
        \param delay Time in seconds to delay after start has been called before particles
        are emitted
        \param duration Amount of time in seconds to emit particles for. If this is set to
        zero particles will emit indefinitely, until stop() is called.
        */
        void start(sf::Uint8 releaseCount = 1, float delay = 0.f, float duration = 0.f);
        /*!
        \brief Returns true if the particle system is currently started and emitting particles
        */
        bool started() const;
        /*!
        \brief stops the particle system from emitting particles
        */
        void stop();
        /*!
        \brief Allows xygine to update the particle system

        No need to call this manually
        */
        void update(float dt);
        /*!
        \brief Returns true if this is an active particle system

        Once a particle system is stopped it may stay have existing particles being drawn to
        screen. Use this to check if it is safe to remove a particle system once it has been
        stopped.
        */
        bool active() const;
        /*!
        \brief Return the number of currently active particles
        */
        sf::Uint32 getParticleCount() const;

    private:
        std::deque<Particle> m_particles;
        sf::Texture* m_texture;
        sf::Color m_colour;
        sf::Vector2f m_position;
        bool m_followParent;
        sf::Vector2f m_particleSize;
        sf::Vector2f m_texCoords;
        float m_particleLifetime;
        float m_lifetimeVariance;
        float m_startDelay;

        sf::Vector2f m_initialVelocity;
        bool m_randVelocity;
        std::vector<sf::Vector2f> m_randVelocities;
        sf::Vector2f m_inertia;
        float m_emitRate;
        bool m_randPosition;
        std::vector<sf::Vector2f> m_randPositions;
        sf::Vector2f m_offset;

        bool m_started;
        float m_accumulator;

        std::vector<Affector> m_affectors;

        mutable std::vector<sf::Vertex> m_vertices;
        mutable std::size_t m_vertexCount;
        mutable bool m_needsUpdate;
        mutable sf::FloatRect m_bounds;

        sf::Clock m_durationClock;
        float m_duration;
        sf::Uint8 m_releaseCount;

        sf::BlendMode m_blendMode;
        sf::Shader* m_shader;

        void emit(float dt);
        void addParticle(const sf::Vector2f& position);
        void addVertex(const sf::Vector2f& position, float u, float v, const sf::Color& colour)const;
        void updateVertices() const;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
    };

    template <typename T>
    void ParticleSystem::addAffector(T& affector)
    {
        Affector a = std::bind(affector, std::placeholders::_1, std::placeholders::_2);
        m_affectors.push_back(a);
    }
}
#endif //XY_PARTICLES_HPP_
