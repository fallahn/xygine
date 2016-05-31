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

#include <xygine/mesh/Skeleton.hpp>
#include <xygine/Assert.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace xy;

Skeleton::Animation::Animation(std::size_t frameCount, std::size_t startFrame, float frameRate, const std::string& name, bool looped)
    : m_frameCount      (frameCount),
    m_startFrame        (startFrame),
    m_endFrame          (m_startFrame + frameCount),
    m_frameRate         (frameRate),
    m_name              (name),
    m_looped            (looped),
    m_skeleton          (nullptr),
    m_currentTime       (0.f),
    m_frameTime         (1.f / frameRate),
    m_currentFrameIndex (0u),
    m_nextFrameIndex    (0u)
{

}

//public
const std::vector<glm::mat4>& Skeleton::Animation::getCurrentFrame() const
{
    return m_currentFrame;
}

void Skeleton::Animation::setSkeleton(Skeleton* s)
{
    m_skeleton = s;
    if (m_skeleton)
    {
        buildFirstFrame();
    }
}

void Skeleton::Animation::update(float dt)
{
    m_currentTime += dt;
    if (m_currentTime >= m_frameTime && (m_currentFrameIndex < m_endFrame || m_looped))
    {
        m_currentFrameIndex++;
        if (m_currentFrameIndex == m_endFrame) m_currentFrameIndex = m_startFrame;
        m_nextFrameIndex = (m_currentFrameIndex + 1u) % m_frameCount;
        m_currentTime = 0.f;
    }
    interpolate(m_skeleton->m_keyFrames[m_currentFrameIndex], m_skeleton->m_keyFrames[m_nextFrameIndex], m_currentTime / m_frameTime);
}

//private
void Skeleton::Animation::buildFirstFrame()
{
    XY_ASSERT(m_skeleton, "Animation has no skeleton associated");

    auto size = m_skeleton->m_keyFrames[m_startFrame].size();
    m_currentFrame.resize(size);

    const auto& boneIndices = m_skeleton->m_jointIndices;
    for (auto i = 0u; i < size; ++i)
    {
        const auto& mat = m_skeleton->m_keyFrames[m_startFrame][i];       
        if (boneIndices[i] >= 0)
        {
            m_currentFrame[i] = m_currentFrame[boneIndices[i]] * mat;
        }
        else
        {
            m_currentFrame[i] = mat;
        }
    }
}

void Skeleton::Animation::interpolate(const std::vector<glm::mat4>& a, const std::vector<glm::mat4>& b, float time)
{
    auto size = m_skeleton->m_keyFrames[m_startFrame].size();
    const auto& boneIndices = m_skeleton->m_jointIndices;
    for (auto i = 0u; i < size; ++i)
    {
        glm::mat4 mat(glm::mix(a[i], b[i], time));
        if (boneIndices[i] >= 0)
        {
            m_currentFrame[i] = m_currentFrame[boneIndices[i]] * mat;
        }
        else
        {
            m_currentFrame[i] = mat;
        }
    }
}