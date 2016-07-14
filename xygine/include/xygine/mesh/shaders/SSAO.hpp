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
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string QuadVertex =
                "#version 120\n"

                "varying vec2 v_texCoord;\n"

                "void main()\n"
                "{\n"
                "    gl_Position = gl_Vertex;\n"
                "    gl_Position.xy = (gl_Vertex.xy * 2.0) - vec2(1.0);\n"
                "    v_texCoord = gl_Vertex.xy;\n"
                "}";

            static const std::string SSAOFragment =
                "#version 120\n"
                "#define MAX_KERNEL_SIZE 64\n"

                "varying vec2 v_texCoord;\n"

                "uniform sampler2D u_positionMap;\n"
                "uniform float u_sampleRadius = 1.5f;\n"
                "uniform mat4 u_projectionMatrix;\n"
                "uniform vec3 u_kernel[MAX_KERNEL_SIZE];\n"

                "void main()\n"
                "{\n"
                "    vec3 pos = texture2D(u_positionMap, v_texCoord).xyz;\n"

                "    float AO = 0.0;\n"

                "    for (int i = 0; i < MAX_KERNEL_SIZE; ++i)\n"
                "    {\n"
                "        vec3 samplePos = pos + u_kernel[i];\n"
                "        vec4 offset = vec4(samplePos, 1.0);\n"
                "        offset = u_projectionMatrix * offset;\n"
                "        offset.xy /= offset.w;\n"
                "        offset.xy = offset.xy * 0.5 + vec2(0.5);\n"

                "        float sampleDepth = texture2D(u_positionMap, offset.xy).b;\n"

                "        if (abs(pos.z - sampleDepth) < u_sampleRadius)\n"
                "        {\n"
                "            AO += step(sampleDepth, samplePos.z);\n"
                "        }\n"
                "    }\n"

                "    AO = AO / 64.0;\n"

                "    gl_FragColor = vec4(vec3(clamp(AO * 2.0, 0.0, 1.0)), 1.0); //vec4(vec3(AO * AO), 1.0);//\n"
                "}";

            static const std::string SSAOFragment2 =
                "#version 120\n"

                "varying vec2 v_texCoord;\n"

                "uniform sampler2D u_positionMap;\n"
                "uniform sampler2D u_normalMap;\n"
                "uniform sampler2D u_noiseMap;\n"
                "uniform vec3 u_kernel[64];\n"
                "uniform mat4 u_projectionMatrix;\n"

                "const int kernelSize = 64;\n"
                "const float radius = 5.0;\n"

                "void main()\n"
                "{\n"
                "    vec3 fragPos = texture2D(u_positionMap, v_texCoord).xyz;\n"
                "    vec3 normal = texture2D(u_normalMap, v_texCoord).rgb;\n"
                "    vec3 randomVec = texture2D(u_noiseMap, v_texCoord * vec2(960.0f / 4.0f, 540.0f / 4.0f)).xyz;\n"

                "    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal))\n;"
                "    vec3 bitangent = cross(normal, tangent);\n"
                "    mat3 TBN = mat3(tangent, bitangent, normal);\n"

                "    float occlusion = 0.0;\n"
                "    for (int i = 0; i < kernelSize; ++i)\n"
                "    {\n"
                "        vec3 sample = TBN * u_kernel[i];\n"
                "        sample = fragPos + sample * radius;\n"

                "        vec4 offset = vec4(sample, 1.0);\n"
                "        offset = u_projectionMatrix * offset;\n"
                "        offset.xyz /= offset.w;\n"
                "        offset.xyz = offset.xyz * 0.5 + 0.5;\n"

                "        float sampleDepth = -texture2D(u_positionMap, offset.xy).w;\n"

                "        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));\n"
                "        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;\n"
                "    }"
                "    occlusion = 1.0 - (occlusion / kernelSize);\n"

                "    gl_FragColor = vec4(vec3(occlusion), 1.0);\n"
                "}";
        }
    }
}

#endif //XY_MESH_SSAO_HPP_