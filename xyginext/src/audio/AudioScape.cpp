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

#include "xyginext/audio/AudioScape.hpp"
#include "xyginext/audio/Mixer.hpp"
#include "xyginext/resources/Resource.hpp"

using namespace xy;

AudioScape::AudioScape(AudioResource& rx)
    : m_audioResource(rx)
{

}

//public
bool AudioScape::loadFromFile(const std::string& path)
{
    ConfigFile file;
    if (!file.loadFromFile(xy::FileSystem::getResourcePath() + path))
    {
        xy::Logger::log(path + ": file not found", xy::Logger::Type::Error);
        return false;
    }

    m_emitterConfigs.clear();

    const auto& objs = file.getObjects();
    for (auto& obj : objs)
    {
        if (obj.getName() == "emitter")
        {
            auto name = obj.getId();
            if (name.empty())
            {
                continue;
            }

            //check we have min required properties
            if (obj.findProperty("path") == nullptr)
            {
                xy::Logger::log("Skipping " + name + " no source file information found", xy::Logger::Type::Warning);
                continue;
            }

            if (obj.findProperty("streaming") == nullptr)
            {
                xy::Logger::log("Skipping " + name + " no streaming property found", xy::Logger::Type::Warning);
                continue;
            }

            if (m_emitterConfigs.count(name) == 0)
            {
                m_emitterConfigs.insert(std::make_pair(name, obj));
            }
        }
    }

    return !m_emitterConfigs.empty();
}

bool AudioScape::saveToFile(const std::string& path)
{
    if (!m_emitterConfigs.empty())
    {
        ConfigFile file("audio_scape");
        for (const auto& cfg : m_emitterConfigs)
        {
            file.addObject(cfg.second);
        }
        return file.save(path);
    }

    return false;
}

AudioEmitter AudioScape::getEmitter(const std::string& name) const
{
    AudioEmitter emitter;

    if(m_emitterConfigs.count(name) != 0)
    {
        const auto& cfg = m_emitterConfigs.find(name);
        auto path = cfg->second.findProperty("path")->getValue<std::string>();
        bool streaming = cfg->second.findProperty("streaming")->getValue<bool>();

        if (streaming)
        {
            emitter.setSource(path);
        }
        else
        {
            emitter.setSource(m_audioResource.get(path));
        }

        const auto& properties = cfg->second.getProperties();
        for (const auto& prop : properties)
        {
            auto propertyName = prop.getName();
            if (propertyName == "pitch")
            {
                emitter.setPitch(prop.getValue<float>());
            }
            else if (propertyName == "volume")
            {
                emitter.setVolume(prop.getValue<float>());
            }
            else if (propertyName == "relative_listener")
            {
                emitter.setRelativeTolistener(prop.getValue<bool>());
            }
            else if (propertyName == "min_distance")
            {
                emitter.setMinDistance(prop.getValue<float>());
            }
            else if (propertyName == "attenuation")
            {
                emitter.setAttenuation(prop.getValue<float>());
            }
            else if (propertyName == "looped")
            {
                emitter.setLooped(prop.getValue<bool>());
            }
            else if (propertyName == "mixer_channel")
            {
                auto channel = prop.getValue<sf::Int32>();
                if (channel > -1 && channel < AudioMixer::MaxChannels)
                {
                    emitter.setChannel(static_cast<sf::Uint8>(channel));
                }
            }
        }
        emitter.applyMixerSettings(); //makes sure volume is applied immediately to prevent loud bursts
    }
    return emitter;
}

void AudioScape::addEmitter(const std::string& name, const AudioEmitter& emitter)
{
    Logger::log("Adding emitters is currently not implemented until file paths are retrievable", xy::Logger::Type::Error);
    return;


    ConfigObject obj("emitter", name);
    obj.addProperty("path", "buns. we need to put the path to the buffer file here");
    obj.addProperty("streaming", emitter.isStreaming() ? "true" : "false");
    obj.addProperty("pitch", std::to_string(emitter.getPitch()));
    obj.addProperty("volume", std::to_string(emitter.getVolume()));
    obj.addProperty("relative_listener", emitter.isRelativeToListener() ? "true" : "false");
    obj.addProperty("min_distance", std::to_string(emitter.getMinDistance()));
    obj.addProperty("attenuation", std::to_string(emitter.getAttenuation()));
    obj.addProperty("looped", emitter.isLooped() ? "true" : "false");
    obj.addProperty("mixer_channel", std::to_string(emitter.getChannel()));

    m_emitterConfigs[name] = obj;
}
