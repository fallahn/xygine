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

#ifndef XY_MESH_LIGHT_BLUR_HPP_
#define XY_MESH_LIGHT_BLUR_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string LightBlurFrag =
                "#version 140\n"

                "uniform sampler2D u_illuminationMap;\n"

                "in vec2 v_texCoord;\n"
                "out vec4 fragOut;\n"

                "const int blurSize = 12;\n"
                "void main()\n"
                "{\n"
                "    vec2 texelSize = 1.0 / vec2(textureSize(u_illuminationMap, 0));\n"
                "    vec4 result = vec4(0.0);\n"
                "    for (int x = 0; x < blurSize; ++x)\n"
                "    {"
                "        for (int y = 0; y < blurSize; ++y)\n"
                "        {\n"
                "            vec2 offset = (vec2(-6.0) + vec2(float(x * 1.2), float(y * 1.2))) * texelSize;\n"
                "            result += texture2D(u_illuminationMap, v_texCoord + offset);\n"
                "        }\n"
                "    }\n"
                "    fragOut = result / float(blurSize * 4.0);\n"
                "}";

            static const std::string LightDownsampleFrag =
                "#version 120\n"

                "uniform sampler2D u_diffuseMap;\n"
                "uniform sampler2D u_maskMap;\n"

                "varying vec2 v_texCoord;\n"

                "void main()\n"
                "{\n"
                "    vec4 diffuse = texture2D(u_diffuseMap, v_texCoord);\n"
                "    gl_FragColor.rgb = diffuse.rgb * texture2D(u_maskMap, v_texCoord).b;\n"
                "    gl_FragColor.a = diffuse.a;\n"
                "}\n";
        }
    }
}

#endif //XY_MESH_LIGHT_BLUR_HPP_