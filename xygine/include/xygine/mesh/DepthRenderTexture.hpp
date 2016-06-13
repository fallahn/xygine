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

#ifndef XY_DEPTH_RENDER_TEXTURE_HPP_
#define XY_DEPTH_RENDER_TEXTURE_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/GlResource.hpp>
#include <SFML/Window/Context.hpp>

#include <memory>

namespace xy
{
    /*!
    \brief An off-screen render target with a single texture attached to the depth buffer.

    Used internally by the MeshRenderer class for depth mapping the SFML compatible render
    target has only a single texture attached to the depth buffer. This is useful for techniques
    such as shadow mapping when depth values need comparing.
    */
    class XY_EXPORT_API DepthRenderTexture final : public sf::RenderTarget, public sf::GlResource
    {
    public:
        /*!
        \brief Constructor.
        The DepthRenderTexture is in an invalid state until create() is called.
        */
        DepthRenderTexture();
        ~DepthRenderTexture();

        /*!
        \brief (Re)creates the DepthRenderTexture.
        \returns true if successful, else false.
        */
        bool create(sf::Uint32 width, sf::Uint32 height);

        /*!
        \brief Activate or deactivate the depth texture for rendering.
        \returns true if successful, else false
        */
        bool setActive(bool = true);

        /*!
        \brief Update the contents of the depth texture once rendering 
        is completed. As any sf::RenderTarget this must be called each
        time rendering is finished to leave the buffer in a valid state.
        */
        void display();

        /*!
        \brief Returns the size of the render target.
        */
        sf::Vector2u getSize() const override;

        /*!
        \brief Returns a reference to the texture attached to the buffer.
        */
        const sf::Texture& getTexture() const;

    private:

        std::unique_ptr<sf::Context> m_context;
        unsigned int m_fbo;
        sf::Texture m_texture;

        bool activate(bool) override;
    };
}

#endif //XY_DEPTH_RENDER_TEXTURE_HPP_