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

#ifndef XY_SHADER_POSTCHRAB_HPP_
#define XY_SHADER_POSTCHRAB_HPP_

#include <xygine/shaders/Default.hpp>

#define CHRAB_NO_DISTORT "#version 120\n" + xy::Shader::PostChromeAb::fragment
#define CHRAB_DISTORT "#version 120\n#define DISTORTION\n" + xy::Shader::PostChromeAb::fragment

namespace xy
{
    namespace Shader
    {
        namespace PostChromeAb
        {
            static const std::string fragment =
                /*"#version 120\n" \*/
                /*"#define BLUR\n" \*/
                "uniform sampler2D u_sourceTexture;\n" \
                "uniform float u_time;\n" \
                "uniform float u_lineCount = 6000.0;\n" \

                "const float noiseStrength = 0.7;\n" \

                "#if defined(BLUR)\n" \
                "const float maxOffset = 1.0 / 35.0;\n" \
                "#define KERNEL_SIZE 9\n" \
                "const vec2 kernel[KERNEL_SIZE] = vec2[KERNEL_SIZE]\n" \
                "(\n" \
                "    vec2(0.0000000000000000000, 0.04416589065853191),\n" \
                "    vec2(0.10497808951021347),\n" \
                "    vec2(0.0922903086524308425, 0.04416589065853191),\n" \
                "    vec2(0.0112445223775533675, 0.10497808951021347)," \
                "    vec2(0.40342407932501833),\n" \
                "    vec2(0.1987116566428735725, 0.10497808951021347),\n" \
                "    vec2(0.0000000000000000000, 0.04416589065853191),\n" \
                "    vec2(0.10497808951021347),\n" \
                "    vec2(0.0922903086524308425, 0.04416589065853191)\n" \
                ");\n" \
                "#else\n" \
                "const float maxOffset = 1.0 / 450.0;\n" \
                "#endif\n" \

                "const float centreDistanceSquared = 0.25;\n" \
                "float distanceSquared(vec2 coord)\n" \
                "{\n" \
                "    return dot(coord, coord);\n" \
                "}\n" \

                "void main()\n" \
                "{\n" \
                "    vec2 distortOffset = vec2(0.01, 0.01);\n" \
                "    vec2 texCoord = gl_TexCoord[0].xy;\n" \
                "#if defined(DISTORTION)\n" \
                "    float distSquared = distanceSquared(0.5 - texCoord);\n" \
                "    if(distSquared > centreDistanceSquared) texCoord += ((vec2(0.5, 0.5) - texCoord) * (centreDistanceSquared - distSquared)) * 0.12;\n" \
                "#endif\n" \
                "    vec2 offset = vec2((maxOffset / 2.0) - (texCoord.x * maxOffset), (maxOffset / 2.0) - (texCoord.y * maxOffset));\n"
                "    vec3 colour = vec3(0.0);\n" \
                "#if defined(BLUR)\n" \
                "    for(int i = 0; i < KERNEL_SIZE; ++i)\n" \
                "    {\n" \
                "        colour.r += texture2D(u_sourceTexture, texCoord + (offset * kernel[i])).r;\n" \
                "        colour.g += texture2D(u_sourceTexture, texCoord).g;\n" \
                "        colour.b += texture2D(u_sourceTexture, texCoord - (offset * kernel[i])).b;\n" \
                "    }\n" \
                "    colour /= KERNEL_SIZE;\n"
                "#else\n" \
                "    colour.r = texture2D(u_sourceTexture, texCoord + offset).r;\n" \
                "    colour.g = texture2D(u_sourceTexture, texCoord).g;\n" \
                "    colour.b = texture2D(u_sourceTexture, texCoord - offset).b;\n" \
                "#endif\n" \

                /*noise*/
                "    float x = (texCoord.x + 4.0) * texCoord.y * u_time * 10.0;\n" \
                "    x = mod(x, 13.0) * mod(x, 123.0);\n" \
                "    float grain = mod(x, 0.01) - 0.005;\n" \
                "    vec3 result = colour + vec3(clamp(grain * 100.0, 0.0, 0.07));\n" \
                /*scanlines*/
                "    vec2 sinCos = vec2(sin(texCoord.y * u_lineCount), cos(texCoord.y * u_lineCount + u_time));\n" \
                "    result += colour * vec3(sinCos.x, sinCos.y, sinCos.x) * (noiseStrength * 0.08);\n" \
                "    colour += (result - colour) * noiseStrength;\n" \
                "    gl_FragColor = vec4(colour, 1.0);" \
                "}";
        }
    }//namespace Shader
}//namespace xy

#endif //XY_SHADER_POSTCHRAB_HPP_
