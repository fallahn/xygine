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

#ifndef XY_MESH_LIGHTING_HPP_
#define XY_MESH_LIGHTING_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
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
                "#define MAX_POINT_LIGHTS 8\n"
                "struct PointLight\n"
                "{\n"
                "    vec4 diffuseColour;\n"
                "    vec4 specularColour;\n"
                "    float inverseRange;\n"
                "    float intensity;\n"
                "    vec3 position;\n"
                "    vec3 padding;\n"
                "};\n"

                "layout (std140) uniform u_lightBlock\n"
                "{\n"
                "    PointLight u_pointLights[MAX_POINT_LIGHTS];\n"
                "    vec3 u_cameraWorldPosition;\n"
                "};\n"

                "uniform mat4 u_viewMatrix;\n"
                "uniform sampler2D u_diffuseMap;\n"
                "uniform sampler2D u_normalMap;\n"
                "uniform sampler2D u_positionMap;\n"
                "uniform sampler2D u_aoMap;\n"
                "uniform vec4 u_ambientColour = vec4(0.2, 0.2, 0.2, 1.0);\n"

                "in vec2 v_texCoord;\n"

                "out vec4 fragOut;\n"

                "vec3 diffuseColour;\n"
                "vec3 eyeDirection;\n"
                "vec3 calcLighting(vec3 normal, vec3 lightDirection, vec3 lightDiffuse, vec3 lightSpec, float falloff)\n"
                "{\n"
                "    float diffuseAmount = max(dot(normal, lightDirection), 0.0);\n"
                "    diffuseAmount = pow((diffuseAmount * 0.5) + 0.5, 2.0);\n"
                "    vec3 mixedColour = diffuseColour * lightDiffuse * diffuseAmount * falloff;\n"

                "    vec3 halfVec = normalize(eyeDirection + lightDirection);\n"
                "    float specularAngle = clamp(dot(normal, halfVec), 0.0, 1.0);\n"
                "    vec3 specularColour = lightSpec * vec3(pow(specularAngle, 180.0)) * falloff;\n"

                "    return mixedColour + specularColour;\n"
                "}\n" \

                "void main()\n"
                "{\n"
                "    vec3 fragPosition = texture(u_positionMap, v_texCoord).xyz;\n"
                "    vec3 normal = texture(u_normalMap, v_texCoord).rgb;\n"
                "    vec4 diffuse = texture(u_diffuseMap, v_texCoord);\n"
                "    diffuseColour = diffuse.rgb;\n"
                "    vec3 blendedColour = diffuse.rgb * u_ambientColour.rgb;\n"
                "    eyeDirection = normalize(vec3(960.0, 540.0, 2015.0) - fragPosition);\n"

                "    for (int i = 0; i < MAX_POINT_LIGHTS; ++i)\n"
                "    {\n"
                "        vec3 pointLightDirection = (vec3(960.0, 540.0, 200.0) - fragPosition) * 0.0008;//u_pointLights[i].inverseRange;\n"
                "        float falloff = clamp(1.0 - sqrt(dot(pointLightDirection, pointLightDirection)), 0.0, 1.0);\n"
                "        blendedColour += calcLighting(normal, normalize(pointLightDirection), vec3(0.3, 0.5, 1.0), vec3(1.0), falloff);// * u_pointLights[i].intensity;\n" \
                "    }\n"


                "    fragOut = vec4(blendedColour * texture(u_aoMap, v_texCoord).rgb, diffuse.a);//\n"
                "}";
        }
    }
}

#endif //XY_MESH_LIGHTING_HPP_