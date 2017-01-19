/*********************************************************************
© Matt Marchant 2014 - 2017
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

//controls animations of entities composed of animated drawables

#ifndef XY_ANIMATION_CONTROLLER_HP__
#define XY_ANIMATION_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    class AnimatedDrawable;
    /*!
    \brief Animation Controller Component

    When attached to an entity which also has an Animated Drawable
    component attached the animation controller can be used to listen
    for events and play specific animations accordingly.
    */
    class XY_EXPORT_API AnimationController final : public Component
    {
    public:
        using Ptr = std::unique_ptr<AnimationController>;

        explicit AnimationController(MessageBus&);
        ~AnimationController() = default;

        Component::Type type() const override;
        void entityUpdate(Entity&, float) override;

        void onStart(Entity&) override;

    private:

        AnimatedDrawable* m_drawable;
        enum AnimationID //TODO we need to ensure these are properly mapped to loaded animations
        {
            MoveRight,
            MoveLeft,
            MoveUp,
            MoveDown
        };
    };
}

#endif //XY_ANIMATION_CONTROLLER_HPP_