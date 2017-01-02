/*********************************************************************
Matt Marchant 2014 - 2017
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
#include <xygine/Entity.hpp>

#include <SFML/Graphics/Color.hpp>

#include <vector>
#include <map>

namespace xy
{
    /*!
    \brief Particle System controller component

    The particle controller is a utility component for
    firing particle systems, recycling its resources as much
    as possible. Rather than create a new particle system
    each time one is triggered, the controller will look for
    existing inactive systems and use those. New systems are
    created only when none are available. A scene will normally
    only require a single controller attached to an entity in
    the scene. Particle controllers are then able to be triggered
    by sending a Command to its parent entity requesting a system
    be fired, or by adding custom message handlers. The controller
    is useful for creating effects such as explosions which maybe
    be repeated around the scene rather than particle systems 
    attached to specific entities such as trails.
    */
    class XY_EXPORT_API ParticleController final : public Component
    {
    public:          
        using SystemID = sf::Int32;
        using Ptr = std::unique_ptr<ParticleController>;

        explicit ParticleController(MessageBus&);
        ~ParticleController() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;
        void onStart(Entity&);

        /*!
        \brief Add a particle definition to the controller

        Particle Definitions allow creating preset particle systems
        either programatically, or via a loaded json file. When a
        definition is added to the controller it is paired with the
        given ID which can be used to identify a system when fired.

        \param SystemID An integer value used to identify a particular
        particle system definition
        \param ParticleSystem::Definition used to create a set of
        predefined values for a particle system

        \see ParticleSystem::Definition
        */
        void addDefinition(SystemID, const ParticleSystem::Definition&);
        /*!
        \brief Fires a particle system

        If a particle system has been added with the given ID the system
        is then started based on its definition paramters.

        \param SystemID ID of the system to fire
        \param position World space coordinates at which the system is fired
        */
        void fire(SystemID, const sf::Vector2f& position);

    private:

        Entity* m_entity;
        std::vector<Entity::Ptr> m_pendingDefinitions;
        std::map<ParticleController::SystemID, std::pair<Entity*, ParticleSystem::Definition>> m_activeSystems;
    };
}
#endif //XY_PARTICLE_CONTROLLER_HPP_