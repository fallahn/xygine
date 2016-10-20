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
#include <xygine/mesh/BoundingBox.hpp>

#include <SFML/Graphics/Glsl.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

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
    class XY_EXPORT_API Model final : public xy::Component
    {
        friend class MeshRenderer;
    public:
        /*!
        \brief Define around which axis to prepend a model's transform
        */
        enum class Axis
        {
            X, Y, Z
        };
        
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
        \brief Returns the 2D globally transformed AABB of the model based
        on the 3D AABB of the model's mesh.
        */
        sf::FloatRect globalBounds() const override { return m_globalBounds; }

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

        /*!
        \brief Adds a pre-transform rotation.
        Some modelling packages such as blender use different coordinate
        systems to OpenGL causing imported models to appear disoriented. Use
        this function to correct any rotation, which is applied to the model
        before the scene's world matrix transform.
        \param Axis around which to rotate
        \param rotation, in degrees, by which to rotate the model
        */
        void rotate(Model::Axis, float);

        /*!
        \brief Sets a pre-transform rotation.
        Some modelling packages such as blender use different coordinate
        systems to OpenGL causing imported models to appear disoriented. Use
        this function to correct any rotation, which is applied to the model
        before the scene's world matrix transform.
        \param 3 component vector representing the euler rotation, in degrees,
        to which the model's rotation should be set.
        */
        void setRotation(const sf::Vector3f&);

        /*!
        \brief Sets the position of the model relative to its entity.
        Meshes may have an abitrary origin which is, by default, aligned
        to the position of the model's entity. This function translates
        the model relative to the entity position, so that models may be
        aligned as required.
        \param Position 3-component vector representing the position to
        set the model relative to the entity to which it is attached.
        */
        void setPosition(const sf::Vector3f&);

        /*!
        \brief Sets the scale of the model's mesh.
        Because units in xygine are generally used to match pixels as
        screen coordinates many meshes are modelled with different units in
        mind and will often appear very small. This function scales the model
        relative to the entity, around the model's origin point. Some
        correction may also be needed via setPosition to allow for any offset.
        \param scale 3-component vector representing the scale in each axis
        */
        void setScale(const sf::Vector3f& scale);

        /*!
        \brief Returns the current rotation, in degrees, or the model around
        the given axis.
        */
        float getRotation(Model::Axis) const;

        /*!
        \brief Returns the model's translation relative to its parent entity
        */
        sf::Vector3f getTranslation() const { return{ m_translation.x, m_translation.y, m_translation.z }; }

        /*!
        \brief Returns the model's scale, relative to its own origin
        */
        sf::Vector3f getScale() const { return{ m_scale.x, m_scale.y, m_scale.z }; }

        /*!
        \brief Allows adding skeletal data to the model if the mesh supports
        skeletal animation.
        */
        void setSkeleton(const Skeleton&);

        /*!
        \brief Adds an animation to the list of animations this model has
        */
        void addAnimation(const Skeleton::Animation&);

        /*!
        \brief Sets the list of animations of this model to the given list.
        This will remove any existing animations. To append an animation to
        the model's list use addAnimation()
        */
        void setAnimations(const std::vector<Skeleton::Animation>&);

        /*!
        \brief Sets the playback rate of the current animation.
        Currently only positive values are supported, where 1.0 is normal speed.
        */
        void setPlaybackRate(float);

        /*!
        \brief Plays the animation with the given index, if it exists.
        \param index Animation index
        \param fade Crossfade time to blend between animations
        */
        void playAnimation(std::size_t index, float fade);

        /*!
        \brief Returns the index of the currently playing animation
        */
        std::size_t getCurrentAnimation() const { return m_currentAnimation; }

        /*!
        \brief Returns a const reference to the Model's mesh
        */
        const Mesh& getMesh() const { return m_mesh; }
    private:
        
        MeshRenderer* m_meshRenderer;
        
        glm::vec3 m_translation;
        glm::quat m_rotation;
        glm::vec3 m_scale;
        glm::mat4 m_preTransform;
        bool m_needsUpdate;
        void updateTransform();
        float m_depthScale;
        void update2DScale();
        

        glm::mat4 m_worldMatrix;
        const Mesh& m_mesh;
        BoundingBox m_boundingBox;
        sf::FloatRect m_globalBounds; //<entity global bounds including depth scaling
        sf::FloatRect m_worldBounds; //<globalBounds transformed by entity world position
        mutable bool m_visible;

        const Material* m_material;
        std::vector<const Material*> m_subMaterials;
        std::map<ShaderID, VertexAttribBinding> m_vaoBindings;

        const Skeleton* m_skeleton;
        std::vector<glm::mat4> m_currentFrame;
        std::vector<Skeleton::Animation> m_animations;
        std::size_t m_currentAnimation;
        std::int32_t m_nextAnimation;
        float m_blendTime;
        float m_currentBlendTime;

        float m_playbackRate;

        std::size_t draw(const glm::mat4&, const sf::FloatRect&, RenderPass::ID) const;
        void updateVertexAttribs(ShaderID newShader, const Material& newMaterial);
        void removeUnusedAttribs(ShaderID);

        void setBones(sf::Shader&, UniformID) const;
        void buildFirstFrame();
    };
}

#endif //XY_MODEL_COMPONENT_HPP_