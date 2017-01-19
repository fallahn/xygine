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

#ifndef XY_RENDER_QUAD_HPP_
#define XY_RENDER_QUAD_HPP_

#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/VertexAttribBinding.hpp>
#include <xygine/mesh/RenderPass.hpp>

#include <SFML/Graphics/Drawable.hpp>

namespace sf
{
    class Shader;
}

namespace xy
{
    /*
    \brief Drawable textured quad.
    Used internally by the mesh model renderer, it is recommended
    for other uses that standard SFML drawables be used.
    */
    class RenderQuad final : public sf::Drawable
    {
    public:
        RenderQuad(const sf::Vector2f&, sf::Shader&);
        ~RenderQuad() = default;

        sf::Shader& getShader() const;
        
        void addRenderPass(RenderPass::ID, sf::Shader&);
        void setActivePass(RenderPass::ID);

    private:
        xy::Mesh m_mesh;
        xy::Material m_material;
        xy::VertexAttribBinding m_vao;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //XY_RENDER_QUAD_HPP_