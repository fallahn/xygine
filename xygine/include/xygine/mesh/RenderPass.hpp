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
    enum BlendFunc
    {
        ZERO                    = GL_ZERO,
        ONE                     = GL_ONE,
        SRC_COLOUR              = GL_SRC_COLOR,
        ONE_MINUS_SOURCE_COLOUR = GL_ONE_MINUS_SRC_COLOR,
        DEST_COLOUR             = GL_DST_COLOR,
        ONE_MINUS_DEST_COLOUR   = GL_ONE_MINUS_DST_COLOR,
        SRC_ALPHA               = GL_SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA     = GL_ONE_MINUS_SRC_ALPHA,
        DEST_ALPHA              = GL_DST_ALPHA,
        ONE_MINUS_DEST_ALPHA    = GL_ONE_MINUS_DST_ALPHA,
        CONST_ALPHA             = GL_CONSTANT_ALPHA,
        ONE_MINUS_CONST_ALPHA   = GL_ONE_MINUS_CONSTANT_ALPHA
    };

    enum DepthFunc
    {
        NEVER    = GL_NEVER,
        LESS     = GL_LESS,
        EQUAL    = GL_EQUAL,
        LEQUAL   = GL_LEQUAL,
        GREATER  = GL_GREATER,
        NOTEQUAL = GL_NOTEQUAL,
        GEQUAL   = GL_GEQUAL,
        ALWAYS   = GL_ALWAYS
    };
    
    enum Winding
    {
        CLOCKWISE         = GL_CW,
        COUNTER_CLOCKWISE = GL_CCW
    };

    enum CullFace
    {
        FRONT          = GL_FRONT,
        BACK           = GL_BACK,
        FRONT_AND_BACK = GL_FRONT_AND_BACK
    };

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
        \brief Returns the shader used by this pass
        */
        sf::Shader& getShader() const { return m_shader; }

        /*!
        \brief Returns the attribute ID of the requested vertex attribute if it
        exists in the pass shader, else returns -1
        */
        VertexAttribID getVertexAttributeID(const std::string&) const;

        /*!
        \brief Sets the Source and Destination blend function paramters for this pass
        \param src The source function
        \param dst The destination function
        */
        void setBlendFunc(BlendFunc src, BlendFunc dst)
        {
            m_blendFuncSrc = src;
            m_blendFuncDst = dst;
        }

        /*!
        \brief Sets the depth function to be used by this pass
        */
        void setDepthFunc(DepthFunc func)
        {
            m_depthFunc = func;
        }

        /*!
        \brief Sets the vertex winding direction which defines a front
        facing polygon when this pass is rendered.
        */
        void setWinding(Winding winding) { m_winding = winding; }

        /*!
        \brief Sets which face direction should be culled when drawing with
        this pass
        */
        void setCullFace(CullFace face) { m_cullface = face; }

        /*!
        \brief Returns the uniform ID of the joint array if this pass
        supports skinning, else returns -1
        */
        UniformID getSkinID() const { return m_skinID; }

    private:
        sf::Shader& m_shader;

        std::vector<MaterialProperty> m_properties;
        std::vector<std::pair<UniformBlockID, const UniformBuffer*>> m_uniformBuffers;

        std::unordered_map<std::string, VertexAttribID> m_vertexAttributes;

        BlendFunc m_blendFuncSrc;
        BlendFunc m_blendFuncDst;
        DepthFunc m_depthFunc;
        Winding m_winding;
        CullFace m_cullface;

        UniformID m_skinID;
    };
}

#endif //XY_RENDER_PASS_HPP_