/*********************************************************************
(c) Matt Marchant 2017
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

#include "StateIDs.hpp"

#include <xyginext/core/State.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace xy
{
    class MessageBus;
}

class IntroState final : public xy::State
{
public:
    IntroState(xy::StateStack&, Context);
    ~IntroState() = default;

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override {}
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const override { return StateID::Intro; }

private:

    sf::Texture m_texture;
    sf::Sprite m_sprite;
    sf::RectangleShape m_rectangleShape;
    sf::Shader m_noiseShader;
    sf::Shader m_lineShader;
    float m_windowRatio;

    xy::MessageBus& m_messageBus;

    float m_fadeTime;
    enum class Fade
    {
        In,
        Hold,
        Out
    }m_fade;

    sf::SoundBuffer m_soundBuffer;
    sf::Sound m_sound;
};
