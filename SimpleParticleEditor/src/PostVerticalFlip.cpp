/*********************************************************************
(c) Matt Marchant 2017 - 2021
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

#include "PostVerticalFlip.hpp"

#include <SFML/Graphics/RenderTexture.hpp>

#include <string>

namespace
{
    const std::string vertex = 
    R"(
    #version 120
    void main()
    {
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
        gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

        gl_TexCoord[0].y = 1.0 - gl_TexCoord[0].y;

        gl_FrontColor = gl_Color;
    })";

    const std::string fragment = 
    R"(
    #version 120

    uniform sampler2D u_texture;

    void main()
    {
        gl_FragColor = texture2D(u_texture, gl_TexCoord[0].xy);
    })";
}

PostVerticalFlip::PostVerticalFlip()
{
    m_shader.loadFromMemory(vertex, fragment);
}

void PostVerticalFlip::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_texture", src.getTexture());
    applyShader(m_shader, dst);
}