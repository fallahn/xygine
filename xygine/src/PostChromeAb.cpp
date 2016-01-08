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

#include <xygine/PostChromeAb.hpp>
#include <xygine/Shaders.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

namespace
{
    float accumulatedTime = 0.f;
    const float scanlineCount = 5500.f;
}

using namespace xy;

PostChromeAb::PostChromeAb()
{
    m_shaderResource.preload(Shader::Type::ChromeAb, Shader::FullPass::vertex, Shader::PostChromeAb::fragment);
}

//public
void PostChromeAb::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    float windowRatio = static_cast<float>(dst.getSize().y) / static_cast<float>(src.getSize().y);

    auto& shader = m_shaderResource.get(Shader::Type::ChromeAb);
    shader.setParameter("u_sourceTexture", src.getTexture());
    shader.setParameter("u_time", accumulatedTime * (10.f * windowRatio));
    shader.setParameter("u_lineCount", windowRatio  * scanlineCount);

    applyShader(shader, dst);
}

void PostChromeAb::update(float dt)
{
    accumulatedTime += dt;
}

PostProcess::Ptr PostChromeAb::create()
{
    return std::move(std::make_unique<PostChromeAb>());
}