/*********************************************************************
Matt Marchant 2014 - 2015
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

#include <xygine/Affectors.hpp>
#include <xygine/components/Component.hpp>
#include <xygine/Particle.hpp>

#include <deque>
#include <functional>
#include <vector>

namespace xy
{
    class TextureResource;
    class ParticleSystem final : public Component, public sf::Drawable
    {
    public:
        using Affector = std::function<void(Particle& p, float dt)>;
        using Ptr = std::unique_ptr<ParticleSystem>;

        //used for particle system presets
        class Definition final
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

            Ptr createSystem(MessageBus&) const;
            void loadFromFile(const std::string&, TextureResource&);

        private:
            void reset();
        };

    private:
        using FactoryFunc = Ptr(&)(MessageBus&);
    public:
        static const FactoryFunc create;

        explicit ParticleSystem(MessageBus&);
        ~ParticleSystem() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;

        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;

        void setTexture(const sf::Texture& t);
        void setColour(const sf::Color& colour);
        void setBlendMode(sf::BlendMode mode);
        void setShader(sf::Shader& shader);

        void setParticleSize(const sf::Vector2f& size);
        void setPosition(const sf::Vector2f& position);
        void move(const sf::Vector2f& amount);
        void followParent(bool);

        void setParticleLifetime(float time);
        void setInitialVelocity(const sf::Vector2f& vel);
        void setRandomInitialVelocity(const std::vector<sf::Vector2f>& randValues);
        void setEmitRate(float rate);
        void setRandomInitialPosition(const std::vector<sf::Vector2f>&);

        void addAffector(Affector& a);
        template <typename T>
        void addAffector(T& affector);

        void start(sf::Uint8 releaseCount = 1, float delay = 0.f, float duration = 0.f);
        bool started() const;
        void stop();
        void update(float dt);
        bool active() const;

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
        float m_startDelay;

        sf::Vector2f m_initialVelocity;
        bool m_randVelocity;
        std::vector<sf::Vector2f> m_randVelocities;
        float m_emitRate;
        bool m_randPosition;
        std::vector<sf::Vector2f> m_randPositions;

        bool m_started;
        float m_accumulator;

        std::vector<Affector> m_affectors;

        mutable std::vector<sf::Vertex> m_vertices;
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
