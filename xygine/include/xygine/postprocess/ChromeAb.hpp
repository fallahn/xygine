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

//simulates chromatic aberration

#ifndef XY_POST_CHROMEAB_HPP_
#define XY_POST_CHROMEAB_HPP_

#include <xygine/postprocess/PostProcess.hpp>
#include <xygine/ShaderResource.hpp>

namespace xy
{
    /*!
    \brief Creates a Chromatic Abberation effect

    Chromatic abberation is the apparent split in colours often
    noticed near the edges of powerful lenses. The effect also adds
    noise and scanlines to try and recreate the overall effect of
    and old CRT type monitor
    */
    class XY_EXPORT_API PostChromeAb final : public PostProcess
    {
    public:
        /*!
        \brief Constructor.
        \param distort If set to true the post effect will apply a
        CRT style bowed distortion around the edge of the screen
        */
        PostChromeAb(bool distort = false);
        ~PostChromeAb();
        /*!
        \see PostProcess
        */
        void apply(const sf::RenderTexture&, sf::RenderTarget&) override;
        /*!
        \see PostProcess
        */
        void update(float) override;

    private:
        ShaderResource m_shaderResource;
    };
}
#endif //XY_POST_CHROMEAB_HPP_