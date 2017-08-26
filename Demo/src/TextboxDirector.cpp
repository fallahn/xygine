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

#include "TextboxDirector.hpp"
#include "CommandIDs.hpp"

#include <xyginext/ecs/components/Text.hpp>
#include <xyginext/ecs/components/Transform.hpp>
#include <xyginext/ecs/systems/CommandSystem.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    const std::size_t maxBufferLength = 20;
}

TextboxDirector::TextboxDirector(SharedStateData& sharedData)
    : m_sharedData(sharedData)
{

}

//public
void TextboxDirector::handleEvent(const sf::Event& evt)
{
    auto updateText = [&]()
    {
        xy::Command cmd;
        cmd.targetFlags = CommandID::MenuText;
        cmd.action = [&](xy::Entity entity, float)
        {
            auto& text = entity.getComponent<xy::Text>();
            text.setString(m_sharedData.remoteIP);
        };
        sendCommand(cmd);
    };
    
    if (evt.type == sf::Event::TextEntered)
    {
        // Handle ASCII characters only, skipping backspace and delete
        if (evt.text.unicode > 31
            && evt.text.unicode < 127
            && m_sharedData.remoteIP.size() < maxBufferLength)
        {
            m_sharedData.remoteIP += static_cast<char>(evt.text.unicode);
            updateText();
        }
    }
    else if (evt.type == sf::Event::KeyReleased
        && evt.key.code == sf::Keyboard::BackSpace
        && !m_sharedData.remoteIP.empty())
    {
        m_sharedData.remoteIP.pop_back();
        updateText();
    }
}

void TextboxDirector::process(float dt)
{

}