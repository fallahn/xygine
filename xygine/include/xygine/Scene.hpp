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

//root class containing scene entities

#ifndef XY_SCENE_HPP_
#define XY_SCENE_HPP_

#include <xygine/Entity.hpp>
#include <xygine/PostProcess.hpp>
#include <xygine/Command.hpp>
#include <xygine/QuadTree.hpp>
#include <xygine/components/Camera.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <vector>
#include <functional>

namespace xy
{
    class PointLight;

    /*!
    \brief Scene class

    The Scene class is reponsible for updating and drawing the
    scene graph constructed of entities.
    */
    class XY_EXPORT_API Scene final : public sf::Drawable
    {
    public:
        /*!
        \brief Layer names to which entities may be added
        */
        enum Layer
        {
            BackRear = 0,
            BackMiddle,
            BackFront,
            FrontRear,
            FrontMiddle,
            FrontFront,
            UI,
            Count
        };

        explicit Scene(MessageBus&);
        ~Scene();
        Scene(const Scene&) = delete;
        const Scene& operator = (const Scene&) = delete;

        /*!
        \brief Updates the entire scene with the current frame time

        This should be called once per frame to make sure all entities
        and components belonging to the scene are updated
        */
        void update(float);
        /*!
        \brief Scene message handler

        This should be called once a frame with any messages
        received from the message bus. This forwards all messages to
        entities and components within the scene
        */
        void handleMessage(const Message&);
        /*!
        \brief Moves an entity into the scene on to the given
        layer.
        \returns Pointer to the entity once it has been added

        Once an entity is added to the scene the scene takes
        ownership, invalidating the original entity pointer.
        The returned pointer can be used to continue to modify
        the entity but care should be taken as deleted entities
        will lead to dangling pointers which are kept alive for
        too long.
        */
        Entity* addEntity(Entity::Ptr&, Layer);
        /*!
        \brief Find an entity with the given unique ID
        \returns Pointer to the entity if it exists within 
        the scene, else returns nullptr
        */
        Entity* findEntity(sf::Uint64);
        /*!
        \brief Returns a reference to the root entity of a given layer
        */
        Entity& getLayer(Layer);

        /*!
        \brief Sets the current scene view and activates the default scene camera
        */
        void setView(const sf::View& v);
        /*!
        \brief Returns the view of the current active camera
        */
        sf::View getView() const;
        /*!
        \brief Returns a FloatRect representing the visible area of the active camera.
        Useful for quad tree queries
        */
        sf::FloatRect getVisibleArea() const;
        /*!
        \brief Sets the scene's currently active camera.
        Passing nullptr tells the scene to use the default camera
        */
        void setActiveCamera(const Camera*);
        /*!
        \brief Set the clear colour of the scene's default camera
        
        This only affects buffers used in post process effects, else
        it has no effect
        */
        void setClearColour(const sf::Color&);
        
        /*!
        \brief Set the area which the scene will cover.
        This is only necessary when using the QuadTree. By default
        the queryable quad tree area is the same size as the default
        view (1920/1080). Passing a FloatRect to this function will
        resize the total queryable area of the quad tree.
        */
        void setSize(const sf::FloatRect&);

        /*!
        \brief Sends a command to the scene graph

        Commands can be used to update a specific entity or group of 
        entities.
        \see Command
        */
        void sendCommand(const Command&);

        /*!
        \brief Queries the scene's quad tree
        \param FloatRect and area in world coordinates to query the quad tree
        \returns vector of pointers to QuadTree components found in the queried area

        The components in the returned vector can be used to determine entities
        which fall in a given area that can be used in collision testing or
        render culling for example. For an entity to be considered it must
        have a QuadTreeComponent attached.
        */
        std::vector<QuadTreeComponent*> queryQuadTree(const sf::FloatRect&) const;

        /*!
        \brief Returns a list of point lights found in the queried area
        \param FloatRect Area in world coordinates in which to retrieve visible lights
        \returns vector of pointers to lights contained in the queried area
        */
        std::vector<PointLight*> getVisibleLights(const sf::FloatRect&) const;

        /*!
        \brief Resets the scene removing all entities and post effects
        */
        void reset();

        /*!
        \brief Adds a post effect to the end of the render chain
        \see PostProcess
        */
        void addPostProcess(PostProcess::Ptr&);

        /*!
        \brief enables output debug information when _DEBUG_ is defined

        Scene debug info includes drawing the AABB of scene entities as
        well as partition boundries of the QuadTree
        */
        void drawDebug(bool);

        /*!
        \brief Returns the view matrix of the active camera if a scene exists
        else returns an identity matrix
        */
        static sf::Transform getViewMatrix();

    private:

        QuadTree m_quadTree; //must live longer than any entity
        QuadTree m_lightTree;
        std::vector<Entity::Ptr> m_layers;
        std::vector<std::pair<Layer, Entity::Ptr>> m_pendingEntities;
        
        Camera* m_defaultCamera;
        const Camera* m_activeCamera;
        
        MessageBus& m_messageBus;
        CommandQueue m_commandQueue;

        bool m_drawDebug;

        struct RenderPass
        {
            PostProcess::Ptr postEffect;
            sf::RenderTexture* inBuffer = nullptr;
            sf::RenderTexture* outBuffer = nullptr;
        };
        std::vector<RenderPass> m_renderPasses;

        mutable sf::RenderTexture m_sceneBufferA;
        mutable sf::RenderTexture m_sceneBufferB;

        std::function<void(sf::RenderTarget&, sf::RenderStates)> m_currentRenderPath;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        void defaultRenderPath(sf::RenderTarget&, sf::RenderStates) const;
        void postEffectRenderPath(sf::RenderTarget&, sf::RenderStates) const;
    };
}
#endif //XY_SCENE_HPP_