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

#ifndef XY_MESH_SSAO_HPP_
#define XY_MESH_SSAO_HPP_

#include <string>

namespace xy
{
    namespace Shader3D
    {
        const std::string SSAOVertex =
            "#version 150\n"

            /*"in vec3 a_position;\n"*/

            "out vec2 v_texCoord;\n"

            "void main()\n"
            "{"
            "    gl_Position = gl_Vertex;//vec4(a_position, 1.0);\n"
            "    v_texCoord = (gl_Vertex.xy + vec2(1.0)) / 2.0;\n"
            "}\n";

        const std::string SSAOFragment =
            "#version 150\n"
            "#define MAX_KERNEL_SIZE 128\n"

            "in vec2 v_texCoord;\n"
            
            "uniform sampler2D u_positionMap;\n"
            "uniform float u_sampleRadius = 1.5f;\n"
            "uniform mat4 u_projectionMatrix;\n"
            "uniform vec3 u_kernel[MAX_KERNEL_SIZE];\n"
            
            "out vec4 fragColour;\n"

            "void main()\n"
            "{\n"
            "    vec3 pos = texture(u_positionMap, v_texCoord).xyz;\n"

            "    float AO = 0.0;\n"

            "    for (int i = 0; i < MAX_KERNEL_SIZE; ++i)\n"
            "    {\n"
            "        vec3 samplePos = pos + u_kernel[i];\n"
            "        vec4 offset = vec4(samplePos, 1.0);\n"
            "        offset = u_projectionMatrix * offset;\n"
            "        offset.xy /= offset.w;\n"
            "        offset.xy = offset.xy * 0.5 + vec2(0.5);\n"

            "        float sampleDepth = texture(u_positionMap, offset.xy).b;\n"

            "        if (abs(pos.z - sampleDepth) < u_sampleRadius)\n"
            "        {\n"
            "            AO += step(sampleDepth, samplePos.z);\n"
            "        }\n"
            "    }\n"

            "    AO = AO / 128.0;\n"

            "    fragColour = vec4(clamp(AO * 2.0, 0.0, 1.0));\n"
            "}";
    }
}

#endif //XY_MESH_SSAO_HPP_