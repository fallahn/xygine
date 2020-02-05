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

#include "../Config.hpp"
#include "../ecs/components/AudioEmitter.hpp"
#include "../core/ConfigFile.hpp"

#include <string>
#include <unordered_map>

namespace xy
{
    class AudioResource;

    /*!
    \brief Contains one or more definitions used to create AudioEmitter components.
    Similarly to the SpriteSheet class used for loading Sprites from configuration
    files, AudioScapes can be used to define the settings used to create AudioEmitter
    components. AudioScape files usually have the extension *.xas with the following
    ConfigFile compatible layout:
    \code
    audio_scape NAME
    {
        emitter NAME_ONE
        {
            path = "STR_PATH_TO_FILE"
            streaming = BOOL
            pitch = FLOAT
            volume = FLOAT
            relative_listener = BOOL
            min_distance = FLOAT
            attenuation = FLOAT
            looped = BOOL
            mixer_channel = INT
        }

        emitter NAME_TWO
        {
            path = "STR_PATH_TO_FILE"
            streaming = BOOL
            pitch = FLOAT
            volume = FLOAT
            relative_listener = BOOL
            min_distance = FLOAT
            attenuation = FLOAT
            looped = BOOL
            mixer_channel = INT
        }
    }
    \endcode
    Emitter properties are optional (except path and streaming) and any omitted properties
    will fall back to their default values.
    */
    class XY_EXPORT_API AudioScape final
    {
    public:
        /*!
        \brief Constructor
        \param rx Reference to an AudioResource object used to cache
        any non-streamed audio files.
        */
        explicit AudioScape(AudioResource& rx);

        /*!
        \brief Attempts to load an AudioScape file from the given path
        \param path String containing the path to the *.xas file to load
        \returns bool true if successful else false
        */
        bool loadFromFile(const std::string& path);

        /*!
        \brief Saves the AudioScape configuration to a given path.
        AudioScape files normally have the extension *.xas
        */
        bool saveToFile(const std::string& path);

        /*!
        \brief Returns an AudioEmitter configured with the given name from
        the loaded confugration, if it exists, else returns an Uninitialised
        AudioEmitter. Use AudioEmitter::hasSource() to check validity of the
        returned emitter.
        */
        AudioEmitter getEmitter(const std::string& name) const;

        /*!
        \brief Adds an emitter configuration to the AudioScape.
        \param name Name of the AudioEmitter as it appears in the configuration.
        If an emitter with the name already exists then it will be overwritten.
        \param emitter An AudioEmitter whose current settings will be saved in the
        configuration file.
        */
        void addEmitter(const std::string& name, const AudioEmitter& emitter);

    private:
        AudioResource& m_audioResource;
        std::unordered_map<std::string, ConfigObject> m_emitterConfigs;
    };
} //namespace xy