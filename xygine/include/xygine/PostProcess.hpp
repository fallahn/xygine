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

//abstract base class for full screen post process effects

#ifndef XY_POST_PROCESS_HPP_
#define XY_POST_PROCESS_HPP_

#include <xygine/Config.hpp>
#include <xygine/MessageBus.hpp>

#include <memory>
#include <functional>

namespace sf
{
    class RenderTarget;
    class RenderTexture;
    class Shader;
}

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>

namespace xy
{
    /*!
    \brief Abstract base class for full screen post processes

    Post processes can be added to a scene so that they are applied
    after a scene is rendered. Multiple post processes can be chained
    but be aware that too many will affect performance.
    */
    class XY_EXPORT_API PostProcess
    {
    public:
        /*!
        \brief Post Process Message Handler

        Allows callback behaviour for post process effects when
        message events are raised on the message bus. Message
        handlers can be added to a post process via addMessageHandler
        */
        struct MessageHandler final
        {
            using Action = std::function<void(const Message&)>;
            Message::ID id; //< Message ID for this handler to respond to
            Action action; //< Executed when message with ID is received
        };

        using Ptr = std::unique_ptr<PostProcess>;

        PostProcess() = default;
        virtual ~PostProcess() = default;
        PostProcess(const PostProcess&) = delete;
        const PostProcess& operator = (const PostProcess&) = delete;

        /*!
        \brief Applies the effect
        \param RenderTexture A render Texture containing the scene to which the 
        effect should be applied.
        \param RenderTarget The target to which the effected scene is rendered.
        This may be the current render window, or another RenderTexture when
        chaining multiple effects together.

        The source and destinations are handled automatically by xygine when
        a post process effect is added to a scene, but must be specified
        maunally when using the effect outside the scene rendering.
        */
        virtual void apply(const sf::RenderTexture&, sf::RenderTarget&) = 0;
        /*!
        \brief Updates the effect

        Some effects may require shader parameters to be updated over time.
        This optionally overridable function passes in the current frame time
        and allows updating those parameters. This is called automatically
        when the effect is added to a Scene, but will need to be called manually
        when using the effect on its own.
        */
        virtual void update(float) {}

        /*!
        \brief Utility function for creating post process effects
        */
        template <typename T, typename... Args>
        static Ptr create(Args&&... args)
        {
            static_assert(std::is_base_of<PostProcess, T>::value, "Can only create post process types");
            return std::move(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /*!
        \brief Used internally by xygine to call any message handler
        callbacks associated with a post process effect when an effect
        is added to a Scene. This should be called manually when explicitly
        applying post process effects and message handling is required.

        \param Message Reference to message passed to callbacks
        */
        void handleMessage(const Message&);

        /*!
        \brief Adds a message handler to the list of message handling
        callbacks for this effect.

        Custom message handlers can be added to any instance of a post
        process via the MessageHandler struct.
        \see MessageHandler
        */
        void addMessageHandler(const MessageHandler&);

    protected:
        static void applyShader(const sf::Shader&, sf::RenderTarget&);

    private:
        std::vector<MessageHandler> m_messageHandlers;

    };
}

#endif //XY_POST_PROCESS_HPP_