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

#include "xyginext/ecs/System.hpp"
#include "xyginext/ecs/components/ParticleEmitter.hpp"

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <vector>
#include <array>

namespace xy
{
    /*!
    \brief ParticleSystem.
    Responsible for updating all ParticelEmitter components in the scene,
    and rendering the output.
    */
    class XY_API ParticleSystem final : public xy::System, public sf::Drawable
    {
    public:
        explicit ParticleSystem(xy::MessageBus&);
        ~ParticleSystem();

        ParticleSystem(const ParticleSystem&) = delete;
        ParticleSystem(ParticleSystem&&) = delete;

        ParticleSystem& operator = (const ParticleSystem&) = delete;
        ParticleSystem& operator = (ParticleSystem&&) = delete;

        void process(float) override;

        /*!
        \brief Returns a reference to the shader used by the particle system
        */
        sf::Shader& getShader() { return m_shader; }

        /*!
        \brief Disables rendering the particle system.
        Useful for multi-pass effects where particles should be omitted
        */
        void setVisible(bool visible) { m_visible = visible; }

    private:

        void onEntityAdded(xy::Entity) override;
        void onEntityRemoved(xy::Entity) override;

        mutable sf::Shader m_shader;
        bool m_visible;

        struct Vertex final
        {
            struct
            {
                float x = 0.f;
                float y = 0.f;
                float currentFrame = 0.f;
            }position;

            struct
            {
                std::uint8_t r = 0;
                std::uint8_t g = 0;
                std::uint8_t b = 0;
                std::uint8_t a = 0;
            }colour;

            struct
            {
                float rotation = 0.f;
                float scale = 1.f;
            }uv;

            static constexpr std::size_t PositionOffset = 0;
            static constexpr std::size_t ColourOffset = sizeof(position);
            static constexpr std::size_t UVOffset = ColourOffset + sizeof(colour);
        };

        struct EmitterArray final
        {
            std::array<Vertex, ParticleEmitter::MaxParticles> vertices;
            std::size_t count = 0;
            sf::Texture* texture = nullptr;
            sf::FloatRect bounds;
            sf::BlendMode blendMode = sf::BlendAlpha;
            std::uint32_t frameCount = 1;
        };

        std::vector<EmitterArray> m_emitterArrays;
        std::size_t m_arrayCount;
        std::size_t m_activeArrayCount;

        sf::Texture m_fallbackTexture;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}
