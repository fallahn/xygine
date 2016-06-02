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

#ifndef XY_RENDER_PASS_HPP_
#define XY_RENDER_PASS_HPP_

#include <xygine/Config.hpp>
#include <xygine/mesh/MaterialProperty.hpp>

#include <SFML/Graphics/Shader.hpp>

#include <vector>
#include <unordered_map>

namespace xy
{
    class UniformBuffer;

    /*!
    \brief Represents an OpenGL state used during a specific render pass
    */
    class XY_EXPORT_API RenderPass final
    {
    public:
        enum ID
        {
            Default,
            ShadowMap,
            Debug,
            Count
        };
        /*!
        \brief Constructor.
        \param Shader to use when rendering with this pass
        */
        explicit RenderPass(sf::Shader&);
        ~RenderPass() = default;
        //RenderPass(RenderPass&&) = default;

        /*!
        \brief Binds this pass for rendering
        */
        void bind() const;
        
        /*!
        \brief Adds a new property to the Material
        */
        void addProperty(const MaterialProperty&);
        
        /*!
        \brief Returns a Property with the given name if it
        exists, else returns nullptr
        */
        
        MaterialProperty* getProperty(const std::string&);
        
        /*!
        \brief Add a UniformBuffer to be bound when drawing
        with the Material.
        UniformBuffers are useful for setting blocks of values
        used by multiple materials such as projection matrices.
        Care should be taken to make sure the UniformBuffer outlives
        the Material.
        \returns true If the UniformBuffer name was found in the shader
        and the UniformBuffer was successfully added, else returns false
        */
        bool addUniformBuffer(const UniformBuffer&);
       
        /*!
        \brief Returns the shaderused by this pass
        */
        sf::Shader& getShader() const { return *m_shader; }

        /*!
        \brief Returns the attribute ID of the requested vertex attribute if it
        exists in the pass shader, else returns -1
        */
        VertexAttribID getVertexAttributeID(const std::string&) const;

    private:
        sf::Shader* m_shader;

        std::vector<MaterialProperty> m_properties;
        std::vector<std::pair<UniformBlockID, const UniformBuffer*>> m_uniformBuffers;

        std::unordered_map<std::string, VertexAttribID> m_vertexAttributes;
    };
}

#endif //XY_RENDER_PASS_HPP_