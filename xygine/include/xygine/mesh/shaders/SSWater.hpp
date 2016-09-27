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

#ifndef XY_MESH_WATER_HPP_
#define XY_MESH_WATER_HPP_

#include <string>

namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string WaterFragment =
                "#version 120\n"

                "uniform sampler2D u_diffuseMap;\n"
                "uniform sampler2D u_positionMap;\n"

                "uniform sampler2D u_surfaceMap;\n"

                "uniform float u_time;\n"
                "uniform float u_nearPlane = 0.86;\n"
                "uniform float u_waterLevel = 250.0;\n"
                "uniform float u_surfaceStrength = 0.15;\n"
                "uniform vec4 u_waterColour = vec4(0.1, 0.4, 0.9, 1.0);\n"

                "uniform vec3 u_cameraWorldPosition;\n"

                "uniform vec4 u_lightDiffuse = vec4(1.0);\n"
                "uniform vec4 u_lightSpecular = vec4(1.0);\n"
                "uniform vec3 u_lightDirection = vec3(0.0, 1.0, 0.0);\n"

                /*this is a kludge because of the inverted Y coord*/
                "const float sceneHeight = 1080.0;\n"
                "const float tau = 6.2831;\n"
                "const float waveFrequency = 0.5;\n"

                "vec3 calcSurfaceNormal(vec2 texCoord)\n"
                "{\n"
                "        float n1 = texture2D(u_surfaceMap, texCoord + (vec2(-1.0, 0.0) / 256.0)).r * tau;\n"
                "        n1 = sin((waveFrequency * u_time) + n1) * 0.5 + 0.5;\n"

                "        float n2 = texture2D(u_surfaceMap, texCoord + (vec2(1.0, 0.0) / 256.0)).r * tau;\n"
                "        n2 = sin((waveFrequency * u_time) + n2) * 0.5 + 0.5;\n"

                "        float n3 = texture2D(u_surfaceMap, texCoord + (vec2(0.0, -1.0) / 256.0)).r * tau;\n"
                "        n3 = sin((waveFrequency * u_time) + n3) * 0.5 + 0.5;\n"

                "        float n4 = texture2D(u_surfaceMap, texCoord + (vec2(0.0, 1.0) / 256.0)).r * tau;\n"
                "        n4 = sin((waveFrequency * u_time) + n4) * 0.5 + 0.5;\n"

                "        return normalize(vec3(n1 - n2, -1.0, n3 - n4));\n"
                "}\n"


                "void main()\n"
                "{\n"
                "    float waterLevel = sceneHeight - u_waterLevel;\n"
                "    vec4 position = texture2D(u_positionMap, vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y));\n"

                "    if(u_cameraWorldPosition.y < u_waterLevel)\n"
                "    {\n"
                "        //TODO distance based fogging, and rendering surface as seen from below\n"
                "        gl_FragColor = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n"
                "        return;\n"
                "    }\n"

                /*remember SFML Y coord is inverted!*/
                "    if(position.y > waterLevel)\n"
                "    {\n"

                "        float reflectionCutoff = (u_waterLevel / sceneHeight) * u_nearPlane;\n"

                "        vec3 eyeVec = normalize(position.xyz - u_cameraWorldPosition);\n"
                "        float t = (u_waterLevel - position.y) / eyeVec.y;\n"
                "        vec3 surfacePoint = u_cameraWorldPosition - eyeVec * t;\n"
                "        eyeVec = normalize(u_cameraWorldPosition - surfacePoint);\n"
                "        vec2 texCoord = (surfacePoint.xz + eyeVec.xz * 0.1) * 0.001;\n"
                "        vec3 surface = calcSurfaceNormal(texCoord);\n"

                "        float depth = clamp(surfacePoint.y - position.y, 0.0, 1.0);\n"
                "        float refractOffset = sin((gl_TexCoord[0].y * 50.0 + u_time)) * (0.001 * depth);\n"

                "        if(gl_TexCoord[0].y > reflectionCutoff)\n"
                "        {\n"
                "            texCoord = gl_TexCoord[0].xy + (surface.xz * u_surfaceStrength);\n"
                "        }\n"
                "        else\n"
                "        {\n"
                "            texCoord = gl_TexCoord[0].xy + refractOffset;\n"
                "        }\n"
                "        vec4 refractPosition = texture2D(u_positionMap, vec2(texCoord.x, 1.0 - texCoord.y));\n"
                "        vec4 refraction = texture2D(u_diffuseMap, texCoord);\n"
                "        refraction.rgb *= u_waterColour.rgb * (1.0 - refractPosition.a) * depth;\n"
                /*TODO diminish the amount of light with distance*/
                "        refraction.rgb *= u_lightDiffuse.rgb;\n"

                /*the camera is fixed along the z-axis so we can save time by flipping the buffer vertically*/
                "        float reflectionOffset = ((waterLevel - u_waterLevel) / sceneHeight);\n"
                "        vec3 reflectPosition = texture2D(u_positionMap, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + reflectionOffset)).rgb;\n"
                "        vec4 reflection = vec4(0.0);\n"
                "        if(reflectPosition.y <= waterLevel - 4.0)\n"
                "        {\n"
                "            reflection = texture2D(u_diffuseMap, vec2(gl_TexCoord[0].x, 1.0 - (gl_TexCoord[0].y + reflectionOffset)));\n"
                "        }\n"

                "        vec4 colour = refraction;\n"
                "        vec3 halfVec = normalize(eyeVec - u_lightDirection);\n"
                "        float specularAngle = clamp(dot(surface, halfVec), 0.0, 1.0) * 2.0;\n"
                "        vec3 specColour = vec3(u_lightSpecular.rgb * vec3(pow(specularAngle, 250.0)));\n"

                "        if(gl_TexCoord[0].y > reflectionCutoff)\n"
                "        {\n"
                "            colour.rgb = mix(refraction.rgb, reflection.rgb, (1.0 - refractPosition.a));\n"
                "            colour.rgb += specColour;\n"
                "        }\n"
                "        gl_FragColor = colour;\n"
                "    }\n"
                "    else\n"
                "    {\n"
                "        gl_FragColor = texture2D(u_diffuseMap, gl_TexCoord[0].xy);\n"
                "    }\n"
                "}";
        }
    }
}

#endif //XY_MESH_WATER_HPP_