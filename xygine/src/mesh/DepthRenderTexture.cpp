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

#include <xygine/mesh/DepthRenderTexture.hpp>
#include <xygine/Assert.hpp>
#include <xygine/detail/GLCheck.hpp>

using namespace xy;

DepthRenderTexture::DepthRenderTexture()
    : m_fbo(0)
{

}

DepthRenderTexture::~DepthRenderTexture()
{
    ensureGlContext();

    if (m_fbo)
    {
        GLuint fbo = static_cast<GLuint>(m_fbo);
        glCheck(glDeleteFramebuffers(1, &fbo));
    }
}

//public
bool DepthRenderTexture::create(sf::Uint32 width, sf::Uint32 height)
{
    if (!glGenFramebuffers)
    {
        Logger::log("OpenGL extensions required for depth texture are unavailable", Logger::Type::Error);
        return false;
    }

    if (!m_texture.create(width, height))
    {
        Logger::log("failed creating depth texture", Logger::Type::Error);
        return false;
    }
    else
    {
        //override the default type
        glCheck(glBindTexture(GL_TEXTURE_2D, m_texture.getNativeHandle()));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
        glCheck(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        glCheck(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        glCheck(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        glCheck(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    }

    m_context = std::make_unique<sf::Context>();
    //create FBO
    GLuint fbo = 0;
    glCheck(glGenFramebuffers(1, &fbo));
    m_fbo = static_cast<unsigned int>(fbo);
    if (m_fbo == 0)
    {
        Logger::log("Failed creating FBO for depth texture", xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

    glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture.getNativeHandle(), 0));
    glCheck(glDrawBuffer(GL_NONE));
    glCheck(glReadBuffer(GL_NONE));

    //check everything went OK
    GLenum result;
    glCheck(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (result != GL_FRAMEBUFFER_COMPLETE)
    {
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        Logger::log("Failed to attach texture to FBO", xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }

    sf::RenderTarget::initialize();
    return true;
}

bool DepthRenderTexture::setActive(bool active)
{
    return m_context->setActive(active);
}

void DepthRenderTexture::display()
{
    if (setActive(true))
    {
        glCheck(glFlush());
    }
}

sf::Vector2u DepthRenderTexture::getSize() const
{
    return m_texture.getSize();
}

const sf::Texture& DepthRenderTexture::getTexture() const 
{
    return m_texture;
}

//private
bool DepthRenderTexture::activate(bool active)
{
    return setActive(active);
}