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

//drawable component compatible with sfml drawable objects for quick wrapping
//of objects such as shapes, sprites or text

#ifndef SF_DRAWABLE_COMPONENT_HPP_
#define SF_DRAWABLE_COMPONENT_HPP_

#include <xygine/Component.hpp>
#include <xygine/Entity.hpp>
#include <xygine/ShaderProperty.hpp>

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace xy
{
    template <class T>
    class SfDrawableComponent final : public Component, public sf::Transformable, public sf::Drawable, public ShaderProperty
    {
    public:
        explicit SfDrawableComponent(MessageBus& mb)
            : Component(mb, this)
        {
            static_assert(std::is_base_of<sf::Drawable, T>::value && std::is_base_of<sf::Transformable, T>::value, "must be sfml drawable and transformable");
        }
        ~SfDrawableComponent() = default;

        Component::Type type() const override { return Component::Type::Drawable; }
        void entityUpdate(Entity& entity, float) override
        {
            m_globalBounds = entity.getWorldTransform().transformRect(m_drawable.getGlobalBounds());
        }
        void handleMessage(const Message&) override {}

        T& getDrawable() { return m_drawable; }

        sf::FloatRect globalBounds() const override
        {
            return m_globalBounds;
        }

    private:
        T m_drawable;
        sf::FloatRect m_globalBounds;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            states.shader = getShader();
            rt.draw(m_drawable, states);
        }
    };
}
#endif //SF_DRAWABLE_COMPONENT_HPP_