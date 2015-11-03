/*********************************************************************
Matt Marchant 2014 - 2015
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

//drawable component compatible with sfml shape objects

#ifndef SHAPE_DRAWBLE_HPP_
#define SHAPE_DRAWABLE_HPP_

#include <xygine/Component.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace xy
{
    template<typename CONDITION>
    using enable_if = typename std::enable_if<CONDITION::value>::type;

    template <class T, enable_if<std::is_base_of<sf::Shape, T>>...>
    class ShapeDrawable final : public Component, public sf::Transformable, public sf::Drawable
    {
    public:
        explicit ShapeDrawable(MessageBus& mb)
            : Component(mb, this) {}
        ~ShapeDrawable() = default;

        Component::Type type() const override { return Component::Type::Drawable; }
        void entityUpdate(Entity&, float) override {}
        void handleMessage(const Message&) override {}

        T& getShape() { return m_shape; }


    private:
        T m_shape;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            rt.draw(m_shape, states);
        }
    };
}
#endif //SHAPE_DRAWABLE_HPP_