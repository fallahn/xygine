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
                "    vec3 u_cameraPosition;\n"
                "};\n"

                "uniform sampler2D u_diffuseMap;\n"
                "uniform sampler2D u_normalMap;\n"

                "in vec2 v_texCoord;\n"

                "out vec4 fragOut;\n"

                "void main()\n"
                "{\n"
                "    fragOut = texture(u_normalMap, v_texCoord);\n"
                "}";
        }
    }
}

#endif //XY_MESH_LIGHTING_HPP_