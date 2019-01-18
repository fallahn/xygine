/*********************************************************************
(c) Matt Marchant 2017 - 2019
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#pragma once

#include "xyginext/graphics/postprocess/PostProcess.hpp"

#include <SFML/Graphics/Shader.hpp>

namespace xy
{
    /*!
    \brief Antique Post process effect.
    Creates an old 'filmic' look, with dust, jitter and desaturation.
    */
    class XY_EXPORT_API PostAntique final : public xy::PostProcess
    {
    public:
        PostAntique();
        //~PostAntique();

        void apply(const sf::RenderTexture&, sf::RenderTarget&) override;
        void update(float) override;

        /*!
        \brief Allows updating shader parameters.
        Available paramters are:
        u_tone sf::Glsl::vec3 representing normalise RGB values to tint the image
        u_toneMix float 0 - 1 mix between untinted and tone colour
        u_vignetteRadius float 0 - 1 range affecting the radius of the non-vignetted area
        u_vignetteSoftness float 0 - 1 range feathering the edge of the vignette
        u_vignetteAmount float 0 -1 range affecting the vignette mix. 0 is no vignette, 1 is hard black
        u_flickerAmount float 0 - 1 affects the frequency of flicker, 0 is no flicker
        u_jitterAmount float 0 - 1 affects the offset of jitter, 0 is no jitter
        u_noiseAmount float 0 - 1 increases the amount of visible noise
        */
        template <typename T>
        void setUniform(const std::string& name, T value)
        {
            m_shader.setUniform(name, value);
        }

    private:
        sf::Shader m_shader;
    };
}
