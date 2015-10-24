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

#ifndef SCENE_HPP_
#define SCENE_HPP_

#include <xygine/Entity.hpp>
#include <xygine/PostBloom.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/Command.hpp>
#include <xygine/QuadTree.hpp>

#include <SFML/Graphics/Drawable.hpp>

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

        enum PostEffect
        {
            None = 0,
            Bloom = 0x1,
            ChromaticAbberation = 0x2
        };

        Scene(MessageBus&, bool createBuffers = true);
        ~Scene() = default;
        Scene(const Scene&) = delete;
        const Scene& operator = (const Scene&) = delete;

        void update(float);
        void handleMessage(const Message&);
        void addEntity(Entity::Ptr&, Layer);
        Entity* findEntity(sf::Uint64);
        Entity& getLayer(Layer);

        void setView(const sf::View& v);
        const sf::View& getView() const;

        void sendCommand(const Command&);

        sf::FloatRect getVisibleArea() const;
        void setSceneWidth(float);
        float getSceneWidth() const;

        std::vector<QuadTreeComponent*> queryQuadTree(const sf::FloatRect&);

        void reset();

        //enable available post effects via given bit mask
        void setPostEffects(sf::Uint32 flags);

        //enables output debug information when _DEBUG_ is defined
        void drawDebug(bool);

    private:
        QuadTree m_quadTree; //must live longer than any entity
        std::vector<Entity::Ptr> m_layers;
        std::vector<std::pair<Layer, Entity::Ptr>> m_pendingEntities;
        float m_sceneWidth;

        MessageBus& m_messageBus;
        CommandQueue m_commandQueue;

        bool m_drawDebug;

        mutable sf::RenderTexture m_sceneBufferA;
        mutable sf::RenderTexture m_sceneBufferB;
        mutable std::unique_ptr<PostBloom> m_bloomEffect;
        mutable std::unique_ptr<PostChromeAb> m_chromeAbEffect;

        std::function<void(sf::RenderTarget&, sf::RenderStates)> m_currentRenderPath;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        void defaultRenderPath(sf::RenderTarget&, sf::RenderStates) const;
        void bloomRenderPath(sf::RenderTarget&, sf::RenderStates) const;
        void chromeAbRenderPath(sf::RenderTarget&, sf::RenderStates) const;
        void fullRenderPath(sf::RenderTarget&, sf::RenderStates) const;
    };
}
#endif //SCENE_HPP_