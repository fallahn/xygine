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

//shaders used for visualising normals on a model

#ifndef XY_GEOM_VIS_HPP_
#define XY_GEOM_VIS_HPP_

#include <string>
namespace xy
{
    namespace Shader
    {
        namespace Mesh
        {
            static const std::string geomVisVert =
                "in vec4 a_position;\n" \
                "in vec3 a_normal;\n" \
                "#if defined(BUMP)\n" \
                "in vec3 a_tangent;\n" \
                "in vec3 a_bitangent;\n" \
                "#endif\n" \
                "out VS_OUT\n" \
                "{\n" \
                "    vec3 normal;\n" \
                "    vec3 tangent;\n" \
                "    vec3 bitangent;\n" \
                "} vs_out;\n" \
                "void main()\n" \
                "{\n" \
                "    gl_Position = a_position;\n" \
                "    vs_out.normal = a_normal;\n" \
                "#if defined(BUMP)\n" \
                "    vs_out.tangent = a_tangent;\n" \
                "    vs_out.bitangent = a_bitangent;\n" \
                "#endif\n" \
                "}\n";

            static const std::string geomVisGeom =
                "layout (triangles) in;\n" \
                "layout (line_strip, max_vertices = 6) out;\n"
                "in VS_OUT\n" \
                "{\n" \
                "    vec3 normal;\n" \
                "    vec3 tangent;\n" \
                "    vec3 bitangent;\n" \
                "}gs_in[];\n" \


                "uniform mat4 u_worldViewMatrix;\n" \
                "struct Point\n" \
                "{\n" \
                "    vec3 position;\n" \
                "    float padding;\n" \
                "};\n" \

                "layout (std140) uniform u_matrixBlock\n" \
                "{\n" \
                "    mat4 u_viewMatrix;\n" \
                "    mat4 u_projectionMatrix;\n" \
                "    Point u_pointLightPositions[8];\n" \
                "    vec3 u_cameraWorldPosition;\n" \
                "};\n" \

                "out vec3 vertColour;\n" \
                "const float lineLength = 10.3;\n" \
                "void createLines(int index)\n" \
                "{\n" \
                "    mat4 worldViewProjectionMatrix = u_projectionMatrix * u_worldViewMatrix;\n" \
                "    gl_Position = worldViewProjectionMatrix * gl_in[index].gl_Position;\n" \
                "    vertColour = vec3(0.0, 0.0, 1.0);\n" \
                "    EmitVertex();\n" \
                "    gl_Position = worldViewProjectionMatrix * vec4(gl_in[index].gl_Position.xyz + gs_in[index].normal * lineLength, 1.0);\n" \
                "    EmitVertex();\n" \
                "#if defined(BUMP)\n" \
                "    gl_Position = worldViewProjectionMatrix * gl_in[index].gl_Position;\n" \
                "    vertColour = vec3(1.0, 0.0, 0.0);\n" \
                "    EmitVertex();\n" \
                "    gl_Position = worldViewProjectionMatrix * vec4(gl_in[index].gl_Position.xyz + gs_in[index].tangent * lineLength, 1.0);\n" \
                "    EmitVertex();\n" \
                "    gl_Position = worldViewProjectionMatrix * gl_in[index].gl_Position;\n" \
                "    vertColour = vec3(0.0, 1.0, 0.0);\n" \
                "    EmitVertex();\n" \
                "    gl_Position = worldViewProjectionMatrix * vec4(gl_in[index].gl_Position.xyz + gs_in[index].bitangent * lineLength, 1.0);\n" \
                "    EmitVertex();\n" \
                "#endif\n" \
                "    EndPrimitive();\n" \
                "}\n" \
                "void main()\n" \
                "{\n" \
                "    createLines(0);\n" \
                "    createLines(1);\n" \
                "    createLines(2);\n" \
                "}\n";
            static const std::string geomVisFrag =
                "in vec3 vertColour;\n" \
                "out vec4 colour;\n" \
                "void main()\n" \
                "{\n" \
                "    colour = vec4(vertColour.r, vertColour.g, vertColour.b, 1.0);\n" \
                "}\n";
        }
    }
}

#define GEOM_VERT "#version 150\n#define BUMP\n" + xy::Shader::Mesh::geomVisVert
#define GEOM_GEOM "#version 150\n#define BUMP\n" + xy::Shader::Mesh::geomVisGeom
#define GEOM_FRAG "#version 150\n#define BUMP\n" + xy::Shader::Mesh::geomVisFrag

#endif //XY_GEOM_VIS_HPP_