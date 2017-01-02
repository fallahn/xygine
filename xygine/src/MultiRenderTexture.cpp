/*********************************************************************
Matt Marchant 2014 - 2017
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

#include <xygine/detail/GLExtensions.hpp>

#include <xygine/MultiRenderTexture.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Log.hpp>
#include <xygine/detail/GLCheck.hpp>

#include <vector>

using namespace xy;

namespace
{
    const std::size_t MAX_TEXTURES = 4u;
}

MultiRenderTexture::MultiRenderTexture()
    : m_textureCount    (0u),
    m_fbo               (0u),
    m_depthbuffer       (0u)
{

}

MultiRenderTexture::~MultiRenderTexture()
{
    ensureGlContext();

    if (m_depthbuffer)
    {
        GLuint depthbuffer = static_cast<GLuint>(m_depthbuffer);
        glCheck(glDeleteRenderbuffers(1, &depthbuffer));
    }

    if (m_fbo)
    {
        GLuint fbo = static_cast<GLuint>(m_fbo);
        glCheck(glDeleteFramebuffers(1, &fbo));
    }
}

//public
bool MultiRenderTexture::create(sf::Uint32 width, sf::Uint32 height, sf::Uint32 count, bool depthBuffer, bool depthTexture)
{    
    if (!glGenFramebuffers)
    {
        Logger::log("OpenGL extensions required for MRT are unavailble", Logger::Type::Error, Logger::Output::All);
        return false;
    }
    
    XY_ASSERT(count > 0, "You need to create at least one texture");
    XY_ASSERT(count <= MAX_TEXTURES, "Currently up to 4 textures are supported");
    XY_WARNING(count == 1, "Only having one texture may be sub-optimal. Consider using sf::RenderTexture instead.");

    m_textureCount = count;

    for (auto i = 0u; i < count; ++i)
    {
        if (!m_textures[i].create(width, height))
        {
            Logger::log("Failed creating MRT texture", Logger::Type::Error, Logger::Output::All);
            return false;
        }
        m_textures[i].setSmooth(false);
    }

    {
        m_context = std::make_unique<sf::Context>();
        GLuint fbo = 0;
        glCheck(glGenFramebuffers(1, &fbo));
        m_fbo = static_cast<unsigned int>(fbo);
        if (m_fbo == 0)
        {
            Logger::log("Failed creating FBO for MRT", xy::Logger::Type::Error, xy::Logger::Output::All);
            return false;
        }
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

        if (depthBuffer) //TODO buffers are a bit faster so we could make the texture an option
        {
            if (!depthTexture)
            {
                GLuint depth = 0;
                glCheck(glGenRenderbuffers(1, &depth));
                m_depthbuffer = static_cast<unsigned int>(depth);
                if (m_depthbuffer == 0)
                {
                    Logger::log("Failed creating depth buffer for MRT", Logger::Type::Error, Logger::Output::All);
                    return false;
                }
                glCheck(glBindRenderbuffer(GL_RENDERBUFFER, m_depthbuffer));
                glCheck(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
                glCheck(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthbuffer));
            }
            else
            {
                m_depthTexture.create(width, height);
                glCheck(glBindTexture(GL_TEXTURE_2D, m_depthTexture.getNativeHandle()));
                glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
                glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture.getNativeHandle(), 0);
            }
        }
        //attach textures
        for (auto i = 0u; i < count; ++i)
        {
            glCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i].getNativeHandle(), 0));
        }

        //tell draw buffers which textures to use
        std::vector<GLenum> buffers;
        for (auto i = 0u; i < count; ++i)
        {
            buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
        glCheck(glDrawBuffers(count, buffers.data()));

        //check everything went OK
        GLenum result;
        glCheck(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (result != GL_FRAMEBUFFER_COMPLETE)
        {
            glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
            Logger::log("Failed to attach texture to FBO", xy::Logger::Type::Error, xy::Logger::Output::All);
            return false;
        }
    }

    sf::RenderTarget::initialize();
    return true;
}

void MultiRenderTexture::setSmooth(bool smooth)
{
    for (auto i = 0u; i < m_textureCount; ++i)
    {
        m_textures[i].setSmooth(smooth);
    }
}

bool MultiRenderTexture::isSmooth() const
{
    return m_textures[0].isSmooth();
}

void MultiRenderTexture::setRepeated(bool repeat)
{
    for (auto i = 0u; i < m_textureCount; ++i)
    {
        m_textures[i].setRepeated(repeat);
    }
}

bool MultiRenderTexture::isRepeated() const
{
    return m_textures[0].isRepeated();
}

bool MultiRenderTexture::setActive(bool active)
{
    return m_context->setActive(active);
}

void MultiRenderTexture::display()
{
    if (setActive(true))
    {
        glCheck(glFlush());
        //TODO we need to mark textures as pixelsFlipped - but we don't have access here
    }
}

sf::Vector2u MultiRenderTexture::getSize() const
{
    return m_textures[0].getSize();
}

const sf::Texture& MultiRenderTexture::getTexture(std::size_t index) const
{
    XY_ASSERT(index < m_textureCount, "MultiRenderTexture index out of range.");
    return m_textures[index];
}

//private
bool MultiRenderTexture::activate(bool active)
{
    return setActive(active);
}