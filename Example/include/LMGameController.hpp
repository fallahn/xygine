/*********************************************************************
Matt Marchant 2014 - 2016
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

#ifndef LM_GAME_CONTROLLER_HPP_
#define LM_GAME_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/Scene.hpp>

#include <list>

namespace lm
{
    class PlayerController;
    class GameController final : public xy::Component
    {
    public:
        GameController(xy::MessageBus&, xy::Scene&);
        ~GameController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void setInput(sf::Uint8);
        

    private:
        xy::Scene& m_scene;

        sf::Uint8 m_inputFlags;

        bool m_spawnReady;
        PlayerController* m_player;
        void spawnPlayer();

        xy::Entity* m_mothership;
        void createMothership();

        struct DelayedEvent
        {
            float time = 0.f;
            std::function<void()> action;
        };
        std::list<DelayedEvent> m_delayedEvents;
    };
}

#endif //LM_GAME_CONTROLLER_HPP_