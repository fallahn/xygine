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

#ifndef XY_MESH_FORWARD_HPP_
#define XY_MESH_FORWARD_HPP_

#include <xygine/mesh/shaders/DeferredRenderer.hpp>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string ForwardFrag = R"(
                #define MAX_POINT_LIGHTS 8

                struct PointLight
                {
                    vec4 diffuseColour;
                    vec4 specularColour;
                    float inverseRange;
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
                #if defined(BUMPED)
                uniform sampler2D u_normalMap;
                #endif
                #endif
                uniform sampler2D u_depthMap;
                uniform int u_lightCount = MAX_POINT_LIGHTS;
                uniform sampler2DArray u_depthMaps;

                #if !defined(BUMPED)
                in vec3 v_normalVector;
                #else
                in vec3 v_tbn[3];
                #endif

                #if defined (TEXTURED)
                in vec2 v_texCoord;
                #endif

                in vec3 v_worldPosition;

                #if defined(VERTEX_COLOUR)
                in vec3 v_colour;
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

                float lenSqr(vec3 vector)
                {
                    return dot(vector, vector);
                }

                void main()
                {
                    vec3 fragPosition = v_worldPosition;
                    mask = texture(u_maskMap, v_texCoord);
                #if !defined(BUMPED)
                    vec3 normal = normalize(v_normalVector);
                #else
                    vec3 normal = normalize(v_tbn[0] * normal.x + v_tbn[1] * normal.y + v_tbn[2] * normal.z).rgb; //TODO move to vertex shader?
                #endif
                    vec4 diffuse = texture(u_diffuseMap, v_texCoord);
                    diffuseColour = diffuse.rgb;
                    vec3 blendedColour = diffuse.rgb * u_ambientColour.rgb;
                    eyeDirection = normalize(u_cameraWorldPosition - fragPosition);

                    if(u_skyLight.intensity > 0.0)
                    {
                        blendedColour += calcLighting(normal, normalize(-u_skyLight.direction), u_skyLight.diffuseColour.rgb, u_skyLight.specularColour.rgb, 1.0) * u_skyLight.intensity;
                    }
                    
                    blendedColour = mix(blendedColour.rgb, diffuse.rgb, mask.b);
                    fragOut = vec4(blendedColour.rgb, diffuse.a);
                })";
        }
    }
}

#define ALPHABLEND_TEST_FRAG "#version 150\n#define TEXTURED\n" + xy::Shader::Mesh::ForwardFrag

#endif //XY_MESH_FORWARD_HPP_