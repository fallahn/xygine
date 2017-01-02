/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef PLATFORM_PLAYER_HPP_
#define PLATFORM_PLAYER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    namespace Physics
    {
        class RigidBody;
    }
    class Model;
}

namespace Plat
{
    enum Input
    {
        Left = 0x1,
        Right = 0x2,
        Jump = 0x4
    };

    class PlayerController final : public xy::Component
    {
    public:
        explicit PlayerController(xy::MessageBus&);
        ~PlayerController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void applyInput(sf::Uint8);
    private:

        xy::Physics::RigidBody* m_body;
        xy::Model* m_model;
        sf::Uint8 m_lastInput;

        bool m_faceLeft;
        bool m_faceRight;
    };
}

#endif //PLATFORM_PLAYER_HPP_