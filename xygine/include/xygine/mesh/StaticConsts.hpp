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

#ifndef XY_STATIC_CONSTS_HPP_
#define XY_STATIC_CONSTS_HPP_

#include <string>

namespace xy
{
    static const std::string VertexAttribPosition("a_position");
    static const std::string VertexAttribColour("a_colour");
    static const std::string VertexAttribNormal("a_normal");
    static const std::string VertexAttribTangent("a_tangent");
    static const std::string VertexAttribBitangent("a_bitangent");
    static const std::string VertexAttribUV0("a_texCoord0");
    static const std::string VertexAttribUV1("a_texCoord1");
    static const std::string VertexAttribBlendIndices("a_boneIndices");
    static const std::string VertexAttribBlendWeights("a_boneWeights");

    namespace Shader3D
    {
        static const int MAX_LIGHTS = 8;
        static const std::string DefaultVertex =
            "#version 150\n"
            "#define MAX_POINT_LIGHTS 8\n"

            "in vec3 a_position;\n"
            "in vec3 a_normal;\n"

            "uniform mat4 u_worldMatrix;\n"
            "uniform mat4 u_worldViewMatrix;\n"
            "uniform mat3 u_normalMatrix;\n"

            "struct Point\n"
            "{\n"
            "    vec3 position;\n"
            "    float padding;\n"
            "};\n"

            "layout (std140) uniform u_matrixBlock\n"
            "{\n"
            "    mat4 u_viewMatrix;\n"
            "    mat4 u_projectionMatrix;\n"
            "    Point u_pointLightPositions[MAX_POINT_LIGHTS];\n"
            "    vec3 u_cameraWorldPosition;\n"
            "};\n"

            "out vec3 v_normalVector;\n"
            "out vec3 v_pointLightDirections[MAX_POINT_LIGHTS];\n"
            "out vec3 v_eyeDirection;\n"
            "out vec3 v_viewPosition;\n"

            "void main()\n"
            "{\n"
            "    vec4 viewVertex = u_worldViewMatrix * vec4(a_position, 1.0);\n"
            "    gl_Position = u_projectionMatrix * viewVertex;\n"

            "    v_normalVector = u_normalMatrix * a_normal;\n"

            "    vec3 worldVertex = vec3(u_worldMatrix * vec4(a_position, 1.0));\n"
            "    for(int i = 0; i < MAX_POINT_LIGHTS; ++i)\n"
            "    {\n"
            "        v_pointLightDirections[i] = u_pointLightPositions[i].position - worldVertex;\n"
            "    }\n"

            "    v_eyeDirection = u_cameraWorldPosition - worldVertex;\n"
            "    v_viewPosition = viewVertex.xyz;\n"
            "}";

        static const std::string DefaultFragment =
            "#version 150\n"
            "#define MAX_POINT_LIGHTS 8\n"

            "struct PointLight\n"
            "{\n"
            "    vec4 diffuseColour;\n"
            "    vec4 specularColour;\n"
            "    float inverseRange;\n"
            "    float intensity;\n"
            "    vec2 padding;\n"
            "};\n"

            "layout (std140) uniform u_lightBlock\n"
            "{\n"
            "    PointLight u_pointLights[MAX_POINT_LIGHTS];\n"
            "};\n"

            "uniform vec4 u_colour = vec4(1.0, 1.0, 0.0, 1.0);\n"
            "uniform vec3 u_ambientColour = vec3(0.2, 0.2, 0.2);\n"

            "in vec3 v_normalVector;\n"
            "in vec3 v_pointLightDirections[MAX_POINT_LIGHTS];\n"
            "in vec3 v_eyeDirection;\n"
            "in vec3 v_viewPosition;\n"

            "out vec4[2] fragOut;\n"

            "vec3 diffuseColour;\n"
            "vec3 calcLighting(vec3 normal, vec3 lightDirection, vec3 lightDiffuse, vec3 lightSpec, float falloff)\n"
            "{\n"
            "    float diffuseAmount = max(dot(normal, lightDirection), 0.0);\n"
            "    diffuseAmount = pow((diffuseAmount * 0.5) + 0.5, 2.0);\n"
            "    vec3 mixedColour = diffuseColour * lightDiffuse * diffuseAmount * falloff;\n"

            "    vec3 eyeDirection = normalize(v_eyeDirection);\n"
            "    vec3 halfVec = normalize(eyeDirection + lightDirection);\n"
            "    float specularAngle = clamp(dot(normal, halfVec), 0.0, 1.0);\n"
            "    vec3 specularColour = lightSpec * vec3(pow(specularAngle, 180.0)) * falloff;\n"

            "    return mixedColour + specularColour;\n"
            "}\n"

            "void main()\n"
            "{\n"
            "    diffuseColour = u_colour.rgb;\n"
            "    vec3 blendedColour = diffuseColour * u_ambientColour;\n"

            "    vec3 normal = normalize(v_normalVector);\n"
            "    for(int i = 0; i < MAX_POINT_LIGHTS; ++i)\n"
            "    {\n"
            "        vec3 pointLightDirection = v_pointLightDirections[i] * u_pointLights[i].inverseRange;\n"
            "        float falloff = clamp(1.0 - sqrt(dot(pointLightDirection, pointLightDirection)), 0.0, 1.0);\n"
            "        blendedColour += calcLighting(normal, normalize(v_pointLightDirections[i]), u_pointLights[i].diffuseColour.rgb, u_pointLights[i].specularColour.rgb, falloff) * u_pointLights[i].intensity;\n"
            "    }\n"

            /*"    blendedColour += calcLighting(normal, normalize(vec3(1.0, 1.0, 0.0)), vec3(1.0), vec3 (1.0), 1.0);"*/

            "    fragOut[0] = vec4(blendedColour, 1.0);\n"
            "    fragOut[1] = vec4(v_viewPosition, 1.0);\n"
            "}";
    }
}
#endif //XY_STATIC_CONSTS_HPP_
