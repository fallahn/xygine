/*********************************************************************
Matt Marchant 2014 - 2015
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
#include <xygine/ComponentCamera.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <vector>
#include <functional>

namespace xy
{
    class Scene final : public sf::Drawable
    {
    public:
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

        Scene(MessageBus&, bool createBuffers = true);
        ~Scene() = default;
        Scene(const Scene&) = delete;
        const Scene& operator = (const Scene&) = delete;

        void update(float);
        void handleMessage(const Message&);
        //moves an entity into the scene on to the given
        //layer. returns a pointer to the entity
        Entity* addEntity(Entity::Ptr&, Layer);
        //returns a pointer to the entity with given UID
        //or nullptr if that entity does exist
        Entity* findEntity(sf::Uint64);
        //returns a reference to the root entity
        //of a given layer
        Entity& getLayer(Layer);

        //sets the current scene view and activates
        //the default scene camera
        void setView(const sf::View& v);
        //returns the view of the current active camera
        sf::View getView() const;
        //returns a floatrect representing the visible
        //area of the active camera. useful for quad tree queries
        sf::FloatRect getVisibleArea() const;
        //sets the scene's currently active camera
        void setActiveCamera(const Camera*);
        //set the clear colour of the scene's default camera
        //this only affects buffers used in post process effects
        void setClearColour(const sf::Color&);
        
        //send a command targetting one or more entities
        void sendCommand(const Command&);

        //returns a vector of quad tree components found in the
        //queried area.
        std::vector<QuadTreeComponent*> queryQuadTree(const sf::FloatRect&);

        //resets the scene removing all entities and post effects
        void reset();

        //add a post effect to the end of the render chain
        void addPostProcess(PostProcess::Ptr&);

        //enables output debug information when _DEBUG_ is defined
        void drawDebug(bool);

    private:
        QuadTree m_quadTree; //must live longer than any entity
        std::vector<Entity::Ptr> m_layers;
        std::vector<std::pair<Layer, Entity::Ptr>> m_pendingEntities;
        
        Camera::Ptr m_defaultCamera;
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