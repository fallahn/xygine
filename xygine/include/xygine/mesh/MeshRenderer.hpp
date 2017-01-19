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

#ifndef XY_MESH_RENDERER_HPP_
#define XY_MESH_RENDERER_HPP_

#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/UniformBuffer.hpp>
#include <xygine/mesh/RenderQuad.hpp>
#include <xygine/mesh/MeshResource.hpp>
#include <xygine/mesh/MaterialResource.hpp>
#include <xygine/mesh/Skeleton.hpp>
#include <xygine/ShaderResource.hpp>
#include <xygine/MultiRenderTexture.hpp>
#include <xygine/mesh/DepthRenderTexture.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/Clock.hpp>

#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

namespace xy
{
    class MessageBus;
    class Message;
    class Mesh;
    class MeshDrawable;
    class Model;
    class Scene;
    class ModelBuilder;
    /*!
    \brief Mesh rendering class. The MeshRenderer is
    responsible for drawing the model components in a scene.
    The MeshRenderer automatically translates the given scene's camera
    into 3D space and aligns it with the 2D world so that 3D models will
    fit, creating a 2.5D like effect. The mesh renderer itself is an
    SFML drawable, so can be drawn on top of a scene like any other
    drawable class, but can also return a drawable component, allowing
    the 3D view to be drawn on any layer in a Scene.
    */
    class XY_EXPORT_API MeshRenderer final : public sf::Drawable
    {
        friend class Model;
        friend class MeshDrawable;
    public:
        /*!
        \brief Constructor.
        \param sf::Vector2u Size of the scene to render. By default xygine
        uses xy::DefaultSceneSize but if rendering the scene with multiple
        cameras it is necessary to set the size to match the camera view.
        \param Scene An instance of the scene containing the model components
        which belong to this MeshRenderer
        */
        MeshRenderer(const sf::Vector2u&, const Scene&);
        ~MeshRenderer();

        /*!
        \brief Loads a model via the provided model builder.
        Models added to the MeshRenderer are mapped to the given ID an
        resource managed, along with any other model properties such as
        materials or animations.
        \param std::int32_t Unique ID to map the model to.
        \param ModelBuilder Reference to the instance of a model builder
        used to load the model into the renderer's resource manager.
        */
        void loadModel(std::int32_t, ModelBuilder&);

        /*!
        \brief Creates a model component from previously loaded model data.
        This will return a model component complete with mesh and any materials
        and aniamtions if they were loaded via the ModelBuilder.
        \param std::int32_t Unique ID used when loading the model.
        \param MessageBus Reference to the actuve message bus.
        \returns Model component if successful, else nullptr (for example an invalid ID was provided)
        */
        std::unique_ptr<Model> createModel(std::int32_t, MessageBus&);

        /*!
        \brief Factory function for creating Model components.
        Model components require registration with the MeshRenderer
        so the only valid way to create them is via this function (or one of its overloads).
        */
        std::unique_ptr<Model> createModel(const Mesh&, MessageBus&);

        /*!
        \brief Factory function to create drawable components of the MeshRenderer.
        MeshSrawable components allow drawing the MeshRenderer directly within a 
        Scene, and therefore enables layer ordering. It is recommended for performance
        reasons that there usually be no more than one of these per MeshRenderer.
        \see MeshDrawable
        */
        std::unique_ptr<MeshDrawable> createDrawable(MessageBus&);

        /*!
        \brief Updates the MeshRenderer with the current scene status.
        Should be called once per frame after updating the scene to which
        the MeshRenderer is associated.
        */
        void update();

        /*!
        \brief Message Handler.
        System messages should be passed to this handler once per frame.
        */
        void handleMessage(const Message&);

        /*!
        \brief Returns the UniformBuffer containing the MeshRenderer's
        matrix uniforms.
        In order to render properly a shader's material requires the renderer's
        projection and view matrix. This UniformBuffer will automatically update
        the properties of any material which uses a shader with a u_matrixBlock
        uniform.
        \see Shader::Mesh::DefaultVertex
        */
        const UniformBuffer& getMatrixUniforms() const { return m_matrixBlockBuffer; }

        /*!
        \brief Returns the UniformBuffer containing the MeshRenderer's lighting
        properties.
        The MeshRenderer updates the lighting properties from its associated scene
        and scene lighting components. This properties are stored in this uniform
        buffer and updated automatically, and therefore should be used by any material
        which requires scene lighting information.
        */
        const UniformBuffer& getLightingUniforms() const { return m_lightingBlockBuffer; }

        /*!
        \brief Enables or disables the glow effect on self illuminating materials.
        Disabling this effect provides a small performance boost.
        */
        void enableGlowPass(bool);

        /*!
        \brief Sets the view to be used  when rendering the 3D scene.
        This view will override any active view when the MeshRenderer is drawn, before
        restoring the original view. This is generally the default view of the Stack context
        but can be set multiple times, for example when drawing split screen views.
        This is also useful for setting the view to that of render buffers other than the
        default window, such as sf::RenderTexture or xy::MultiRenderTexture
        */
        void setView(const sf::View& v) { m_view = v; }

        /*
        \brief Sets the multipliers used to calculate the near and far plane.
        The near and far planes ar calculated as a ratio of the virtual camera depth and
        must be between 0-1 for the near plane and greater than 1 for the far plane.
        The default is set to the optimal 0.8 and 1.2 respectively, these values should
        generally only be changed when loading models which appear in the far background.
        */
        void setNearFarRatios(float n, float f);

        /*!
        \brief Sets the field of view used by the 3D camera.
        \param float Field of view of the camera in degrees.
        The FOV is 10 degrees to 180 degrees
        */
        void setFOV(float);

        /*!
        \brief Returns the current scen FOV
        */
        float getFOV() const { return m_fov; }

        /*!
        \brief Makes a material 'transparent' based on the
        diffuse colour's alpha channel.
        \param Material to render transparently
        \param enum representing description of this material's use
        Note transparent materials currently only work with MeshDrawable
        components, and are not affected by screen space water effects.
        */
        void enableTransparency(Material&, Material::Description) const;

        /*!
        \brief Adds a material to the MeshRenderer's material cache.
        \param std::uint32_t ID Unique ID to assign to the material
        \param Material::Description used to hint to the MeshRenderer which shader paramters should be set
        \param bool castShadows Set to true if this material should cast shadows
        \param bool useAlphaBlend Set to true to enable blending this material via the diffuse
        colour's alpha value. \see enableTransparency()
        \returns Reference to the newly added material
        */
        xy::Material& addMaterial(std::uint32_t, Material::Description, bool = false, bool = false);

        /*!
        \brief returns a pointer to the material with the given ID if it exists, else returns 
        the default material.
        */
        xy::Material& getMaterial(std::uint32_t) const;
    private:
        struct Lock final {};

        MeshResource m_meshResource;
        mutable MaterialResource m_materialResource;
        mutable ShaderResource m_shaderResource;

        struct AnimationData
        {
            AnimationData(std::unique_ptr<Skeleton>& s, const std::vector<Skeleton::Animation>& a)
                : skeleton(std::move(s)), animations(a) {}
            std::unique_ptr<Skeleton> skeleton;
            std::vector<Skeleton::Animation> animations;
        };
        std::map<std::int32_t, AnimationData> m_animationResource;

        const Scene& m_scene;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        glm::mat4 m_flippedProjectionMatrix;
        float m_cameraZ;
        sf::View m_view;

        enum MaterialChannel
        {
            Diffuse = 0,
            Normal,
            Mask,
            Position
        };
        //TODO sort out this mutable mess
        struct MatrixBlock final
        {
            //use raw arrays 'cos GLSL
            float u_viewMatrix[16];
            float u_projectionMatrix[16];
            float u_lightViewProjectionMatrix[16];
        }mutable m_matrixBlock;
        mutable UniformBuffer m_matrixBlockBuffer;

        struct PointLight final
        {
            float diffuseColour[4];
            float specularColour[4];
            float inverseRange;
            float range;
            float intensity;
            bool castShadow;
            float position[3];
            float padding;
            float vpMatrix[16];            
        };
        struct SkyLight final
        {
            float diffuseColour[4];
            float specularColour[4];
            float direction[3];
            float padding;
            float vpMatrix[16];
            float intensity;           
        };
        struct LightBlock final
        {
            PointLight u_pointLights[8];
            float u_ambientColour[4];
            float u_cameraWorldPosition[3];
            float padding;
            SkyLight u_skyLight;
        }m_lightingBlock;
        UniformBuffer m_lightingBlockBuffer;

        std::array<sf::Glsl::Vec3, 64> m_ssaoKernel;
        sf::Shader m_ssaoShader;
        mutable sf::RenderTexture m_ssaoTexture;
        sf::Sprite m_ssaoSprite;
        sf::Texture m_ssaoNoiseTexture;
        void createNoiseTexture();

        mutable sf::Shader m_lightBlurShader;
        sf::Shader m_lightDownsampleShader;
        mutable sf::RenderTexture m_lightBlurTexture;
        mutable sf::RenderTexture m_lightDownsampleTexture;
        mutable sf::Sprite m_lightBlurSprite;
        sf::Sprite m_downSampleSprite;
        bool m_doLightBlur;
        sf::Texture m_lightFallback;

        sf::Shader m_lightingShader;
        UniformBlockID m_lightingBlockID;

        glm::mat4 m_lightProjectionMatrix;
        mutable DepthRenderTexture m_depthTexture;
        sf::Shader m_depthShader;
        void drawDepth(const sf::FloatRect&) const;

        sf::Shader m_waterShader;
        sf::Texture m_surfaceTexture;
        sf::Clock m_shaderClock;

        mutable std::vector<Model*> m_models;
        mutable xy::MultiRenderTexture m_gBuffer;
        void drawScene(const sf::FloatRect&) const;
        void drawAlphaBlended(const sf::FloatRect&) const;

        sf::Shader m_debugShader;
        sf::Texture m_dummyTexture;
        sf::Sprite m_dummySprite;
        std::unique_ptr<RenderQuad> m_outputQuad;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        float m_nearRatio;
        float m_farRatio;
        float m_fov;

        void updateView();
        void updateLights(const glm::vec3&);     

        void resizeGBuffer(sf::Uint32, sf::Uint32);

        void initSSAO();
        void initSelfIllum();
        void initOutput();

        void setupConCommands();
        void addDebugMenus();
    };
}

#endif //XY_MESH_RENDERER_HPP_