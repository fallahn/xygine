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

#ifndef XY_MESH_LIGHTING_HPP_
#define XY_MESH_LIGHTING_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string DebugFrag =
                "#version 150\n"
                "uniform sampler2D u_texture;\n"
                "in vec2 v_texCoord;\n"
                "out vec4 fragOut;\n"
                "void main()\n"
                "{\n"
                "    fragOut = texture(u_texture, v_texCoord);\n"
                "}\n";

            static const std::string DepthFrag =
                "#version 150\n"
                "uniform sampler2DArray u_texture;\n"
                "uniform float u_texIndex = 0.0;\n"
                "in vec2 v_texCoord;\n"
                "out vec4 fragOut;\n"
                "void main()\n"
                "{\n"
                "    float depth = texture(u_texture, vec3(v_texCoord, u_texIndex)).r;\n"
                "    fragOut = vec4(vec3(depth), 1.0);\n"
                "    //fragOut.rgb = texture(u_texture, vec3(v_texCoord, u_texIndex)).rgb;\n"
                "    //fragOut.a = 1.0;\n"
                "}\n";

            static const std::string LightingVert =
                "#version 130\n"

                "out vec2 v_texCoord;\n"

                "void main()\n"
                "{\n"
                "    gl_Position = gl_Vertex;\n"
                "    gl_Position.xy = (gl_Vertex.xy * 2.0) - vec2(1.0);\n"
                "    v_texCoord = gl_Vertex.xy;\n"
                "    v_texCoord.y = 1.0 - v_texCoord.y;\n"
                "}";
            
            static const std::string LightingFrag =
                "#version 150\n"
                "//#define WATER\n"
                "#define MAX_POINT_LIGHTS 8\n"
                "struct PointLight\n"
                "{\n"
                "    vec4 diffuseColour;\n"
                "    vec4 specularColour;\n"
                "    float inverseRange;\n"
                "    float range;\n"
                "    float intensity;\n"
                "    bool castShadows;\n"
                "    vec3 position;\n"
                "    float padding;\n"
                "    mat4 vpMatrix;\n"
                "};\n"

                "struct SkyLight\n"
                "{\n"
                "    vec4 diffuseColour;\n"
                "    vec4 specularColour;\n"
                "    vec3 direction;\n"
                "    float padding;\n"
                "    mat4 vpMatrix;\n"
                "    float intensity;\n"
                "};\n"

                "layout (std140) uniform u_lightBlock\n"
                "{\n"
                "    PointLight u_pointLights[MAX_POINT_LIGHTS];\n"
                "    vec4 u_ambientColour;\n"
                "    vec3 u_cameraWorldPosition;\n"
                "    float padding;\n"
                "    SkyLight u_skyLight;\n"
                "};\n"

                "uniform sampler2D u_diffuseMap;\n"
                "uniform sampler2D u_normalMap;\n"
                "uniform sampler2D u_maskMap;\n"
                "uniform sampler2D u_positionMap;\n"
                "//uniform sampler2D u_aoMap;\n"
                "uniform sampler2D u_illuminationMap;\n"
                "uniform sampler2D u_reflectMap;\n"
                "uniform int u_lightCount = MAX_POINT_LIGHTS;\n"
                "uniform sampler2DArray u_depthMaps;\n"
                "uniform sampler2D u_depthBuffer;\n"

                "in vec2 v_texCoord;\n"

                "out vec4 fragOut;\n"

                "vec3 diffuseColour;\n"
                "vec4 mask;\n"
                "vec3 eyeDirection;\n"
                "vec3 calcLighting(vec3 normal, vec3 lightDirection, vec3 lightDiffuse, vec3 lightSpec, float falloff)\n"
                "{\n"

                "    float diffuseAmount = max(dot(normal, lightDirection), 0.0);\n"
                "    diffuseAmount = pow((diffuseAmount * 0.5) + 0.5, 2.0);\n"
                "    vec3 mixedColour = diffuseColour * lightDiffuse * diffuseAmount * falloff;\n"

                "    vec3 halfVec = normalize(eyeDirection + lightDirection);\n"
                "    float specularAngle = clamp(dot(normal, halfVec), 0.0, 1.0);\n"
                "    vec3 specularColour = lightSpec * vec3(pow(specularAngle, (254.0 * mask.r) + 1.0)) * falloff;\n"

                "    return mixedColour + (specularColour * mask.g);\n"
                "}\n" \

                "const vec2 kernel[16] = vec2[](\n"
                "    vec2(-0.94201624, -0.39906216),\n"
                "    vec2(0.94558609, -0.76890725),\n"
                "    vec2(-0.094184101, -0.92938870),\n"
                "    vec2(0.34495938, 0.29387760),\n"
                "    vec2(-0.91588581, 0.45771432),\n"
                "    vec2(-0.81544232, -0.87912464),\n"
                "    vec2(-0.38277543, 0.27676845),\n"
                "    vec2(0.97484398, 0.75648379),\n"
                "    vec2(0.44323325, -0.97511554),\n"
                "    vec2(0.53742981, -0.47373420),\n"
                "    vec2(-0.26496911, -0.41893023),\n"
                "    vec2(0.79197514, 0.19090188),\n"
                "    vec2(-0.24188840, 0.99706507),\n"
                "    vec2(-0.81409955, 0.91437590),\n"
                "    vec2(0.19984126, 0.78641367),\n"
                "    vec2(0.14383161, -0.14100790)\n"
                ");\n"
                "const int filterSize = 3;\n"
                "float calcShadow(int layer, vec4 position)\n"
                "{\n"
                "    float bias = 0.001;\n"
                "    vec3 projectionCoords = position.xyz / position.w;\n"
                "    projectionCoords = projectionCoords * 0.5 + 0.5;\n"
                "    if(projectionCoords.z > 1.0) return 1.0;\n"
                "    float shadow = 0.0;\n"
                "    vec2 texelSize = 1.0 / textureSize(u_depthMaps, 0).xy;\n"
                "    for(int x = 0; x < filterSize; ++x)\n"
                "    {\n"
                "        for(int y = 0; y < filterSize; ++y)\n"
                "        {\n"
                "            float pcfDepth = texture(u_depthMaps, vec3(projectionCoords.xy + kernel[y * filterSize + x] * texelSize, float(layer))).r;\n"
                "            shadow += (projectionCoords.z - bias) > pcfDepth ? 1.0 : 0.0;\n"
                "        }\n"
                "    }\n"
                "    return 1.0 - (shadow / 9.0);\n"
                "}\n"

                "float vsmShadow(int layer, vec4 position)\n"
                "{\n"
                "	vec3 projectionCoords = position.xyz / position.w;\n"
                "	projectionCoords = projectionCoords * 0.5 + 0.5;\n"

                "	vec2 moments = texture(u_depthMaps, vec3(projectionCoords.xy, float(layer))).rg;\n"
                "	if (projectionCoords.z <= moments.x) return 1.0;\n"
                "	float variance = moments.y - (moments.x * moments.x);\n"
                "	variance = max(variance, 0.00002);\n"
                "	float d = projectionCoords.z - moments.x;\n"
                "	float p_max = variance / (variance + d * d);\n"

                "	return p_max;\n"
                "}\n"

                "float lenSqr(vec3 vector)\n"
                "{\n"
                "    return dot(vector, vector);\n"
                "}\n"

                /*"    float blendOverlay(float base, float blend)\n"
                "    {\n"
                "        return (base < 0.5) ? (2.0 * base * blend) : (1.0 - 2.0 * (1.0 - base) * (1.0 - blend));\n"
                "    }\n"*/

                "vec3 getReflection(vec3 normal)\n"
                "{\n"
                "    vec3 reflection = reflect(normal, eyeDirection);\n"
                "    vec2 coord = vec2(reflection.x, reflection.y);\n"
                "    return texture(u_reflectMap, coord).rgb;\n"
                "}\n"

                //#include "Water.inl"

                "void main()\n"
                "{\n"
                "    vec4 fragPosition = texture(u_positionMap, v_texCoord);\n"
                "    mask = texture(u_maskMap, v_texCoord);\n"
                "    vec3 normal = texture(u_normalMap, v_texCoord).rgb;\n"
                "    vec4 diffuse = texture(u_diffuseMap, v_texCoord);\n"
                "    diffuseColour = diffuse.rgb;\n"
                "    vec3 blendedColour = diffuse.rgb * u_ambientColour.rgb;\n"
                "    eyeDirection = normalize(u_cameraWorldPosition - fragPosition.xyz);\n"

                "    if(u_skyLight.intensity > 0.0)\n"
                "    {\n"
                "         vec3 lighting = calcLighting(normal, normalize(-u_skyLight.direction), u_skyLight.diffuseColour.rgb, u_skyLight.specularColour.rgb, 1.0) * u_skyLight.intensity;\n"
                "         lighting *= calcShadow(MAX_POINT_LIGHTS, u_skyLight.vpMatrix * vec4(fragPosition.xyz, 1.0));\n"
                "         blendedColour += lighting;\n"
                "    }\n"

                "    for (int i = 0; i < u_lightCount && i < MAX_POINT_LIGHTS; ++i)\n"
                "    {\n"
                "        vec3 pointLightDirection = (u_pointLights[i].position - fragPosition.xyz);\n"
                "        float range = u_pointLights[i].range * u_pointLights[i].range;\n"
                "        float distance = lenSqr(pointLightDirection);\n"
                "        if(distance > range) continue;//fragment not in light range\n"

                "        pointLightDirection *= u_pointLights[i].inverseRange;\n"
                "        float falloff = clamp(1.0 - sqrt(dot(pointLightDirection, pointLightDirection)), 0.0, 1.0);\n"
                "        pointLightDirection = normalize(pointLightDirection);\n"
                "        vec3 lighting = calcLighting(normal, pointLightDirection, u_pointLights[i].diffuseColour.rgb, u_pointLights[i].specularColour.rgb, falloff) * u_pointLights[i].intensity;\n" \
                "        if(u_pointLights[i].castShadows) lighting *= (calcShadow(i, u_pointLights[i].vpMatrix * vec4(fragPosition.xyz, 1.0)) * (distance / range));\n"
                "        blendedColour += lighting;\n"
                "        //blendedColour *= calcShadow(i, max(0.05 * (1.0 - dot(normal, pointLightDirection)), 0.005), u_pointLights[i].vpMatrix * vec4(fragPosition.xyz, 1.0));\n"
                "    }\n"


                "    //blendedColour *= texture(u_aoMap, v_texCoord).rgb;\n"
                "    blendedColour = mix(blendedColour, diffuse.rgb, mask.b);\n"
                "    //blendedColour = mix(getReflection(normal), blendedColour, mask.a);\n"
                "    blendedColour += getReflection(normal) * (1.0 - mask.a);\n"

                /*"    vec3 illumination  = texture(u_illuminationMap, v_texCoord).rgb;\n"
                "    illumination.r = blendOverlay(blendedColour.r, illumination.r);\n"
                "    illumination.g = blendOverlay(blendedColour.g, illumination.g);\n"
                "    illumination.b = blendOverlay(blendedColour.b, illumination.b);\n"
                "    blendedColour += illumination;\n"*/

                "    blendedColour += texture(u_illuminationMap, v_texCoord).rgb * (2.0 - u_skyLight.intensity);\n"
                "#if defined (WATER)\n"
                "    blendedColour = calcWater(blendedColour, fragPosition.xyz);\n"
                "#endif\n"
                "    gl_FragDepth = texture(u_depthBuffer, v_texCoord).r;\n"
                "    fragOut = vec4(blendedColour, diffuse.a);//vec4(vec3(texture(u_depthMaps, vec3(v_texCoord, 1.0)).r), 1.0);//\n"
                "}";
        }
    }
}

#endif //XY_MESH_LIGHTING_HPP_