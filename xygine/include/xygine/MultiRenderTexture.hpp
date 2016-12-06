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

#ifndef XY_MULTI_RENDER_TEXTURE_HPP_
#define XY_MULTI_RENDER_TEXTURE_HPP_

#include <xygine/Config.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/GlResource.hpp>
#include <SFML/Window/Context.hpp>

#include <array>
#include <memory>

namespace xy
{
    /*!
    \brief An off-screen render target which can draw on multiple textures at once

    An SFML-compatible render target, the MultiRenderTexture class allows a single
    draw call to draw to multiple textures at once. This is useful for cases such
    as deferred rendering where diffuse/normal/specular data for a scene may be drawn
    to separate textures before blending in a single shader.
    */
    class XY_EXPORT_API MultiRenderTexture final : public sf::RenderTarget, sf::GlResource
    {
    public:
        /*!
        \brief Constructor. The MRT is empty and invalid until create() is called
        */
        MultiRenderTexture();
        ~MultiRenderTexture();

        /*!
        \brief Creates the MultiRenderTexture.
        By default MRTs are empty and invalid and this function must first be called
        before the MRT can be used. Currently all of the textures created in the MRT
        have the same parameter, width, height, colour depth etc.
        \param width Width of the render target in pixels
        \param height Height of the render target in pixels
        \param count Number of textures to create. Currently the max supported is 4
        \param depthbuffer Set to true if you want to create a depth buffer with the
        target. This is false by default as it is the most common use case.
        \param depthTexture If this is true a depth texture is created (assuming depthbuffer
        is also true) which can be used for rendering depth values.
        \return True if creation is successful
        */
        bool create(sf::Uint32 width, sf::Uint32 height, sf::Uint32 count, bool depthbuffer = false, bool depthTexture = false);
        /*!\brief Prevents implicitly converting bool to int when inadvertantly missing out the count parameter*/
        template <typename T>
        bool create(sf::Uint32 width, sf::Uint32 height, T count, bool depthbuffer = false, bool depthTexture = false) = delete;

        /*!
        \brief Enables or disables texture smoothing. This is false by default.
        */
        void setSmooth(bool);

        /*!
        \brief Returns true if smoothing is enabled, else returns false.
        */
        bool isSmooth() const;

        /*!
        \brief Enable or disable texture repeating. This is false by default.
        */
        void setRepeated(bool);

        /*!
        \brief Returns true if texture repeating is enabled, else returns false.
        */
        bool isRepeated() const;

        /*!
        \brief Activate or deactivate the MRT for rendering.
        \return True if successful.
        */
        bool setActive(bool active = true);

        /*!
        \brief Update the contents of the MRT once drawing is completed.
        As sf::RenderWindow and sf::RenderTarget this must be called each
        time drawing has been completed to properly update the MRT.
        */
        void display();

        /*!
        \brief Returns the size of the MRT, as it was passed in create()
        */
        sf::Vector2u getSize() const override;

        /*!
        \brief Returns a read-only reference to the texture at the given index.
        Once the MRT has been drawn on and updated a texture can be retrieved
        and used with a sf::Sprite for example, or as a uniform parameter of a
        shader.
        \param index The index of the texture to retrieve. This must be lower
        than the number of textures requested when crea() was called.
        */
        const sf::Texture& getTexture(std::size_t index) const;

        /*!
        \brief returns the depth texture if this MRT was created with one
        */
        const sf::Texture& getDepthTexture() const { return m_depthTexture; }
    private:
        std::size_t m_textureCount;

        std::unique_ptr<sf::Context> m_context;
        unsigned int m_fbo;
        unsigned int m_depthbuffer;
        sf::Texture m_depthTexture;

        //TODO define a const for max textures
        std::array<sf::Texture, 4u> m_textures;
        bool activate(bool) override;
    };
}

#endif //XY_MULTI_RENDER_TEXTURE_HPP_