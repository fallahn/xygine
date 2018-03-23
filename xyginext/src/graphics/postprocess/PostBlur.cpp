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

#include "xyginext/graphics/postprocess/Blur.hpp"
#include "xyginext/core/Assert.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace xy;

namespace
{
    const std::string fragShader =
        R"(
        #version 120
        
        uniform sampler2D u_srcTexture;

        void main()
        {
            gl_FragColor = texture2D(u_srcTexture, gl_TexCoord[0].xy);
        })";

#include "DefaultVertex.inl"
#include "PostGaussianBlur.inl"
#include "PostDownSample.inl"
}

PostBlur::PostBlur()
    : m_amount  (0.f),
    m_enabled   (false),
    m_fadeSpeed (5.f)
{
    m_blurShader.loadFromMemory(Default::vertex, PostGaussianBlur::fragment);
    m_downsampleShader.loadFromMemory(Default::vertex, PostDownSample::fragment);
    m_outShader.loadFromMemory(Default::vertex, fragShader);

    initTextures(sf::Vector2u(DefaultSceneSize));
}

//public
void PostBlur::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    //fade in / out
    if (m_enabled)
    {
        m_amount = std::min(1.f, m_amount + (m_clock.restart().asSeconds() * m_fadeSpeed));
    }
    else
    {
        m_amount = std::max(0.f, m_amount - (m_clock.restart().asSeconds() * m_fadeSpeed));
    }


    //and draw....
    if (m_amount == 0)
    {
        m_outShader.setUniform("u_srcTexture", src.getTexture());
        applyShader(m_outShader, dst);
    }
    else
    {
        downSample(src, m_firstPassTextures[0]);
        blurMultipass(m_firstPassTextures);
        downSample(m_firstPassTextures[0], m_secondPassTextures[0]);
        blurMultipass(m_secondPassTextures);
        m_outShader.setUniform("u_srcTexture", m_secondPassTextures[0].getTexture());

    }
    applyShader(m_outShader, dst);
}

void PostBlur::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

void PostBlur::setFadeSpeed(float speed)
{
    XY_ASSERT(speed > 0, "Speed must be great than 0");
    m_fadeSpeed = speed;
}

//private
void PostBlur::initTextures(sf::Vector2u size)
{
    size /= 2u;
    if (m_firstPassTextures[0].getSize() != size)
    {
        m_firstPassTextures[0].create(size.x, size.y);
        m_firstPassTextures[0].setSmooth(true);

        m_firstPassTextures[1].create(size.x, size.y);
        m_firstPassTextures[1].setSmooth(true);

        m_secondPassTextures[0].create(size.x / 2, size.y / 2);
        m_secondPassTextures[0].setSmooth(true);

        m_secondPassTextures[1].create(size.x / 2, size.y / 2);
        m_secondPassTextures[1].setSmooth(true);
    }
}

void PostBlur::blurMultipass(TexturePair& textures)
{
    auto textureSize = textures[0].getSize();
    for (auto i = 0u; i < 2; ++i)
    {
        blur(textures[0], textures[1], { 0.f, 1.f / static_cast<float>(textureSize.y) });
        blur(textures[1], textures[0], { 1.f / static_cast<float>(textureSize.x), 0.f });
    }
}

void PostBlur::blur(const sf::RenderTexture& src, sf::RenderTexture& dst, const sf::Vector2f& offset)
{
    m_blurShader.setUniform("u_sourceTexture", src.getTexture());
    m_blurShader.setUniform("u_offset", offset * m_amount);

    applyShader(m_blurShader, dst);
    dst.display();
}

void PostBlur::downSample(const sf::RenderTexture& src, sf::RenderTexture& dst)
{
    m_downsampleShader.setUniform("u_sourceTexture", src.getTexture());
    m_downsampleShader.setUniform("u_sourceSize", sf::Vector2f(src.getSize()));

    applyShader(m_downsampleShader, dst);
    dst.display();
}
