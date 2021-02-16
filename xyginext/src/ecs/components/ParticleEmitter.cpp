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

#include "xyginext/ecs/components/ParticleEmitter.hpp"
#include "xyginext/resources/Resource.hpp"
#include "xyginext/resources/ResourceHandler.hpp"
#include "xyginext/core/ConfigFile.hpp"

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
    m_releaseCount = -1;
}


//----emitter settings loader----//
bool EmitterSettings::loadFromFile(const std::string& path, TextureResource& textureResource)
{
    ResourcePointer rp;
    rp.tr = &textureResource;
    return loadFromFile(path, rp);
}

bool EmitterSettings::loadFromFile(const std::string& path, ResourceHandler& textureResource)
{
    ResourcePointer rp;
    rp.rh = &textureResource;
    return loadFromFile(path, rp);
}

bool EmitterSettings::saveToFile(const std::string& path)
{
    auto emitterName = xy::FileSystem::getFileName(path);
    emitterName = emitterName.substr(0, emitterName.size() - 4);

    ConfigFile cfg("particle_system", emitterName);

    auto texPath = texturePath;
    std::replace(texPath.begin(), texPath.end(), '\\', '/');
    if (!texPath.empty())
    {
        if (texPath[0] == '/')
        {
            texPath = texPath.substr(1);
        }
    }

    cfg.addProperty("src", "\"" + texPath + "\"");
    
    if (blendmode == sf::BlendAdd)
    {
        cfg.addProperty("blendmode", "add");
    }
    else if (blendmode == sf::BlendMultiply)
    {
        cfg.addProperty("blendmode", "multiply");
    }
    else
    {
        cfg.addProperty("blendmode", "alpha");
    }

    cfg.addProperty("acceleration").setValue(acceleration);
    cfg.addProperty("gravity").setValue(gravity);
    cfg.addProperty("velocity").setValue(initialVelocity);
    cfg.addProperty("spread").setValue(spread * 2.f);
    cfg.addProperty("lifetime").setValue(lifetime);
    cfg.addProperty("lifetime_variance").setValue(lifetimeVariance);
    cfg.addProperty("colour").setValue(colour);
    cfg.addProperty("random_initial_rotation").setValue(randomInitialRotation);
    cfg.addProperty("rotation_speed").setValue(rotationSpeed);
    cfg.addProperty("scale_affector").setValue(scaleModifier);
    cfg.addProperty("size").setValue(size);
    cfg.addProperty("emit_rate").setValue(emitRate);
    cfg.addProperty("emit_count").setValue(static_cast<std::int32_t>(emitCount));
    cfg.addProperty("spawn_radius").setValue(spawnRadius);
    cfg.addProperty("spawn_offset").setValue(spawnOffset);
    cfg.addProperty("release_count").setValue(releaseCount);
    cfg.addProperty("inherit_rotation").setValue(inheritRotation);
    cfg.addProperty("frame_count").setValue(static_cast<std::int32_t>(frameCount));
    cfg.addProperty("animate").setValue(animate);
    cfg.addProperty("random_frame").setValue(useRandomFrame);
    cfg.addProperty("framerate").setValue(framerate);

    auto forceObj = cfg.addObject("forces");
    for (const auto& f : forces)
    {
        forceObj->addProperty("force").setValue(f);
    }

    return cfg.save(path);
}

//private
bool EmitterSettings::loadFromFile(const std::string& path, ResourcePointer resources)
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
                //make sure to replace windows back slashes
                auto path = p.getValue<std::string>();
                if (!path.empty())
                {
                    std::replace(path.begin(), path.end(), '\\', '/');

                    if (path[0] == '/')
                    {
                        path = path.substr(1);
                    }

                    if (resources.tr)
                    {
                        texture = &resources.tr->get(path);
                    }
                    else
                    {
                        XY_ASSERT(resources.rh != nullptr, "");
                        auto handle = resources.rh->load<sf::Texture>(p.getValue<std::string>());
                        texture = &resources.rh->get<sf::Texture>(handle);
                    }
                    texturePath = path;
                }
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
            else if (name == "acceleration")
            {
                acceleration = p.getValue<float>();
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
                emitCount = p.getValue<std::int32_t>();
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
                releaseCount = p.getValue<std::int32_t>();
            }
            else if (name == "inherit_rotation")
            {
                inheritRotation = p.getValue<bool>();
            }
            else if (name == "frame_count")
            {
                frameCount = std::max(1, p.getValue<std::int32_t>());
            }
            else if (name == "animate")
            {
                animate = p.getValue<bool>();
            }
            else if (name == "random_frame")
            {
                useRandomFrame = p.getValue<bool>();
            }
            else if (name == "framerate")
            {
                framerate = p.getValue<float>();
                if (framerate <= 0)
                {
                    framerate = 1.f;
                }
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
    }

    return false;
}