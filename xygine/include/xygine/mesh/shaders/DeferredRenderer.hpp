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

#ifndef XY_MESH_DEFERRED_HPP_
#define XY_MESH_DEFERRED_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            const static std::string ShadowVertex = R"(
                in vec3 a_position;
                in vec2 a_texCoord0;

                #if defined(SKINNED)
                in vec4 a_boneIndices;
                in vec4 a_boneWeights;
                uniform mat4[80] u_boneMatrices;
                #endif

                out vec2 v_texCoord;

                uniform mat4 u_worldMatrix;

                layout (std140) uniform u_matrixBlock
                {
                    mat4 u_viewMatrix;
                    mat4 u_projectionMatrix;
                    mat4 u_lightViewProjectionMatrix;
                };

                out vec4 v_position;

                void main()
                {
                    vec3 position = a_position;

                #if defined(SKINNED)
                	mat4 skinMatrix = u_boneMatrices[int(a_boneIndices.x)] * a_boneWeights.x;
                	skinMatrix += u_boneMatrices[int(a_boneIndices.y)] * a_boneWeights.y;
                	skinMatrix += u_boneMatrices[int(a_boneIndices.z)] * a_boneWeights.z;
                	skinMatrix += u_boneMatrices[int(a_boneIndices.w)] * a_boneWeights.w;
                	position = (skinMatrix * vec4(position, 1.0)).xyz;
                #endif
                    //gl_Position = u_projectionMatrix * u_worldViewMatrix * vec4(position, 1.0);
                    gl_Position = u_lightViewProjectionMatrix * u_worldMatrix * vec4(position, 1.0);
                    v_position = gl_Position;

                    v_texCoord = a_texCoord0;
                })";

            const static std::string ShadowFragment = R"(
                #version 150
                uniform sampler2D u_diffuseMap;
                in vec2 v_texCoord;
                //out vec4 fragOut;
                void main()
                {
                    if(texture(u_diffuseMap, v_texCoord).a < 0.5) discard;
                    //fragOut = vec4(vec3(gl_FragCoord.z), 1.0);
                })";

            const static std::string ShadowFragmentVSM = R"(
                #version 150
                in vec4 v_position;
                out vec4 fragOut;

                void main()
                {
                    float depth = v_position.z / v_position.w;
                    depth = depth * 0.5 + 0.5;

                    float moment1 = depth;
                    float moment2 = depth * depth;

                    float dx = dFdx(depth);
                    float dy = dFdy(depth);
                    moment2 += 0.25*(dx*dx + dy*dy);

                    fragOut = vec4(moment1, moment2, 0.0, 0.0);
                })";

            const static std::string DeferredVertex =
                "in vec3 a_position;\n" \
                "in vec3 a_normal;\n" \

                "#if defined(VERTEX_COLOUR)\n"
                "in vec4 a_colour;\n"
                "#endif\n"

                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "in vec2 a_texCoord0;\n" \
                "#endif\n" \

                "#if defined(BUMP)\n" \
                "in vec3 a_tangent;\n" \
                "in vec3 a_bitangent;\n" \
                "#endif\n" \

                "#if defined(SKINNED)\n" \
                "in vec4 a_boneIndices;\n" \
                "in vec4 a_boneWeights;\n" \
                "uniform mat4[80] u_boneMatrices;\n" \
                "#endif\n"

                "uniform mat4 u_worldMatrix;\n" \
                "uniform mat4 u_worldViewMatrix;\n" \
                "uniform mat3 u_normalMatrix;\n" \

                "layout (std140) uniform u_matrixBlock\n" \
                "{\n" \
                "    mat4 u_viewMatrix;\n" \
                "    mat4 u_projectionMatrix;\n" \
                "    mat4 u_lightViewProjectionMatrix;\n" \
                "};\n" \

                "#if !defined(BUMP)\n"
                "out vec3 v_normalVector;\n" \
                "#else\n"
                "out vec3 v_tbn[3];\n" \
                "#endif\n"
                "out vec3 v_worldPosition;\n" \
                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "out vec2 v_texCoord;\n" \
                "#endif\n" \

                "#if defined(VERTEX_COLOUR)\n"
                "out vec4 v_colour;\n"
                "#endif\n"

                "void main()\n" \
                "{\n" \
                "    vec3 position = a_position;\n" \

                "#if defined(SKINNED)\n" \
                "	mat4 skinMatrix = u_boneMatrices[int(a_boneIndices.x)] * a_boneWeights.x;\n" \
                "	skinMatrix += u_boneMatrices[int(a_boneIndices.y)] * a_boneWeights.y;\n" \
                "	skinMatrix += u_boneMatrices[int(a_boneIndices.z)] * a_boneWeights.z;\n" \
                "	skinMatrix += u_boneMatrices[int(a_boneIndices.w)] * a_boneWeights.w;\n" \
                "	position = (skinMatrix * vec4(position, 1.0)).xyz;\n" \
                "#endif\n" \

                "    v_worldPosition = (u_worldMatrix * vec4(position, 1.0)).xyz;\n" \
                "    gl_Position = u_projectionMatrix * u_worldViewMatrix * vec4(position, 1.0);\n" \

                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "    v_texCoord = a_texCoord0;\n" \
                "#endif\n" \

                "    vec3 normal = a_normal;\n" \
                "#if defined(SKINNED)\n" \
                "    normal = (skinMatrix * vec4(normal, 0.0)).xyz;\n" \
                "#endif\n" \

                "#if defined(BUMP)\n" \
                "    vec3 tangent = a_tangent;\n" \
                "    vec3 bitangent = a_bitangent;\n" \
                "#if defined(SKINNED)\n" \
                "    tangent = (skinMatrix * vec4(tangent, 0.0)).xyz;\n" \
                "    bitangent = (skinMatrix * vec4(bitangent, 0.0)).xyz;\n" \
                "#endif\n" \
                "#endif\n" \

                "#if !defined(BUMP)\n"
                "    v_normalVector = u_normalMatrix * normal;\n" \
                "#else\n"
                /*"    mat3 normalMatrix = inverse(mat3(u_worldMatrix));\n" \*/
                "    v_tbn[0] = normalize(u_worldMatrix * vec4(tangent, 0.0)).xyz;\n" \
                "    v_tbn[1] = normalize(u_worldMatrix * vec4(bitangent, 0.0)).xyz;\n" \
                "    v_tbn[2] = normalize(u_worldMatrix * vec4(normal, 0.0)).xyz;\n" \
                /*"    v_tbn = mat3(t, b, n);\n" \*/
                "#endif\n"

                "#if defined(VERTEX_COLOUR)\n"
                "    v_colour = a_colour;\n"
                "#endif\n"
                "}";

                const static std::string DeferredFragment =
                    "#if !defined(BUMP)\n" \
                    "in vec3 v_normalVector;\n"
                    "#else\n" \
                    "in vec3 v_tbn[3];\n" \
                    "#endif\n" \
                    "in vec3 v_worldPosition;\n" \
                    "#if defined(TEXTURED) || defined(BUMP)\n" \
                    "in vec2 v_texCoord;\n" \
                    "#endif\n" \

                    "#if defined(VERTEX_COLOUR)\n"
                    "in vec4 v_colour;\n"
                    "#endif\n"

                    "uniform float u_farPlane = 1500.0;\n" \
                    "uniform vec4 u_colour = vec4(1.0);\n" \
                    "#if defined(TEXTURED)\n"
                    "uniform sampler2D u_diffuseMap;\n" \
                    "#endif\n" \
                    "#if defined(BUMP)\n"
                    "uniform sampler2D u_normalMap;\n" \
                    "#endif\n" \
                    "#if defined(BUMP) || defined(TEXTURED)\n"
                    "uniform sampler2D u_maskMap;\n"
                    "#else\n"
                    "uniform vec4 u_maskColour = vec4(vec3(0.0), 1.0);\n"
                    "#endif\n"

                    "out vec4[4] fragOut;\n" \

                    "const float nearPlane = 0.1;\n" \
                    "float lineariseDepth(float val)\n" \
                    "{\n" \
                    "    float z = val * 2.0 - 1.0;\n" \
                    "    return (2.0 * nearPlane * u_farPlane) / (u_farPlane + nearPlane - z * (u_farPlane - nearPlane));\n" \
                    "}\n" \

                    "void main()\n" \
                    "{\n" \
                    "#if defined(TEXTURED)\n" \
                    "    fragOut[0] = texture(u_diffuseMap, v_texCoord) * u_colour;\n" \
                    "#else\n" \
                    "    fragOut[0] = u_colour;\n" \
                    "#endif\n" \

                    "#if defined(VERTEX_COLOUR)\n"
                    "    fragOut[0] *= v_colour;\n"
                    "#endif\n"

                    "#if !defined(BUMP)\n" \
                    "    fragOut[1] = vec4(normalize(v_normalVector), 1.0);\n" \
                    "#else\n" \
                    "    vec3 normal = texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0;\n" \
                    "    fragOut[1] = vec4(normalize(v_tbn[0] * normal.x + v_tbn[1] * normal.y + v_tbn[2] * normal.z).rgb, 1.0);\n" \
                    "#endif\n"
                    "#if defined(BUMP) || defined(TEXTURED)\n"
                    "    fragOut[2] = texture(u_maskMap, v_texCoord);\n"
                    "#else\n"
                    "    fragOut[2] = u_maskColour;\n"
                    "#endif\n"
                    "    fragOut[3].rgb = v_worldPosition;\n" \
                    "    fragOut[3].a = /*lineariseDepth*/(gl_FragCoord.z);\n" \
                "}";
        }
    }
}

#define SHADOW_VERTEX "#version 150\n" + xy::Shader::Mesh::ShadowVertex
#define SHADOW_VERTEX_SKINNED "#version 150\n#define SKINNED\n" + xy::Shader::Mesh::ShadowVertex
#define SHADOW_FRAGMENT xy::Shader::Mesh::ShadowFragment

#define DEFERRED_COLOURED_VERTEX "#version 150\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_COLOURED_FRAGMENT "#version 150\n" + xy::Shader::Mesh::DeferredFragment

#define DEFERRED_COLOURED_BUMPED_VERTEX "#version 150\n#define BUMP\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_COLOURED_BUMPED_FRAGMENT "#version 150\n#define BUMP\n" + xy::Shader::Mesh::DeferredFragment

#define DEFERRED_TEXTURED_VERTEX "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_TEXTURED_FRAGMENT "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredFragment

#define DEFERRED_TEXTURED_BUMPED_VERTEX "#version 150\n#define BUMP\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_TEXTURED_BUMPED_FRAGMENT "#version 150\n#define BUMP\n#define TEXTURED\n" + xy::Shader::Mesh::DeferredFragment

#define DEFERRED_TEXTURED_BUMPED_SKINNED_VERTEX "#version 150\n#define BUMP\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_TEXTURED_SKINNED_VERTEX "#version 150\n#define TEXTURED\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex

#define DEFERRED_VERTCOLOURED_VERTEX "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::DeferredVertex
#define DEFERRED_VERTCOLOURED_FRAGMENT "#version 150\n#define VERTEX_COLOUR\n" + xy::Shader::Mesh::DeferredFragment

#define DEFERRED_VERTCOLOURED_SKINNED_VERTEX "#version 150\n#define VERTEX_COLOUR\n#define SKINNED\n" + xy::Shader::Mesh::DeferredVertex

#endif //XY_MESH_DEFERRED_HPP_