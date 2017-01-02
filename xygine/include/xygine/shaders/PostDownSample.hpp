/*********************************************************************
Matt Marchant 2014 - 2017
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

#ifndef XY_SHADER_POSTDOWNSAMPLE_HPP_
#define XY_SHADER_POSTDOWNSAMPLE_HPP_

#include <xygine/shaders/Default.hpp>

namespace xy
{
    namespace Shader
    {
        //buns
        namespace PostDownSample
        {
            static const std::string fragment =
                "#version 120\n" \
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform vec2 u_sourceSize;\n" \

                "void main()\n" \
                "{\n" \
                "    vec2 pixelSize = 1.0 / u_sourceSize;\n" \
                "    vec2 texCoords = gl_TexCoord[0].xy;\n" \
                "    vec4 colour = texture2D(u_sourceTexture, texCoords);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, 0.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, 0.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(0.0, 1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(0.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, 1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(1.0, -1.0) * pixelSize);\n" \
                "    colour += texture2D(u_sourceTexture, texCoords + vec2(-1.0, 1.0) * pixelSize);\n" \
                "    gl_FragColor = colour / 9.0;\n" \
                "}";
        }
    }//namespace Shader
}//namespace xy

#endif //XY_SHADER_POSTDOWNSAMPLE_HPP_
