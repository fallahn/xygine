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

#include "KeyMapDirector.hpp"
#include "MessageIDs.hpp"
#include "CommandIDs.hpp"
#include "KeyMapping.hpp"
#include "ClientServerShared.hpp"

#include <xyginext/ecs/Scene.hpp>
#include <xyginext/ecs/systems/UISystem.hpp>
#include <xyginext/ecs/components/Text.hpp>

#include <xyginext/core/ConfigFile.hpp>

#include <SFML/Window/Event.hpp>

KeyMapDirector::KeyMapDirector(SharedStateData& ssd, xy::ConfigFile& cfg)
    : m_sharedData(ssd),
    m_config(cfg)
{
    m_cfgName = xy::FileSystem::getConfigDirectory(dataDir);
    m_cfgName += "keybinds.cfg";
}


//public
void KeyMapDirector::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        if (m_activeInput.player > -1)
        {
            //update config file
            xy::ConfigObject* cfg = nullptr;
            if (m_activeInput.player == 0)
            {
                cfg = m_config.findObjectWithName("player_one");
            }
            else
            {
                cfg = m_config.findObjectWithName("player_two");
            }
            XY_ASSERT(cfg, "Config file isntance is not valid");            
            
            
            //update keybind
            auto& kb = m_sharedData.inputBindings[m_activeInput.player];
            switch (m_activeInput.index)
            {
            default: break;
            case 0:
                kb.buttons[InputBinding::Jump] = evt.key.code;
                cfg->findProperty("jump")->setValue(std::to_string(evt.key.code));
                break;
            case 1:
                kb.buttons[InputBinding::Shoot] = evt.key.code;
                cfg->findProperty("shoot")->setValue(std::to_string(evt.key.code));
                break;
            case 2:
                kb.buttons[InputBinding::Left] = evt.key.code;
                cfg->findProperty("left")->setValue(std::to_string(evt.key.code));
                break;
            case 3:
                kb.buttons[InputBinding::Right] = evt.key.code;
                cfg->findProperty("right")->setValue(std::to_string(evt.key.code));
                break;
            }
            m_config.save(m_cfgName);

            //update string
            auto key = evt.key.code;
            auto activeInput = m_activeInput;

            xy::Command cmd;
            cmd.targetFlags = CommandID::KeybindInput;
            cmd.action = [&, key, activeInput](xy::Entity entity, float)
            {
                const auto& km = entity.getComponent<KeyMapInput>();
                if (km.player == activeInput.player && km.index == activeInput.index)
                {
                    entity.getComponent<xy::Text>().setString(KeyMap.at(key));
                }
            };
            sendCommand(cmd);

            //reenable UI
            getScene().setSystemActive<xy::UISystem>(true);

            m_activeInput.player = -1;
        }
    }
}

void KeyMapDirector::handleMessage(const xy::Message& msg)
{
    if (msg.id == MessageID::MenuMessage)
    {
        const auto& data = msg.getData<MenuEvent>();
        if (data.action == MenuEvent::KeybindClicked)
        {
            xy::Command cmd;
            cmd.targetFlags = CommandID::KeybindInput;
            cmd.action = [&, data](xy::Entity entity, float)
            {
                const auto& kb = entity.getComponent<KeyMapInput>();
                if (kb.player == data.player && kb.index == data.index)
                {
                    //disable the input system
                    getScene().setSystemActive<xy::UISystem>(false);

                    //update text
                    entity.getComponent<xy::Text>().setString("Press a Key");

                    //and set input to active
                    m_activeInput.player = data.player;
                    m_activeInput.index = data.index;
                }
            };
            sendCommand(cmd);
        }
    }
}

void KeyMapDirector::process(float)
{

}