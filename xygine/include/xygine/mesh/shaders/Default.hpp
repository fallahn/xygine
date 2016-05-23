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

#ifndef XY_MESH_DEFAULT_HPP_
#define XY_MESH_DEFAULT_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const int MAX_LIGHTS = 8;
            static const std::string DefaultVertex =
                "#define MAX_POINT_LIGHTS 8\n" \

                "in vec3 a_position;\n" \
                "in vec3 a_normal;\n" \

                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "in vec2 a_texCoord0;\n" \
                "#endif\n" \

                "#if defined(BUMP)\n" \
                "in vec3 a_tangent;\n" \
                "in vec3 a_bitangent;\n" \
                "#endif\n" \

                "uniform mat4 u_worldMatrix;\n" \
                "uniform mat4 u_worldViewMatrix;\n" \
                "uniform mat3 u_normalMatrix;\n" \

                "struct Point\n" \
                "{\n" \
                "    vec3 position;\n" \
                "    float padding;\n" \
                "};\n" \

                "layout (std140) uniform u_matrixBlock\n" \
                "{\n" \
                "    mat4 u_viewMatrix;\n" \
                "    mat4 u_projectionMatrix;\n" \
                "    Point u_pointLightPositions[MAX_POINT_LIGHTS];\n" \
                "    vec3 u_cameraWorldPosition;\n" \
                "};\n" \

                "#if !defined(BUMP)\n"
                "out vec3 v_normalVector;\n" \
                "#endif\n"
                "out vec3 v_pointLightDirections[MAX_POINT_LIGHTS];\n" \
                "out vec3 v_eyeDirection;\n" \
                "out vec3 v_viewPosition;\n" \

                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "out vec2 v_texCoord;\n" \
                "#endif\n" \

                "void main()\n" \
                "{\n" \
                "    vec4 viewVertex = u_worldViewMatrix * vec4(a_position, 1.0);\n" \
                "    gl_Position = u_projectionMatrix * viewVertex;\n" \
                
                "    vec3 worldVertex = vec3(u_worldMatrix * vec4(a_position, 1.0));\n" \
                "    vec3 normal = u_normalMatrix * a_normal;\n" \
                "#if defined(BUMP)\n" \
                "    vec3 tangent = u_normalMatrix * a_tangent;\n" \
                "    vec3 bitangent = u_normalMatrix * a_bitangent;\n" \
                "    mat3 tangentSpaceMatrix = mat3(tangent, bitangent, normal);\n" \
                "    for(int i = 0; i < MAX_POINT_LIGHTS; ++i)\n" \
                "    {\n" \
                "        v_pointLightDirections[i] = tangentSpaceMatrix * (u_pointLightPositions[i].position - worldVertex);\n" \
                "    }\n" \

                "    v_eyeDirection = tangentSpaceMatrix * (u_cameraWorldPosition - worldVertex);\n" \
                "#else\n" \
                "    for(int i = 0; i < MAX_POINT_LIGHTS; ++i)\n" \
                "    {\n" \
                "        v_pointLightDirections[i] = u_pointLightPositions[i].position - worldVertex;\n" \
                "    }\n" \

                "    v_eyeDirection = u_cameraWorldPosition - worldVertex;\n" \
                "    v_normalVector = normal;\n" \
                "#endif\n" \
                
                "    v_viewPosition = viewVertex.xyz;\n" \

                "#if defined(TEXTURED)\n" \
                "    v_texCoord = a_texCoord0;\n" \
                "#endif\n" \
                "}";

            static const std::string DefaultFragment =
                "#define MAX_POINT_LIGHTS 8\n" \

                "struct PointLight\n" \
                "{\n" \
                "    vec4 diffuseColour;\n" \
                "    vec4 specularColour;\n" \
                "    float inverseRange;\n" \
                "    float intensity;\n" \
                "    vec2 padding;\n" \
                "};\n" \

                "layout (std140) uniform u_lightBlock\n" \
                "{\n" \
                "    PointLight u_pointLights[MAX_POINT_LIGHTS];\n" \
                "};\n" \

                "#if defined(TEXTURED)\n" \
                "uniform sampler2D u_diffuseMap;\n" \
                "#endif\n" \
                "#if defined(BUMP)\n" \
                "uniform sampler2D u_normalMap;\n" \
                "#endif\n"
                "uniform vec4 u_colour = vec4(1.0, 1.0, 1.0, 1.0);\n" \
                "uniform vec3 u_ambientColour = vec3(0.2, 0.2, 0.2);\n" \

                "#if !defined(BUMP)\n" \
                "in vec3 v_normalVector;\n" \
                "#endif\n" \
                "in vec3 v_pointLightDirections[MAX_POINT_LIGHTS];\n" \
                "in vec3 v_eyeDirection;\n" \
                "in vec3 v_viewPosition;\n" \
                "#if defined(TEXTURED) || defined(BUMP)\n" \
                "in vec2 v_texCoord;\n" \
                "#endif\n" \

                "out vec4[3] fragOut;\n" \

                "vec3 diffuseColour;\n" \
                "vec3 calcLighting(vec3 normal, vec3 lightDirection, vec3 lightDiffuse, vec3 lightSpec, float falloff)\n" \
                "{\n" \
                "    float diffuseAmount = max(dot(normal, lightDirection), 0.0);\n" \
                "    diffuseAmount = pow((diffuseAmount * 0.5) + 0.5, 2.0);\n" \
                "    vec3 mixedColour = diffuseColour * lightDiffuse * diffuseAmount * falloff;\n" \

                "    vec3 eyeDirection = normalize(v_eyeDirection);\n" \
                "    vec3 halfVec = normalize(eyeDirection + lightDirection);\n" \
                "    float specularAngle = clamp(dot(normal, halfVec), 0.0, 1.0);\n" \
                "    vec3 specularColour = lightSpec * vec3(pow(specularAngle, 180.0)) * falloff;\n" \

                "    return mixedColour + specularColour;\n" \
                "}\n" \

                "void main()\n" \
                "{\n" \
                "    diffuseColour = u_colour.rgb;\n" \
                "#if defined(TEXTURED)\n" \
                "    vec4 diffuseMap = texture(u_diffuseMap, v_texCoord);\n" \
                "    diffuseColour *= diffuseMap.rgb;\n" \
                "#endif\n" \
                "    vec3 blendedColour = diffuseColour * u_ambientColour;\n" \

                "#if defined(BUMP)\n" \
                "    vec3 normal = normalize(texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0);\n" \
                /*"    normal.g = 1.0 - normal.g;\n" \
                "    normal = normalize(normal * 2.0 - 1.0);\n" \*/
                "#else\n" \
                "    vec3 normal = normalize(v_normalVector);\n" \
                "#endif\n" \
                "    for(int i = 0; i < MAX_POINT_LIGHTS; ++i)\n" \
                "    {\n" \
                "        vec3 pointLightDirection = v_pointLightDirections[i] * u_pointLights[i].inverseRange;\n" \
                "        float falloff = clamp(1.0 - sqrt(dot(pointLightDirection, pointLightDirection)), 0.0, 1.0);\n" \
                "        blendedColour += calcLighting(normal, normalize(v_pointLightDirections[i]), u_pointLights[i].diffuseColour.rgb, u_pointLights[i].specularColour.rgb, falloff) * u_pointLights[i].intensity;\n" \
                "    }\n" \

                /*"    blendedColour += calcLighting(normal, normalize(vec3(1.0, 1.0, 0.5)), vec3(1.0), vec3 (1.0), 1.0);\n"*/

                "#if defined(TEXTURED)\n" \
                "    fragOut[0] = vec4(blendedColour, diffuseMap.a);\n" \
                "#else\n"
                "    fragOut[0] = vec4(blendedColour, 1.0);\n" \
                "#endif\n"
                "    fragOut[1] = vec4(v_viewPosition, 1.0);\n" \
                "    fragOut[2] = vec4(0.5 * (normalize(normal) + 1.0), 1.0);\n" \
                "}";
        }
    }
}

#define COLOURED_VERTEX "#version 150\n" + xy::Shader::Mesh::DefaultVertex
#define COLOURED_FRAGMENT "#version 150\n" + xy::Shader::Mesh::DefaultFragment
#define TEXTURED_VERTEX "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DefaultVertex
#define TEXTURED_FRAGMENT "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::DefaultFragment

#define COLOURED_BUMPED_VERTEX "#version 150\n#define BUMP\n" + xy::Shader::Mesh::DefaultVertex
#define COLOURED_BUMPED_FRAGMENT "#version 150\n#define BUMP\n" + xy::Shader::Mesh::DefaultFragment
#define TEXTURED_BUMPED_VERTEX "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::DefaultVertex
#define TEXTURED_BUMPED_FRAGMENT "#version 150\n#define TEXTURED\n#define BUMP\n" + xy::Shader::Mesh::DefaultFragment

#endif //XY_MESH_DEFAULT_HPP_