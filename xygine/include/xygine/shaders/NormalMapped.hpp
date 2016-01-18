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

#ifndef XY_SHADER_NORMALMAPPED_HPP_
#define XY_SHADER_NORMALMAPPED_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace NormalMapped
        {
            /*static const std::string textured =
            "#version 120\n#define TEXTURED\n";
            static const std::string coloured =
            "#version 120\n#define COLOURED\n";*/

            static const std::string vertex =
                "#version 120\n" \
                "uniform vec3 u_lightWorldPosition = vec3(960.0, 540.0, 80.0);\n" \
                "uniform vec3 u_cameraWorldPosition = vec3(960.0, 540.0, 480.0);\n" \
                "uniform mat4 u_inverseWorldViewMatrix;\n" \

                "varying vec3 v_eyeDirection;\n" \
                "varying vec3 v_lightDirection;\n" \

                "const vec3 tangent = vec3(1.0, 0.0, 0.0);\n" \
                "const vec3 normal = vec3(0.0, 0.0, 1.0);\n" \

                "void main()\n" \
                "{\n" \
                "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
                "    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;\n" \
                "    gl_FrontColor = gl_Color;\n" \

                "    mat3 normalMatrix = transpose(mat3(u_inverseWorldViewMatrix));\n" \
                "    vec3 n = normalize(normalMatrix * normal);\n" \
                "    vec3 t = normalize(normalMatrix * tangent);\n" \
                "    vec3 b = cross(n,t);\n" \
                "    mat3 tangentSpaceTransformMatrix = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);\n" \

                "    vec3 viewVertex = vec3(gl_ModelViewMatrix * gl_Vertex);\n" \
                "    vec3 viewLightDirection = vec3(gl_ModelViewMatrix * vec4(u_lightWorldPosition, 1.0)) - viewVertex;\n" \

                "    v_lightDirection = tangentSpaceTransformMatrix * normalize(viewLightDirection);\n" \
                "    v_eyeDirection = tangentSpaceTransformMatrix * ((gl_ModelViewMatrix * vec4(u_cameraWorldPosition, 1.0)).xyz - viewVertex);\n" \
                "}";

            static const std::string fragment =
                "#version 120\n" \
                "#define TEXTURED\n" \
                "#if defined(TEXTURED)\n" \
                "uniform sampler2D u_diffuseMap;\n" \
                "#endif\n" \
                "uniform sampler2D u_normalMap;\n" \
                "uniform float u_lightIntensity = 0.86;\n" \
                "uniform vec3 u_ambientColour = vec3 (0.4, 0.4, 0.4);\n" \

                "varying vec3 v_eyeDirection;\n" \
                "varying vec3 v_lightDirection;\n" \

                "const vec3 lightColour = vec3(1.0, 0.98, 0.45);\n" \
                "const float inverseRange = 0.005;\n" \

                "void main()\n" \
                "{\n" \
                "#if defined(TEXTURED)\n" \
                "    vec4 diffuseColour = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n" \
                "#elif defined(COLOURED)\n" \
                "    vec4 diffuseColour = gl_Color;\n" \
                "#endif\n" \
                "    vec3 normalVector = texture2D(u_normalMap, gl_TexCoord[0].xy).rgb * 2.0 - 1.0;\n" \

                "    vec3 blendedColour = diffuseColour.rgb * u_ambientColour;\n" \
                "    float diffuseAmount = max(dot(normalVector, normalize(v_lightDirection)), 0.0);\n" \
                /*multiply by falloff*/
                "    vec3 falloffDirection = v_lightDirection * inverseRange;\n" \
                "    float falloff = clamp(1.0 - dot(falloffDirection, falloffDirection), 0.0, 1.0);\n" \
                "    blendedColour += (lightColour * u_lightIntensity) * diffuseColour.rgb * diffuseAmount;// * falloff;\n" \

                /*"    vec3 eyeDirection = normalize(v_lightDirection);\n" \
                "    vec3 halfVec = normalize(v_lightDirection + eyeDirection);\n" \
                "    float specularAngle = clamp(dot(normalVector, halfVec), 0.0, 1.0);\n" \
                "    vec3 specularColour = vec3(pow(specularAngle, 196.0));// * falloff;\n" \
                "    blendedColour += specularColour;\n" \*/

                "    gl_FragColor.rgb = blendedColour;\n" \
                "    gl_FragColor.a = diffuseColour.a;\n" \
                "#if defined(TEXTURED)\n" \
                "    gl_FragColor *= gl_Color;\n" \
                "#endif\n" \
                "}";
        }
    }//namespace Shader
}//namespace xy

#endif //XY_SHADER_NORMALMAPPED_HPP_
