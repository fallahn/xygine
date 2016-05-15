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

#ifndef XY_MODEL_COMPONENT_HPP_
#define XY_MODEL_COMPONENT_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/VertexAttribBinding.hpp>

#include <SFML/Graphics/Glsl.hpp>

#include <glm/mat4x4.hpp>

#include <vector>
#include <map>

namespace xy
{
    class Mesh;
    /*!
    \brief Allows attaching a 3D mesh and a set of materials to
    an entity in the scene. While the model is fully 3D rendered
    the camera of the scene is still constrained to the 2D world
    giving the scene a '2.5D' appearance.
    */
    class Model final : public xy::Component
    {
        friend class MeshRenderer;
    public:
        /*!
        \brief Constructor.
        Model components should not (and cannot) be created via the
        conventional xy::Component::create() function, but rather with
        the create() function of a valid MeshRenderer.
        \see MeshRenderer
        */
        Model(MessageBus&, const Mesh&, const MeshRenderer::Lock&);
        ~Model() = default;

        Component::Type type() const override { return Component::Type::Mesh; }
        void entityUpdate(Entity&, float) override;

        /*!
        \brief Sets the apparent depth of the model in the scene.
        Provided for convenience this should normally be left at 0
        and only altered if needed. As the rest of the scene is still
        two-dimensional collision may appear incorrect with extreme depth
        settings. This also does not take in to account the mesh's local
        z values, which can cause the appearance of offsetting hte model.
        In these cases setting the depth of the model can be used to
        correct this.
        */
        void setDepth(float depth) { m_depth = depth; }

        /*!
        \brief Sets the base material used but the model.
        \param material Reference to Material to apply
        \param applyToAll Set to true if the Material should be
        applied to all SubMeshes in the model
        */
        void setBaseMaterial(const Material& material, bool applyToAll = true);

        /*!
        \brief Sets the material used by a SubMesh at the given index (if it exists)
        */
        void setSubMaterial(const Material&, std::size_t);

    private:

        glm::mat4 m_worldMatrix;
        const Mesh& m_mesh;
        float m_depth;

        const Material* m_material;
        std::vector<const Material*> m_subMaterials;
        std::map<const Material*, VertexAttribBinding> m_vaoBindings;

        void draw(const glm::mat4&) const;

        void updateVertexAttribs(const Material* oldMat, const Material* newMat);
    };
}

#endif //XY_MODEL_COMPONENT_HPP_