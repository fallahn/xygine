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

#include <xyginext/audio/Mixer.hpp>
#include <xyginext/util/Math.hpp>
#include <xyginext/core/Assert.hpp>

using namespace xy;

namespace
{
    
}
std::array<std::string, AudioMixer::MaxChannels> AudioMixer::m_labels
{{
    "Channel 0", 
    "Channel 1", 
    "Channel 2", 
    "Channel 3", 
    "Channel 4", 
    "Channel 5", 
    "Channel 6", 
    "Channel 7", 
    "Channel 8", 
    "Channel 9", 
    "Channel 10", 
    "Channel 11", 
    "Channel 12", 
    "Channel 13", 
    "Channel 14", 
    "Channel 15"
}};

std::array<float, AudioMixer::MaxChannels> AudioMixer::m_channels
{{ 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f }};

float AudioMixer::m_masterVol = 1.f;

void AudioMixer::setMasterVolume(float vol)
{
    AudioMixer::m_masterVol = Util::Math::clamp(vol, 0.f, 10.f);
}

float AudioMixer::getMasterVolume()
{
    return AudioMixer::m_masterVol;
}

void AudioMixer::setVolume(float vol, sf::Uint8 channel)
{
    XY_ASSERT(channel < MaxChannels, "Channel index out of range");
    AudioMixer::m_channels[channel] = Util::Math::clamp(vol, 0.f, 10.f);
}

float AudioMixer::getVolume(sf::Uint8 channel)
{
    XY_ASSERT(channel < MaxChannels, "Channel index out of range");
    return AudioMixer::m_channels[channel];
}

void AudioMixer::setLabel(const std::string& label, sf::Uint8 channel)
{
    XY_ASSERT(channel < MaxChannels, "Channel index out of range");
    m_labels[channel] = label;
}

const std::string& AudioMixer::getLabel(sf::Uint8 channel)
{
    XY_ASSERT(channel < MaxChannels, "Channel index out of range");
    return m_labels[channel];
}
