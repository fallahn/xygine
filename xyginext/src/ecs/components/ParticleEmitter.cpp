/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include "xyginext/ecs/components/ParticleEmitter.hpp"
#include "xyginext/resources/Resource.hpp"
#include "xyginext/core/ConfigFile.hpp"

#include "xyginext/detail/Serializers.hpp"

#include "cereal/archives/binary.hpp"
#include "cereal/archives/json.hpp"

using namespace xy;

ParticleEmitter::ParticleEmitter()
    : m_arrayIndex      (0),
    m_nextFreeParticle  (0),
    m_running           (false),
    m_releaseCount      (-1)
{

}

//public
void ParticleEmitter::start()
{
    m_running = true;

    if (settings.releaseCount)
    {
        m_releaseCount = settings.releaseCount;
    }
}

void ParticleEmitter::stop()
{
    m_running = false;
}


//----emitter settings loader----//
bool EmitterSettings::loadFromFile(const std::string& path, TextureResource& textureResource)
{
    ConfigFile cfg;
    if (!cfg.loadFromFile(xy::FileSystem::getResourcePath() + path)) return false;

    if (cfg.getName() == "particle_system")
    {
        const auto& properties = cfg.getProperties();
        for (const auto& p : properties)
        {
            auto name = p.getName();
            if (name == "src")
            {
                texturePath = p.getValue<std::string>();
                texture = &textureResource.get(texturePath);
            }
            else if (name == "blendmode")
            {
                auto mode = p.getValue<std::string>();
                if (mode == "add")
                {
                    blendmode = sf::BlendAdd;
                }
                else if (mode == "multiply")
                {
                    blendmode = sf::BlendMultiply;
                }
                else
                {
                    blendmode = sf::BlendAlpha;
                }
            }
            else if (name == "gravity")
            {
                gravity = p.getValue<sf::Vector2f>();
            }
            else if (name == "velocity")
            {
                initialVelocity = p.getValue<sf::Vector2f>();
            }
            else if (name == "spread")
            {
                spread = p.getValue<float>() / 2.f; //because the initial rotation is +- this
            }
            else if (name == "lifetime")
            {
                lifetime = p.getValue<float>();
            }
            else if (name == "lifetime_variance")
            {
                lifetimeVariance = p.getValue<float>();
            }
            else if (name == "colour")
            {
                colour = p.getValue<sf::Color>();
            }
            else if (name == "random_initial_rotation")
            {
                randomInitialRotation = p.getValue<bool>();
            }
            else if (name == "rotation_speed")
            {
                rotationSpeed = p.getValue<float>();
            }
            else if (name == "scale_affector")
            {
                scaleModifier = p.getValue<float>();
            }
            else if (name == "size")
            {
                size = p.getValue<float>();
            }
            else if (name == "emit_rate")
            {
                emitRate = p.getValue<float>();
            }
            else if (name == "emit_count")
            {
                emitCount = p.getValue<sf::Int32>();
            }
            else if (name == "spawn_radius")
            {
                spawnRadius = p.getValue<float>();
            }
            else if (name == "spawn_offset")
            {
                spawnOffset = p.getValue<sf::Vector2f>();
            }
            else if (name == "release_count")
            {
                releaseCount = p.getValue<sf::Int32>();
            }
        }

        const auto& objects = cfg.getObjects();
        for (const auto& o : objects)
        {
            //load force array
            if (o.getName() == "forces")
            {
                const auto& props = o.getProperties();
                std::size_t currentForce = 0;
                for (const auto& force : props)
                {
                    if (force.getName() == "force")
                    {
                        forces[currentForce++] = force.getValue<sf::Vector2f>();
                        if (currentForce == forces.size())
                        {
                            break;
                        }
                    }
                }
            }
        }

        if (!texture)
        {
            Logger::log(path + ": no texture property found", Logger::Type::Warning);
        }
    }

    return false;
}

bool EmitterSettings::saveToFile(const std::string& path)
{
    ConfigFile cfg("particle_system");
    
    cfg.addProperty("src").setValue(texturePath);
    auto p = cfg.addProperty("blendmode");
    if (blendmode == sf::BlendAdd)
    {
        p.setValue("add");
    }
    else if (blendmode == sf::BlendMultiply)
    {
        p.setValue("multiply");
    }
    else
    {
        p.setValue("alpha");
    }
    
    cfg.addProperty("gravity").setValue(gravity);
    cfg.addProperty("velocity").setValue(initialVelocity);
    cfg.addProperty("spread").setValue(spread * 2.f); // Is the *2 correct? just copied from loadFromFile
    cfg.addProperty("lifetime").setValue(lifetime);
    cfg.addProperty("lifetime_variance").setValue(lifetimeVariance);
    cfg.addProperty("colour").setValue(colour);
    cfg.addProperty("random_initial_rotation").setValue(randomInitialRotation);
    cfg.addProperty("rotation_speed").setValue(rotationSpeed);
    cfg.addProperty("scale_affector").setValue(scaleModifier);
    cfg.addProperty("size").setValue(size);
    cfg.addProperty("emit_rate").setValue(emitRate);
    cfg.addProperty("emit_count").setValue(emitCount);
    cfg.addProperty("spawn_radius").setValue(spawnRadius);
    cfg.addProperty("spawn_offset").setValue(spawnOffset);
    cfg.addProperty("release_count").setValue(releaseCount);
    
    
    auto forcesObj = cfg.addObject("forces");
    for (auto& force : forces)
    {
        forcesObj->addProperty("force").setValue(force);
    }
    
    return cfg.save(path);
}

template<class Archive>
void ParticleEmitter::serialize(Archive& ar, const std::uint32_t version)
{
    ar(m_bounds);
    ar(m_releaseCount);
}

template void ParticleEmitter::serialize<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&, const std::uint32_t);
template void ParticleEmitter::serialize<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&, const std::uint32_t);

template void ParticleEmitter::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive&, const std::uint32_t);
template void ParticleEmitter::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&, const std::uint32_t);
