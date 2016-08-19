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

#ifndef XY_MESH_DRAWABLE_HPP_
#define XY_MESH_DRAWABLE_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/Config.hpp>

#include <SFML/Graphics/Drawable.hpp>

namespace xy
{
    /*!
    \brief A drawable component used to draw the 3D scene belonging to a MeshRenderer.
    Although the MeshRenderer can be drawn independently it is sometimes desirable
    to draw the 3D world within a specific layer in a Scene. This component can be
    attached to a node within a scene graph for this purpose. It is generally not 
    recommended to have more than one of these components in a Scene from a performance
    perspective as it while cause the entire 3D scene to be drawn multiple times. In
    these cases it's better to draw the MeshRenderer directly to another buffer such as
    an sf::RenderTexture, and then use that texture for a sprite placed within the Scene.
    */
    class XY_EXPORT_API MeshDrawable final : public xy::Component, public sf::Drawable
    {
    public:
        MeshDrawable(MessageBus& mb, MeshRenderer& mr, const MeshRenderer::Lock&);
        ~MeshDrawable() = default;
        MeshDrawable(const MeshDrawable&) = delete;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(Entity&, float) override;
        sf::FloatRect globalBounds() const override { return m_sprite.getGlobalBounds(); }

    private:
        MeshRenderer& m_meshRenderer;

        mutable sf::RenderTexture m_renderTexture;
        sf::Sprite m_sprite;
        sf::Vector2f m_offset;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
    };
}

#endif //XY_MESH_DRAWABLE_HPP_