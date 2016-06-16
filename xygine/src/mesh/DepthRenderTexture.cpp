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

namespace
{
    GLint maxLayers = 0;
}

DepthRenderTexture::DepthRenderTexture()
    : m_fbo     (0),
    m_layerCount(0)
{
    if (maxLayers == 0)
    {
        glCheck(glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayers));
    }
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
bool DepthRenderTexture::create(sf::Uint32 width, sf::Uint32 height, std::uint8_t layerCount)
{
    XY_ASSERT(layerCount > 0, "Requires at least one layer");
    XY_ASSERT(layerCount < maxLayers, "Maximum layer count on this hardware is " + std::to_string(maxLayers));

    if (!glGenFramebuffers)
    {
        Logger::log("OpenGL extensions required for depth texture are unavailable", Logger::Type::Error);
        return false;
    }

    glCheck(glGenTextures(1, &m_textureID));
    glCheck(glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID));
    glCheck(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, width, height, layerCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
    glCheck(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glCheck(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glCheck(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    //GLfloat borderColour[] = { 1.0, 1.0, 1.0, 1.0 };
    //glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);

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

//    glCheck(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture.getNativeHandle(), 0));
    glCheck(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureID, 0, 0));
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
    m_size.x = width;
    m_size.y = height;
    m_layerCount = layerCount;

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
    return m_size;
}

bool DepthRenderTexture::setLayerActive(std::uint8_t layer)
{
    XY_ASSERT(layer < m_layerCount, "Index out of range");
    glCheck(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
    glCheck(glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_textureID, 0, layer));

    GLenum result;
    glCheck(result = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if (result != GL_FRAMEBUFFER_COMPLETE)
    {       
        Logger::log("Failed to switch depth texture layer to " + std::to_string(layer), xy::Logger::Type::Error, xy::Logger::Output::All);
        glCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        return false;
    }
    return true;
}

//private
bool DepthRenderTexture::activate(bool active)
{
    return setActive(active);
}