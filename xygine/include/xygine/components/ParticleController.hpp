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

//manages spawning of particle systems on parent entity

#ifndef XY_PARTICLE_CONTROLLER_HPP_
#define XY_PARTICLE_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/components/ParticleSystem.hpp>

#include <SFML/Graphics/Color.hpp>

#include <vector>
#include <map>

namespace xy
{
    class ParticleController final : public Component
    {
    public:          
        using SystemId = sf::Int32;
        using Ptr = std::unique_ptr<ParticleController>;
    private:
        using FactoryFunc = Ptr(&)(MessageBus&);
    public:
        static const FactoryFunc create;

        explicit ParticleController(MessageBus&);
        ~ParticleController() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void handleMessage(const Message&) override;
        void onStart(Entity&);

        void addDefinition(SystemId, const ParticleSystem::Definition&);
        void fire(SystemId, const sf::Vector2f& position);

    private:

        Entity* m_entity;
        std::map<ParticleController::SystemId, std::pair<Entity*, ParticleSystem::Definition>> m_activeSystems;
    };
}
#endif //XY_PARTICLE_CONTROLLER_HPP_