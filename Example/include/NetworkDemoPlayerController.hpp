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

#ifndef NET_DEMO_PLAYER_CONTROLLER_HPP_
#define NET_DEMO_PLAYER_CONTROLLER_HPP_

#include <NetworkDemoPlayerInput.hpp>

#include <xygine/components/Component.hpp>

#include <queue>
#include <list>

namespace NetDemo
{
    class PlayerController final : public xy::Component
    {
    public:
        explicit PlayerController(xy::MessageBus&);
        ~PlayerController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void setInput(const Input&, bool = true);
        void reconcile(float position, sf::Uint64);

    private:

        Input m_currentInput;
        sf::Uint64 m_lastInputId;
        std::queue<Input> m_inputBuffer;
        std::list<Input> m_reconcileInputs;

        xy::Entity* m_entity;
        float m_velocity; //TODO probably don't need this
        float m_lastPosition; //and therefore not this either

        void parseCurrentInput();
    };
}

#endif //NET_DEMO_PLAYER_CONTROLLER_HPP_