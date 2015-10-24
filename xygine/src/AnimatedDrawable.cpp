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

#include <xygine/AnimatedDrawable.hpp>
#include <xygine/Resource.hpp>
#include <xygine/Util.hpp>
#include <xygine/JsonUtil.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <xygine/picojson.h>

#include <cassert>
#include <fstream>

using namespace xy;

AnimatedDrawable::AnimatedDrawable(MessageBus& mb)
    : Component     (mb),
    m_shader        (nullptr),
    m_normalMap     (nullptr),
    m_frameCount    (0u),
    m_currentFrame  (0u),
    m_firstFrame    (0u),
    m_lastFrame     (0u),
    m_frameRate     (0.f),
    m_elapsedTime   (0.f),
    m_loop          (false),
    m_playing       (false){}

AnimatedDrawable::AnimatedDrawable(MessageBus& mb, const sf::Texture& t)
    : Component     (mb),
    m_sprite        (t),
    m_shader        (nullptr),
    m_normalMap     (nullptr),
    m_textureSize   (t.getSize()),
    m_frameSize     (m_textureSize),
    m_frameCount    (0u),
    m_currentFrame  (0u),
    m_firstFrame    (0u),
    m_lastFrame     (0u),
    m_frameRate     (0.f),
    m_elapsedTime   (0.f),
    m_loop          (false),
    m_playing       (false){}


//public
Component::Type AnimatedDrawable::type() const
{
    return Component::Type::Drawable;
}

Component::UniqueType AnimatedDrawable::uniqueType() const
{
    return Component::UniqueId::AnimatedDrawableId;
}

void AnimatedDrawable::entityUpdate(Entity&, float dt)
{
    if (m_playing)
    {
        float frameTime = 1.f / m_frameRate;
        m_elapsedTime += dt;

        while (m_elapsedTime >= frameTime && (m_currentFrame <= m_lastFrame || m_loop))
        {
            //while not at the end of the range or loop set to true
            //move frame to the right
            m_subRect.left += m_frameSize.x;

            //wrap around if at texture edge
            if (m_subRect.left + m_subRect.width > static_cast<int>(m_textureSize.x))
            {
                m_subRect.left = 0;
                m_subRect.top += m_frameSize.y;
            }

            //update frame data
            m_elapsedTime -= frameTime;
            
            m_currentFrame++;
        }

        if (m_currentFrame > m_lastFrame)
        {
            if (!m_loop)
            {
                m_playing = false;
            }
            setFrame(m_firstFrame);
        }

        m_sprite.setTextureRect(m_subRect);
    }
}

void AnimatedDrawable::handleMessage(const Message&)
{

}

sf::FloatRect AnimatedDrawable::localBounds() const
{
    //should we be using abs on frame size?
    return sf::FloatRect(sf::Vector2f(), static_cast<sf::Vector2f>(getFrameSize()));
}

sf::FloatRect AnimatedDrawable::globalBounds() const
{
    return getTransform().transformRect(localBounds());
}

void AnimatedDrawable::setTexture(const sf::Texture& t)
{
    m_sprite.setTexture(t);
    m_textureSize = t.getSize();
    setFrameSize(static_cast<sf::Vector2i>(m_textureSize));
}

const sf::Texture* AnimatedDrawable::getTexture() const
{
    return m_sprite.getTexture();
}

void AnimatedDrawable::setShader(sf::Shader& shader)
{
    m_shader = &shader;
}

void AnimatedDrawable::setNormalMap(const sf::Texture& t)
{
    m_normalMap = &t;
}

void AnimatedDrawable::setFrameSize(const sf::Vector2i& size)
{
    m_frameSize = size;
    m_subRect.width = size.x;
    m_subRect.height = size.y;
}

const sf::Vector2i& AnimatedDrawable::getFrameSize() const
{
    return m_frameSize;
}

void AnimatedDrawable::setFrameCount(sf::Uint8 count)
{
    m_frameCount = count;
}

sf::Uint8 AnimatedDrawable::getFrameCount() const
{
    return m_frameCount;
}

void AnimatedDrawable::setFrameRate(float rate)
{
    assert(rate >= 0.f);
    m_frameRate = rate;
    //m_playing = (rate > 0.f);
}

float AnimatedDrawable::getFrameRate() const
{
    return m_frameRate;
}

void AnimatedDrawable::setLooped(bool loop)
{
    m_loop = loop;
}

bool AnimatedDrawable::looped() const
{
    return m_loop;
}

void AnimatedDrawable::play(sf::Int16 start, sf::Int16 end, sf::Int16 offset)
{
    assert(start < m_frameCount && (end < m_frameCount));

    m_playing = !(start == end);// ? false : true;
    m_firstFrame = static_cast<sf::Uint8>(start);
    m_currentFrame = m_firstFrame + offset;
    m_lastFrame = (end < 0) ? m_frameCount - 1 : end;

    setPaused(false);

    //set position of starting frame / sub rect
    setFrame(m_currentFrame);
}

void AnimatedDrawable::play(Animation animation, sf::Int16 offset)
{
    setLooped(animation.m_loop);
    play(animation.m_startFrame, animation.m_endFrame, offset);
}

void AnimatedDrawable::playAnimation(sf::Uint32 index, sf::Int16 offset)
{
    if (index >= m_animations.size())
    {
        LOG("Animated Drawable failed to play animation at index: " + std::to_string(index), Logger::Type::Error);
        return;
    }
    play(m_animations[index], offset);
}

bool AnimatedDrawable::playing() const
{
    return m_playing;
}

void AnimatedDrawable::setPaused(bool paused)
{
    m_playing = !paused;
}

void AnimatedDrawable::setColour(const sf::Color& c)
{
    m_sprite.setColor(c);
}

void AnimatedDrawable::setBlendMode(sf::BlendMode mode)
{
    m_blendmode = mode;
}

sf::Vector2f AnimatedDrawable::getForwardVector() const
{
    return getTransform().transformPoint({ 1.f, 0.f });
}

sf::Vector2f AnimatedDrawable::getRightVector() const
{
    return getTransform().transformPoint({ 0.f, 1.f });
}

void AnimatedDrawable::loadAnimationData(const std::string& path)
{
    std::ifstream file(path);
    if (!file.good() || !Util::File::validLength(file))
    {
        LOG("failed to open " + path + ", or file empty", Logger::Type::Error);
        file.close();
        return;
    }
    
    std::string jsonString;
    while (!file.eof())
    {
        std::string temp;
        file >> temp;
        jsonString += temp;
    }
    if (jsonString.empty())
    {
        LOG(path + "failed to read, or file empty", Logger::Type::Error);
        file.close();
        return;
    }
    file.close();
    
    picojson::value pv;
    auto err = picojson::parse(pv, jsonString);
    if (err.empty())
    {
        //get array of animations
        if (pv.get("Animations").is<picojson::array>())
        {
            const auto& anims = pv.get("Animations").get<picojson::array>();
            for (const auto& a : anims)
            {
                std::string name = (a.get("Name").is<std::string>()) ? a.get("Name").get<std::string>() : "";
                sf::Int16 start = (a.get("Start").is<double>()) ? static_cast<sf::Int16>(a.get("Start").get<double>()) : 0;
                sf::Int16 end = (a.get("End").is<double>()) ? static_cast<sf::Int16>(a.get("End").get<double>()) : 0;
                bool loop = (a.get("Loop").is<bool>()) ? a.get("Loop").get<bool>() : false;
                m_animations.emplace_back(name, start, end, loop);
            }
        }
    
        //properties
        if (pv.get("FrameCount").is<double>())
            m_frameCount = static_cast<sf::Uint8>(pv.get("FrameCount").get<double>());
        else
            Logger::log(path + " missing frame count", Logger::Type::Error, Logger::Output::All);
    
        if (pv.get("FrameSize").is<std::string>())
            setFrameSize(Util::Vector::vec2FromString<int>(pv.get("FrameSize").get<std::string>()));
        else
            Logger::log(path + " missing frame size", Logger::Type::Error, Logger::Output::All);
    
        if (pv.get("FrameRate").is<double>())
            m_frameRate = static_cast<float>(pv.get("FrameRate").get<double>());
        else
            Logger::log(path + " missing frame rate", Logger::Type::Error, Logger::Output::All);
    }
    else
    {
        Logger::log("Animated Sprite: " + err, Logger::Type::Error, Logger::Output::All);
    }

    setFrame(m_currentFrame);
}

const std::vector<Animation>& AnimatedDrawable::getAnimations()const
{
    return m_animations;
}

//private
void AnimatedDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{    
    states.transform *= getTransform();
    states.shader = m_shader;
    states.blendMode = m_blendmode;

    if (m_shader)
    {
        m_shader->setParameter("u_diffuseMap", *m_sprite.getTexture());
        m_shader->setParameter("u_normalMap", *m_normalMap);
        m_shader->setParameter("u_inverseWorldViewMatrix", states.transform.getInverse());
    }
    rt.draw(m_sprite, states);
}

void AnimatedDrawable::setFrame(sf::Uint8 index)
{
    assert(index < m_frameCount);

    auto position = m_textureSize.x / m_frameSize.x;

    auto x = index % position;
    auto y = index / position;

    m_subRect.left = x * m_frameSize.x;
    m_subRect.top = y * m_frameSize.y;

    m_sprite.setTextureRect(m_subRect);
    m_currentFrame = index;
}