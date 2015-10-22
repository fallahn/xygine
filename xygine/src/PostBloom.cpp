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

#include <xygine/PostBloom.hpp>

using namespace xy;

PostBloom::PostBloom()
{
    m_shaderResource.preload(Shader::Type::PostAdditiveBlend, Shader::FullPass::vertex, Shader::PostAdditiveBlend::fragment);
    m_shaderResource.preload(Shader::Type::PostBrightnessExtract, Shader::FullPass::vertex, Shader::PostBrightness::fragment);
    m_shaderResource.preload(Shader::Type::PostDownSample, Shader::FullPass::vertex, Shader::PostDownSample::fragment);
    m_shaderResource.preload(Shader::Type::PostGaussianBlur, Shader::FullPass::vertex, Shader::PostGaussianBlur::fragment);
}

//public
void PostBloom::apply(const sf::RenderTexture& src, sf::RenderTarget& dest)
{
    initTextures(src.getSize());

    filterBright(src, m_brightnessTexture);

    downSample(m_brightnessTexture, m_firstPassTextures[0]);

    blurMultipass(m_firstPassTextures);

    downSample(m_firstPassTextures[0], m_secondPassTextures[0]);

    blurMultipass(m_secondPassTextures);

    add(m_firstPassTextures[0], m_secondPassTextures[0], m_firstPassTextures[1]);
    m_firstPassTextures[1].display();

    add(src, m_firstPassTextures[1], dest);
}

//private
void PostBloom::initTextures(const sf::Vector2u& size)
{
    if (m_brightnessTexture.getSize() != size)
    {
        m_brightnessTexture.create(size.x, size.y);
        m_brightnessTexture.setSmooth(true);

        m_firstPassTextures[0].create(size.x / 2u, size.y / 2u);
        m_firstPassTextures[0].setSmooth(true);
        m_firstPassTextures[1].create(size.x / 2u, size.y / 2u);
        m_firstPassTextures[1].setSmooth(true);

        m_secondPassTextures[0].create(size.x / 4u, size.y / 4u);
        m_secondPassTextures[0].setSmooth(true);
        m_secondPassTextures[1].create(size.x / 4u, size.y / 4u);
        m_secondPassTextures[1].setSmooth(true);
    }
}

void PostBloom::filterBright(const sf::RenderTexture& src, sf::RenderTexture& dst)
{
    auto& shader = m_shaderResource.get(Shader::Type::PostBrightnessExtract);

    shader.setParameter("u_sourceTexture", src.getTexture());
    applyShader(shader, dst);
    dst.display();
}

void PostBloom::blurMultipass(RenderTextureArray& textures)
{
    auto textureSize = textures[0].getSize();
    for (auto i = 0u; i < 2; ++i)
    {
        blur(textures[0], textures[1], { 0.f, 1.f / static_cast<float>(textureSize.y) });
        blur(textures[1], textures[0], { 1.f / static_cast<float>(textureSize.x), 0.f });
    }
}

void PostBloom::blur(const sf::RenderTexture& src, sf::RenderTexture& dst, const sf::Vector2f& offset)
{
    auto& shader = m_shaderResource.get(Shader::Type::PostGaussianBlur);

    shader.setParameter("u_sourceTexture", src.getTexture());
    shader.setParameter("u_offset", offset);

    applyShader(shader, dst);
    dst.display();
}

void PostBloom::downSample(const sf::RenderTexture& src, sf::RenderTexture& dst)
{
    auto& shader = m_shaderResource.get(Shader::Type::PostDownSample);

    shader.setParameter("u_sourceTexture", src.getTexture());
    shader.setParameter("u_sourceSize", sf::Vector2f(src.getSize()));

    applyShader(shader, dst);
    dst.display();
}

void PostBloom::add(const sf::RenderTexture& src, const sf::RenderTexture& bloom, sf::RenderTarget& dst)
{
    auto& shader = m_shaderResource.get(Shader::Type::PostAdditiveBlend);

    shader.setParameter("u_sourceTexture", src.getTexture());
    shader.setParameter("u_bloomTexture", bloom.getTexture());
    applyShader(shader, dst);
}