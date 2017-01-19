/*********************************************************************
© Matt Marchant 2014 - 2017
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

#ifndef XY_MESH_FORWARD_HPP_
#define XY_MESH_FORWARD_HPP_

#include <xygine/mesh/shaders/DeferredRenderer.hpp>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string ForwardFragment = R"(
                #define MAX_POINT_LIGHTS 8

                struct PointLight
                {
                    vec4 diffuseColour;
                    vec4 specularColour;
                    float inverseRange;
                    float range;
                    float intensity;
                    bool castShadows;
                    vec3 position;
                    float padding;
                    mat4 vpMatrix;
                };

                struct SkyLight
                {
                    vec4 diffuseColour;
                    vec4 specularColour;
                    vec3 direction;
                    float padding;
                    mat4 vpMatrix;
                    float intensity;
                };

                layout (std140) uniform u_lightBlock
                {
                    PointLight u_pointLights[MAX_POINT_LIGHTS];
                    vec4 u_ambientColour;
                    vec3 u_cameraWorldPosition;
                    float padding;
                    SkyLight u_skyLight;
                };

                #if defined(TEXTURED)
                uniform sampler2D u_diffuseMap;
                uniform sampler2D u_maskMap;
                #if defined(BUMP)
                uniform sampler2D u_normalMap;
                #endif
                #else
                uniform vec4 u_maskColour = vec4(vec3(0.0), 1.0);
                #endif
                uniform vec4 u_colour = vec4(1.0);
                uniform sampler2D u_depthMap;
                uniform int u_lightcount = MAX_POINT_LIGHTS;
                uniform sampler2DArray u_depthMaps;

                #if !defined(BUMP)
                in vec3 v_normalVector;
                #else
                in vec3 v_tbn[3];
                #endif

                #if defined (TEXTURED) || defined (BUMP)
                in vec2 v_texCoord;
                #endif

                in vec3 v_worldPosition;

                #if defined(VERTEX_COLOUR)
                in vec4 v_colour;
                #endif

                out vec4 fragOut;

                vec3 diffuseColour;
                vec4 mask;
                vec3 eyeDirection;
                vec3 calcLighting(vec3 normal, vec3 lightDirection, vec3 lightDiffuse, vec3 lightSpecular, float falloff)
                {
                    float diffuseAmount = max(dot(normal, lightDirection), 0.0);
                    diffuseAmount = pow((diffuseAmount * 0.5) + 0.5, 2.0);
                    vec3 mixedColour = diffuseColour * lightDiffuse * diffuseAmount * falloff;

                    vec3 halfVec = normalize(eyeDirection + lightDirection);
                    float specularAngle = clamp(dot(normal, halfVec), 0.0, 1.0);
                    vec3 specularColour = lightSpecular * vec3(pow(specularAngle, (254.0 * mask.r) + 1.0)) * falloff;

                    return mixedColour + (specularColour * mask.g);
                }

                const vec2 kernel[16] = vec2[](
                    vec2(-0.94201624, -0.39906216),
                    vec2(0.94558609, -0.76890725),
                    vec2(-0.094184101, -0.92938870),
                    vec2(0.34495938, 0.29387760),
                    vec2(-0.91588581, 0.45771432),
                    vec2(-0.81544232, -0.87912464),
                    vec2(-0.38277543, 0.27676845),
                    vec2(0.97484398, 0.75648379),
                    vec2(0.44323325, -0.97511554),
                    vec2(0.53742981, -0.47373420),
                    vec2(-0.26496911, -0.41893023),
                    vec2(0.79197514, 0.19090188),
                    vec2(-0.24188840, 0.99706507),
                    vec2(-0.81409955, 0.91437590),
                    vec2(0.19984126, 0.78641367),
                    vec2(0.14383161, -0.14100790)
                );
                const int filterSize = 3;
                float calcShadow(int layer, vec4 position)
                {
                    float bias = 0.001;
                    vec3 projectionCoords = position.xyz / position.w;
                    projectionCoords = projectionCoords * 0.5 + 0.5;
                    if(projectionCoords.z > 1.0) return 1.0;
                    float shadow = 0.0;
                    vec2 texelSize = 1.0 / textureSize(u_depthMaps, 0).xy;
                    for(int x = 0; x < filterSize; ++x)
                    {
                        for(int y = 0; y < filterSize; ++y)
                        {
                            float pcfDepth = texture(u_depthMaps, vec3(projectionCoords.xy + kernel[y * filterSize + x] * texelSize, float(layer))).r;
                            shadow += (projectionCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
                        }
                    }
                    return 1.0 - (shadow / 9.0);
                }

                float lenSqr(vec3 vector)
                {
                    return dot(vector, vector);
                }

                void main()
                {
                    vec3 fragPosition = v_worldPosition;
                    
                #if !defined(BUMP)
                    vec3 normal = normalize(v_normalVector);
                #else
                    vec3 texNormal = texture(u_normalMap, v_texCoord).rgb * 2.0 - 1.0;
                    vec3 normal = normalize(v_tbn[0] * texNormal.x + v_tbn[1] * texNormal.y + v_tbn[2] * texNormal.z).rgb;
                #endif
                #if defined(TEXTURED)
                    vec4 diffuse = texture(u_diffuseMap, v_texCoord) * u_colour;
                    mask = texture(u_maskMap, v_texCoord);
                #else
                    vec4 diffuse = u_colour;
                    mask = u_maskColour;
                #endif
                #if defined(VERTEX_COLOUR)
                    diffuse *= v_colour;
                #endif
                    diffuseColour = diffuse.rgb;
                    vec3 blendedColour = diffuse.rgb * u_ambientColour.rgb;
                    eyeDirection = normalize(u_cameraWorldPosition - fragPosition);

                    if(u_skyLight.intensity > 0.0)
                    {
                        vec3 lighting = calcLighting(normal, normalize(-u_skyLight.direction), u_skyLight.diffuseColour.rgb, u_skyLight.specularColour.rgb, 1.0) * u_skyLight.intensity;
                        lighting *= calcShadow(MAX_POINT_LIGHTS, u_skyLight.vpMatrix * vec4(fragPosition, 1.0));
                        blendedColour += lighting;
                    }
                    
                    for(int i = 0; i < u_lightcount && i < MAX_POINT_LIGHTS; ++i)
                    {
                        vec3 pointLightDirection = (u_pointLights[i].position - fragPosition);
                        float range = u_pointLights[i].range * u_pointLights[i].range;
                        float distance = lenSqr(pointLightDirection);
                        if(distance > range) continue;

                        pointLightDirection *= u_pointLights[i].inverseRange;
                        float falloff = clamp(1.0 - sqrt(lenSqr(pointLightDirection)), 0.0, 1.0);
                        pointLightDirection = normalize(pointLightDirection);
                        
                        vec3 lighting = calcLighting(normal, pointLightDirection, u_pointLights[i].diffuseColour.rgb, u_pointLights[i].specularColour.rgb, falloff) * u_pointLights[i].intensity;
                        if(u_pointLights[i].castShadows) lighting *= (calcShadow(i, u_pointLights[i].vpMatrix * vec4(fragPosition.xyz, 1.0)) * (distance / range));                        
                        blendedColour += lighting;
                    }       

                    blendedColour = mix(blendedColour.rgb, diffuse.rgb, mask.b);
                    fragOut = vec4(blendedColour.rgb, diffuse.a);
                })";
        }
    }
}


#define ALPHABLEND_COLOURED_VERTEX DEFERRED_COLOURED_VERTEX
#define ALPHABLEND_COLOURED_BUMPED_VERTEX DEFERRED_COLOURED_BUMPED_VERTEX
#define ALPHABLEND_TEXTURED_VERTEX DEFERRED_TEXTURED_VERTEX
#define ALPHABLEND_TEXTURED_BUMPED_VERTEX DEFERRED_TEXTURED_BUMPED_VERTEX
#define ALPHABLEND_TEXTURED_BUMPED_SKINNED_VERTEX DEFERRED_TEXTURED_BUMPED_SKINNED_VERTEX

#define ALPHABLEND_COLOURED_FRAGMENT "#version 150\n" + xy::Shader::Mesh::ForwardFragment
#define ALPHABLEND_COLOURED_BUMPED_FRAGMENT "#version 150\n#define BUMP\n" + xy::Shader::Mesh::ForwardFragment
#define ALPHABLEND_TEXTURED_FRAGMENT "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::ForwardFragment
#define ALPHABLEND_TEXTURED_BUMPED_FRAGMENT "#version 150\n#define BUMP\n#define TEXTURED\n" + xy::Shader::Mesh::ForwardFragment

#endif //XY_MESH_FORWARD_HPP_