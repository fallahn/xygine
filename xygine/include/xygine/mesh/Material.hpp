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

#ifndef XY_MATERIAL_HPP_
#define XY_MATERIAL_HPP_

#include <xygine/mesh/RenderPass.hpp>
#include <xygine/detail/Aliases.hpp>

#include <SFML/Graphics/Glsl.hpp>

#include <vector>
#include <set>

namespace xy
{
    class UniformBuffer;

    /*!
    \brief Materials are used when rendering Model components.
    One or more materials can be assigned to a single Model to
    be applied to the Model's Mesh and SubMeshes. Materials contain,
    at the very least, an OpenGL 3.2+ compatible shader, as well as
    allowing specifying multiple properties (in the guise of uniform
    values) which are applied to the shader, such as textures or matrices.
    Materials may also have UniformBuffers added to them for convenience,
    and are required for the view/projection matrices at the very least
    when rendering with the MeshRenderer. The UniformBuffer for these
    (and the optional buffer for lighting properties) can be retrieved
    with MeshRenderer::getMatrixUniforms(). This should be the first thing
    to check if a material is apprently not renderering. The 
    MaterialResource class is provided as a convenience for managing the
    lifetime of Materials as the are a shared resource. Bear also in mind
    that, as a shared resource, updating the property of a Material
    instance will affect ALL models to which thie Material is applied.
    */
    class XY_EXPORT_API Material final
    {
    public:
        /*!
        \brief Used to describe the appearance of Materials,
        useful for selecting the correct shader in multipass
        Materials
        */
        enum Description
        {
            Coloured = 0,
            ColouredBumped,
            ColouredSkinned,
            ColouredSkinnedBumped,
            Textured,
            TexturedBumped,
            TexturedSkinned,
            TexturedSkinnedBumped,
            VertexColoured,
            VertexColouredBumped,
            VertexColouredSkinned,
            VertexColouredSkinnedBumped
        };

        /*!
        \brief Constructor.
        \param Shader used to create the default render pass for this material
        */
        explicit Material(sf::Shader&);
        ~Material() = default;
        Material(const Material&) = delete;
        Material& operator = (const Material&) = delete;

        /*
        !\brief Adds a new pass which uses the given shader, mapped to the given ID
        */
        void addRenderPass(RenderPass::ID, sf::Shader&);

        /*!
        \brief Returns a pointer to the RenderPass with the given ID.
        \returns nullptr if RenderPass doesn't exist
        */
        RenderPass* getRenderPass(RenderPass::ID);

        /*!
        \brief Sets the current active RenderPass.
        All materials have a Default pass created with the shader supplied on 
        construction. Subsequent passes need to be activated before drawing in
        order to enable them, via this function.
        \param ID if the RenderPass to activate
        \returns true is activation successful, else false
        */
        bool setActivePass(RenderPass::ID) const;

        /*!
        \brief Binds this active pass for drawing
        */
        void bind() const;

        /*!
        \brief Adds a new property to the Material.
        All passes whose shader contain this property
        are updated.
        */
        void addProperty(const MaterialProperty&);
        
        /*!
        \brief Returns a Property with the given name if it
        exists in the active pass, else returns nullptr
        */
        MaterialProperty* getProperty(const std::string&);
        
        /*!
        \brief Add a UniformBuffer to be bound when drawing
        with the Material.
        UniformBuffers are useful for setting blocks of values
        used by multiple materials such as projection matrices.
        Care should be taken to make sure the UniformBuffer outlives
        the Material. All Materials passes are updated if the uniform
        block exists within the pass shader
        */
        void addUniformBuffer(const UniformBuffer&);

        /*!
        \brief Returns the attribute ID of the requested vertex attribute if it
        exists in the active RenderPass, else returns -1
        */
        VertexAttribID getVertexAttributeID(const std::string&) const;

        /*!
        \brief Returns a reference to the shader belonging to the active pass
        */
        sf::Shader& getShader() const;

        /*!
        \brief Returns the uniform ID of the joint matrices if the active pass
        supports skinning, else returns -1
        */
        UniformID getSkinID() const { return m_activePass->getSkinID(); }

    private:

        std::map<RenderPass::ID, RenderPass> m_passes;
        mutable RenderPass* m_activePass;
        std::vector<const UniformBuffer*> m_uniformBuffers;
    };
}

#endif //XY_MATERIAL_HPP_
