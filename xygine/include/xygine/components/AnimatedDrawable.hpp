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

//drawable for animated  sprite sheets

#ifndef XY_ANISPRITE_HPP_
#define XY_ANISPRITE_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace xy
{
    /*!
    \brief Represents a single animation which can be played by an AnimatedSprite
    */
    struct XY_EXPORT_API Animation
    {
        friend class AnimatedDrawable;
        Animation(const std::string& name, sf::Int16 begin, sf::Int16 end, bool loop = true, float frameRate = 0.f)
            : m_name(name), m_startFrame(begin), m_endFrame(end), m_loop(loop), m_frameRate(frameRate){}

        /*!
        \brief Gets the name of the animation

        \returns std::string containing the name
        */
        const std::string& getName() const
        {
            return m_name;
        }

    private:
        std::string m_name;
        sf::Int16 m_startFrame;
        sf::Int16 m_endFrame;
        bool m_loop;
        float m_frameRate;
    };

    /*!
    \brief functor for searching vectors of animations
    */
    struct FindAnimation
    {
        FindAnimation(const std::string& name) : m_name(name){}
        bool operator()(const Animation& a)
        {
            return (a.getName() == m_name);
        };
    private:
        std::string m_name;
    };

    class TextureResource;

    /*!
    \brief Provides an interface for animated sprites, allowing them to be attached to an entity

    Animated Drawables are usually composed of a sprite sheet loaded int oa texture, and a set
    of one or more animation ranges. Animations can be created by manually setting Animated Drawable
    properties such as frame size and frame rate, or can be loaded from a specially formatted
    animation data file saved as a json string. These files can be created with programs such as the
    Sprite Animation editor included in the xygine repository.
    */
    class XY_EXPORT_API AnimatedDrawable final : public sf::Drawable, public sf::Transformable, public Component
    {
    public:
        using Ptr = std::unique_ptr<AnimatedDrawable>;

        explicit AnimatedDrawable(MessageBus&);
        AnimatedDrawable(MessageBus&, const sf::Texture& t);
        AnimatedDrawable(const AnimatedDrawable&) = delete;
        ~AnimatedDrawable() = default;

        const AnimatedDrawable& operator = (const AnimatedDrawable&) = delete;

        Component::Type type() const override;
        void entityUpdate(Entity&, float dt) override;

        sf::FloatRect localBounds() const override;
        sf::FloatRect globalBounds() const override;

        /*!
        \brief Sets the texture used by the Animated Drawable
        */
        void setTexture(const sf::Texture& t);
        /*!
        \brief Gets the texture used by the Animated Drawable

        \returns a const pointer to the current texture or nullptr if no texture set
        */
        const sf::Texture* getTexture() const;
        /*!
        \brief Sets an optional shader to be used when the component is drawn
        */
        void setShader(sf::Shader& shader);
        /*!
        \brief Sets an optional normal map texture to be used if the component
        uses a normal mapping shader
        */
        void setNormalMap(const sf::Texture&);
        /*!
        \brief Sets an optional mask map texture to be used if the component
        uses a normal mapping shader
        */
        void setMaskMap(const sf::Texture&);
        /*!
        \brief Sets the size of a single frame in the animation
        */
        void setFrameSize(const sf::Vector2i& size);
        /*!
        \brief Gets the current frame size for the active animation
        */
        const sf::Vector2i& getFrameSize() const;
        /*!
        \brief Sets the total number of frames in the current animation
        */
        void setFrameCount(sf::Uint8 count);
        /*!
        \brief Gets the number of frames in the currently active animation
        */
        sf::Uint8 getFrameCount() const;
        /*!
        \brief Sets the number of frames to be displayed per second for the active animation
        */
        void setFrameRate(float rate);
        /*!
        \brief Gets the frame rate in frames per second for the active animation
        */
        float getFrameRate() const;
        /*!
        \brief Set whether or not the current animation should be played looped
        */
        void setLooped(bool looped);
        /*!
        \brief returns true if the current animation is looped, else returns false
        */
        bool looped() const;
        /*!
        \brief Plays the current active animation

        \param First frame to start playing from
        \param Frame to stop the animation on. use -1 to play to the end
        \param Offset into the animation, in frames, to start playing from
        */
        void play(sf::Int16 start = 0, sf::Int16 end = -1, sf::Int16 offset = 0);
        /*!
        \brief Plays a given animation

        \param An instance of an Animation struct to set the playback parameters
        \param Offset in frames into the animation to start playing from
        */
        void play(Animation a, sf::Int16 offset = 0);
        /*!
        \brief Play an animation from the internal list of animations

        \param Index into the list of animations from which to play
        \param Offset from the first frame of the animation to start from
        */
        void playAnimation(sf::Uint32 index, sf::Int16 offset = 0);
        /*!
        \brief Returns true if the current animation is playing, else returns false
        */
        bool playing() const;
        /*!
        \brief Pauses or unpauses the currently playing animation

        \param true to pause the animation, else false to start playing from the current frame
        */
        void setPaused(bool paused);

        /*!
        \brief Sets the colour with which the drawable is multiplied. White by default
        */
        void setColour(const sf::Color& c);
        /*!
        \brief Sets the blend mode used when drawing. Defaults to BlendAlpha
        */
        void setBlendMode(sf::BlendMode);

        /*!
        \brief Returns a vector in world coordinates representing the forward
        facing direction of the Animated Drawable
        */
        sf::Vector2f getForwardVector() const;
        /*!
        \brief Returns a vector in world coordinates representing the right
        facing direction of the Animated Drawable
        */
        sf::Vector2f getRightVector() const;

        /*!
        \brief Loads a formatted json file containing animation information
        as produced by the xygine sprite editor

        \param Relative path to the json file
        */
        void loadAnimationData(const std::string& path);
        /*!
        \brief Returns a vector of Animation structs used by the drawable component
        normally loaded from an animation data file
        */
        const std::vector<Animation>& getAnimations()const;

    private:

        sf::Sprite m_sprite;
        sf::Shader* m_shader;
        const sf::Texture* m_normalMap;
        const sf::Texture*m_maskMap;
        sf::IntRect m_subRect;
        sf::Vector2u m_textureSize;
        sf::Vector2i m_frameSize;

        sf::Uint8 m_frameCount;
        sf::Uint8 m_currentFrame;
        sf::Uint8 m_firstFrame;
        sf::Uint8 m_lastFrame;

        float m_frameRate;
        float m_elapsedTime;
        bool m_loop;
        bool m_playing;

        std::vector<Animation> m_animations;
        sf::BlendMode m_blendmode;

        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
        void setFrame(sf::Uint8 frame);
    };
}
#endif //XY_ANISPRITE_HPP_