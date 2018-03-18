/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#include <xyginext/graphics/postprocess/Antique.hpp>
#include <xyginext/core/App.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

using namespace xy;

namespace
{
    //TODO add contrast control?
    const std::string fragment =
        R"(
        #version 130

        uniform sampler2D u_srcTexture;
        uniform float u_time = 0.0;

        uniform vec3 u_tone = vec3(1.0, 0.9, 0.65);
        uniform float u_toneMix = 0.95;

        uniform float u_vignetteRadius = 0.5;
        uniform float u_vignetteSoftness = 0.45;
        uniform float u_vignetteAmount = 0.5;

        uniform float u_flickerAmount = 0.05;
        uniform float u_jitterAmount = 0.01;
        uniform float u_noiseAmount = 1.0;

        float rnd(vec2 seed)
        {
            return fract(sin(dot(seed.xy ,vec2(12.9898,78.233))) * 43758.5453);
        }

        void main()
        {
            float random = rnd(vec2(u_time));
            vec2 jitter = clamp(vec2(random), 0.0, u_jitterAmount);

            vec3 colour = texture2D(u_srcTexture, gl_TexCoord[0].xy + jitter).rgb;

            float x = (gl_TexCoord[0].x + 4.0) * gl_TexCoord[0].y * u_time * 10.0;
            x = mod(x, 13.0) * mod(x, 123.0);
            float grain = mod(x, 0.01) - 0.005;
            colour += vec3(clamp(grain * 100.0, 0.0, 0.07)) * u_noiseAmount;

            float grey = dot(colour, vec3(0.299, 0.587, 0.114));
            vec3 graded = vec3(grey) * u_tone;
            graded = mix(colour, graded, u_toneMix);

            float linePos = fract(gl_TexCoord[0].x * 0.2);
            if(linePos > 0.9) graded *= 0.0;

            float len = length(gl_TexCoord[0].xy - vec2(0.5));
            vec3 vignette = vec3(smoothstep(u_vignetteRadius, u_vignetteRadius - u_vignetteSoftness, len));
            graded = mix(graded, graded * vignette, u_vignetteAmount);

            graded = clamp(graded + (vec3(random) * u_flickerAmount), 0.0, 1.0);

            gl_FragColor.rgb = graded;
        })";

    float elapsedTime = 0.f;

#ifdef XY_DEBUG
    float toneMix = 0.95f;

    float vignetteRadius = 0.5f;
    float vignetteSoftness = 0.4f;
    float vignetteAmount = 0.5f;

    float flickerAmount = 0.05f;
    float jitterAmount = 0.01f;
    float noiseAmount = 1.f;
#endif //XY_DEBUG
}

PostAntique::PostAntique()
{
    if (!m_shader.loadFromMemory(fragment, sf::Shader::Fragment))
    {
        xy::Logger::log("Failed to create Antique post shader", xy::Logger::Type::Error);
    }

#ifdef XY_DEBUG
    //xy::App::addUserWindow(
    //    [this]()
    //{
    //    nim::Begin("Shader Params");
    //    nim::DragFloat("Tone Amount", &toneMix, 0.01f, 0.f, 1.f);
    //    nim::DragFloat("Vignette Rad", &vignetteRadius, 0.01f, 0.f, 0.75f);
    //    nim::DragFloat("Vignette Amount", &vignetteAmount, 0.01f, 0.f, 1.f);
    //    nim::DragFloat("Flicker Amount", &flickerAmount, 0.001f, 0.f, 0.09f);
    //    nim::DragFloat("Jitter Amount", &jitterAmount, 0.001f, 0.f, 0.1f);
    //    nim::DragFloat("Noise Amount", &noiseAmount, 0.01f, 0.f, 2.f);
    //    nim::End();
    //}, this);
#endif //XY_DEBUG
}

PostAntique::~PostAntique()
{
    //xy::App::removeUserWindows(this);
}

//public
void PostAntique::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_srcTexture", src.getTexture());
    m_shader.setUniform("u_destSize", sf::Glsl::Vec2(dst.getSize()));
    applyShader(m_shader, dst);
}

void PostAntique::update(float dt)
{
    elapsedTime += dt;
    m_shader.setUniform("u_time", elapsedTime);

#ifdef XY_DEBUG

    m_shader.setUniform("u_toneMix", toneMix);
    m_shader.setUniform("u_vignetteRadius", vignetteRadius);
    m_shader.setUniform("u_vignetteAmount", vignetteAmount);
    m_shader.setUniform("u_flickerAmount", flickerAmount);
    m_shader.setUniform("u_jitterAmount", jitterAmount);
    m_shader.setUniform("u_noiseAmount", noiseAmount);

#endif //XY_DEBUG
}
