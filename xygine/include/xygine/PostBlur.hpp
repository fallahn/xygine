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

#ifndef XY_POST_BLUR_HPP_
#define XY_POST_BLUR_HPP_

#include <xygine/PostProcess.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Clock.hpp>

#include <array>

namespace xy
{
    /*!
    \brief Post process effect which applies a screen-wide gaussian blur.
    This effect can be enabled and disabled as needed via direct call to
    setEnabled() or via message callbacks. Blur amount is animated when
    enabling or disabling for a smoother transition.
    */
    class PostBlur final : public PostProcess
    {
    public:
        PostBlur();
        ~PostBlur() = default;

        /*!
        \brief Applies the effect to the screen
        */
        void apply(const sf::RenderTexture&, sf::RenderTarget&) override;
        /*!
        \brief Enables or disables the blur effect.
        */
        void setEnabled(bool);
        /*!
        \brief Sets the speed of the transition when enableing or disabling
        the effect.
        \param Value must be 1 or greater
        */
        void setFadeSpeed(float value);

    private:

        float m_amount;
        sf::Clock m_clock;

        bool m_enabled;
        float m_fadeSpeed;
        sf::Shader m_blurShader;
        sf::Shader m_downsampleShader;
        sf::Shader m_outShader;

        using TexturePair = std::array<sf::RenderTexture, 2u>;
        TexturePair m_firstPassTextures;
        TexturePair m_secondPassTextures;

        void initTextures(sf::Vector2u);
        void blurMultipass(TexturePair&);
        void blur(const sf::RenderTexture&, sf::RenderTexture&, const sf::Vector2f&);
        void downSample(const sf::RenderTexture&, sf::RenderTexture&);
    };
}

#endif //XY_POST_BLUR_HPP_