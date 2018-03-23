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

#include "xyginext/graphics/postprocess/ChromeAb.hpp"
#include "xyginext/core/App.hpp"

#include "../../imgui/imgui.h"

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

namespace
{
    float accumulatedTime = 0.f;
    const float scanlineCount = 500.f;

    float distortionAmount = 0.01f;

#include "PostChromeAb.inl"
}

using namespace xy;

PostChromeAb::PostChromeAb(bool distort)
{
    if (distort)
    {
        m_shader.loadFromMemory(CHRAB_DISTORT, sf::Shader::Fragment);
    }
    else
    {
        m_shader.loadFromMemory(CHRAB_NO_DISTORT, sf::Shader::Fragment);
    }

#ifdef XY_DEBUG

    //xy::App::addUserWindow(
    //    [this]()
    //{
    //    nim::Begin("Abberation Params");
    //    nim::SliderFloat("Distortion", &distortionAmount, 0.f, 0.1f);
    //    nim::End();
    //}, this);

#endif //XY_DEBUG
}

PostChromeAb::~PostChromeAb()
{
    //xy::App::removeUserWindows(this);
}

//public
void PostChromeAb::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    float windowRatio = static_cast<float>(dst.getSize().y) / static_cast<float>(src.getSize().y);

    m_shader.setUniform("u_sourceTexture", src.getTexture());
    m_shader.setUniform("u_time", accumulatedTime * (10.f * windowRatio));
    m_shader.setUniform("u_lineCount", windowRatio  * scanlineCount);

    applyShader(m_shader, dst);
}

void PostChromeAb::update(float dt)
{
    accumulatedTime += dt;

#ifdef XY_DEBUG
    m_shader.setUniform("u_distortStrength", distortionAmount);
#endif //XY_DEBUG
}
