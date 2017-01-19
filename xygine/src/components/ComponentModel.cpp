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

#include <xygine/components/Model.hpp>
#include <xygine/mesh/Mesh.hpp>
#include <xygine/mesh/SubMesh.hpp>
#include <xygine/detail/GLCheck.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/Reports.hpp>
#include <xygine/Scene.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

using namespace xy;

Model::Model(MessageBus& mb, const Mesh& mesh, const MeshRenderer::Lock&)
    : Component         (mb, this),
    m_meshRenderer      (nullptr),
    m_scale             (1.f, 1.f, 1.f),
    m_needsUpdate       (false),
    m_depthScale        (1.f),
    m_mesh              (mesh),
    m_visible           (false),
    m_material          (nullptr),
    m_skeleton          (nullptr),
    m_currentAnimation  (0),
    m_nextAnimation     (-1),
    m_blendTime         (0.f),
    m_currentBlendTime  (0.f),
    m_playbackRate      (1.f)
{
    m_subMaterials.resize(mesh.getSubMeshCount());
    std::fill(m_subMaterials.begin(), m_subMaterials.end(), nullptr);

    m_boundingBox = mesh.getBoundingBox();
}

void Model::entityUpdate(Entity& entity, float dt)
{
    const auto position = entity.getWorldPosition();
    m_worldMatrix = glm::translate(glm::mat4(), glm::vec3(position.x, position.y, 0.f));
    
    const float rotation = xy::Util::Const::degToRad * entity.getRotation();
    m_worldMatrix = glm::rotate(m_worldMatrix, rotation, glm::vec3(0.f, 0.f, 1.f));

    const auto scale = entity.getScale();
    m_worldMatrix = glm::scale(m_worldMatrix, glm::vec3(scale.x, scale.y, (scale.x + scale.y) / 2.f));

    if (m_needsUpdate) updateTransform();
    m_worldMatrix *= m_preTransform;

    //update the bounds of the model depending on its depth
    m_globalBounds = m_boundingBox.asFloatRect();
    //find distance from camera position, scale and add scale difference to global bounds
    auto distFromCam = position - m_meshRenderer->m_scene.getView().getCenter();
    auto scaledDist = distFromCam * m_depthScale;
    scaledDist -= distFromCam;
    m_globalBounds.left += scaledDist.x;
    m_globalBounds.top += scaledDist.y;

    m_worldBounds = entity.getWorldTransform().transformRect(m_globalBounds);
    //-----------------------------------------------------

    //animate if exists and visible on screen
    if (!m_animations.empty() && m_visible)
    {
        if (m_nextAnimation == -1)
        {
            m_animations[m_currentAnimation].update(dt * m_playbackRate, m_currentFrame);
        }
        else
        {
            //blend animations
            m_currentBlendTime += dt;
            const float ratio = std::min(m_currentBlendTime / m_blendTime, 1.f);

            auto size = m_skeleton->getFrame(0).size();
            const auto& boneIndices = m_skeleton->getJointIndices();
            const auto& a = m_skeleton->getFrame(m_animations[m_currentAnimation].getCurrentFrame());
            const auto& b = m_skeleton->getFrame(m_animations[m_nextAnimation].getStartFrame());
            for (auto i = 0u; i < size; ++i)
            {
                glm::mat4 mat(glm::mix(a[i], b[i], ratio));
                if (boneIndices[i] >= 0)
                {
                    m_currentFrame[i] = m_currentFrame[boneIndices[i]] * mat;
                }
                else
                {
                    m_currentFrame[i] = mat;
                }
            }

            if (ratio == 1)
            {
                m_currentAnimation = m_nextAnimation;
                m_animations[m_currentAnimation].play(0);
                m_nextAnimation = -1;
            }
        }
    }
}

void Model::setBaseMaterial(const Material& material, bool applyToAll)
{
    //check for old materials so we can remove vertex attrib
    //bindings should old material shaders no longer be used
    std::vector<ShaderID> oldShaders(RenderPass::ID::Count);
    if (m_material)
    {
        for (auto i = 0; i < RenderPass::ID::Count; ++i)
        {
            if (m_material->setActivePass(static_cast<RenderPass::ID>(i)))
            {
                oldShaders[i] = m_material->getShader().getNativeHandle();
            }
        }
    }

    m_material = &material;
    if (applyToAll && m_mesh.getSubMeshCount() > 0)
    {
        //this will also update the vertex attribs
        for (auto i = 0u; i < m_mesh.getSubMeshCount(); ++i)
        {
            setSubMaterial(material, i);
        }
    }
    else //update them explicitly
    {        
        for (auto i = 0; i < RenderPass::ID::Count; ++i)
        {
            if (m_material->setActivePass(static_cast<RenderPass::ID>(i)))
            {
                updateVertexAttribs(m_material->getShader().getNativeHandle(), material);
            }
        }

        for(auto id : oldShaders) removeUnusedAttribs(id);
    }
}

void Model::setSubMaterial(const Material& material, std::size_t idx)
{
    if (idx < m_mesh.getSubMeshCount())
    {
        //store old shaders we might want to remove
        std::vector<ShaderID> oldShaders(RenderPass::ID::Count);
        if (m_subMaterials[idx])
        {
            for (auto i = 0; i < RenderPass::ID::Count; ++i)
            {
                if (m_subMaterials[idx]->setActivePass(static_cast<RenderPass::ID>(i)))
                {
                    oldShaders[i] = m_subMaterials[idx]->getShader().getNativeHandle();
                }
            }
        }

        m_subMaterials[idx] = &material;

        //update vert attribs
        for (auto i = 0; i < RenderPass::ID::Count; ++i)
        {
            if (m_subMaterials[idx]->setActivePass(static_cast<RenderPass::ID>(i)))
            {
                updateVertexAttribs(material.getShader().getNativeHandle(), material);
            }
        }
        //and remove unused
        for (auto id : oldShaders) removeUnusedAttribs(id);
    }
}

void Model::rotate(Model::Axis axis, float rotation)
{
    glm::vec3 normal;
    switch(axis)
    {
        default: break;
    case Axis::X:
        normal.x = 1.f;
        break;
    case Axis::Y:
        normal.y = 1.f;
        break;
    case Axis::Z:
        normal.z = 1.f;
        break;
    }
    m_rotation = glm::rotate(m_rotation, xy::Util::Const::degToRad * rotation, normal);
    m_needsUpdate = true;
}

void Model::setRotation(const sf::Vector3f& rotation)
{
    m_rotation = glm::toQuat(glm::orientate3(glm::vec3(rotation.x, rotation.y, rotation.z)));
    m_needsUpdate = true;
}

void Model::setPosition(const sf::Vector3f& position)
{
    m_translation = { position.x, position.y, position.z };
    m_needsUpdate = true;
}

void Model::setScale(const sf::Vector3f& scale)
{
    m_scale = { scale.x, scale.y, scale.y };
    m_needsUpdate = true;
}

float Model::getRotation(Model::Axis axis) const
{
    glm::vec3 rotation = glm::eulerAngles(m_rotation);

    switch (axis)
    {
    default: break;
    case Axis::X: return rotation.x * xy::Util::Const::radToDeg;
    case Axis::Y: return rotation.y * xy::Util::Const::radToDeg;
    case Axis::Z: return rotation.z * xy::Util::Const::radToDeg;
    }
    return 0.f;
}

void Model::setSkeleton(const Skeleton& s)
{
    m_skeleton = &s;
    for (auto& a : m_animations)
    {
        a.setSkeleton(m_skeleton);
    }

    buildFirstFrame();
}

void Model::addAnimation(const Skeleton::Animation& animation)
{
    bool empty = m_animations.empty();
    m_animations.push_back(animation);

    if (m_skeleton)
    {
        m_animations.back().setSkeleton(m_skeleton);
        if (empty)
        {
            buildFirstFrame();
        }
    }
}

void Model::setAnimations(const std::vector<Skeleton::Animation>& animations)
{
    m_animations = animations;
    if (m_skeleton)
    {
        for (auto& a : m_animations)
        {
            a.setSkeleton(m_skeleton);
        }
        buildFirstFrame();
    }
}

void Model::setPlaybackRate(float rate)
{
    XY_ASSERT(rate >= 0, "Playback rate must be positive");
    m_playbackRate = rate;
}

void Model::playAnimation(std::size_t index, float fade)
{
    XY_ASSERT(fade > 0, "would cause div by zero!");
    if (index < m_animations.size() 
        && index != m_currentAnimation
        && index != m_nextAnimation)
    {
        m_blendTime = fade;
        m_currentBlendTime = 0.f;
        m_nextAnimation = index;
    }
}

//private
void Model::updateTransform()
{
    m_preTransform = glm::mat4();
    m_preTransform = glm::translate(m_preTransform, m_translation);
    m_preTransform *= glm::toMat4(m_rotation);
    m_preTransform = glm::scale(m_preTransform, m_scale);

    m_boundingBox = m_mesh.getBoundingBox();
    m_boundingBox.transform(m_preTransform);

    update2DScale();

    m_needsUpdate = false;
}

void Model::update2DScale()
{
    float depth = m_meshRenderer->m_cameraZ - m_translation.z;
    float angle = (m_meshRenderer->m_fov * xy::Util::Const::degToRad) / 2.f;
    float relHeight = std::tan(angle) * depth;

    auto sceneViewSize = m_meshRenderer->m_scene.getView().getSize();
    m_depthScale = (sceneViewSize.y / 2.f) / relHeight;
    m_boundingBox.setDepthScale(m_depthScale);
}

std::size_t Model::draw(const glm::mat4& viewMatrix, const sf::FloatRect& visibleArea, RenderPass::ID pass) const
{
    m_visible = m_worldBounds.intersects(visibleArea);
    if (!m_visible) return 0;
    
    glm::mat4 worldViewMat = viewMatrix * m_worldMatrix;

    if (m_mesh.getSubMeshCount() > 0)
    {
        auto count = m_mesh.getSubMeshCount();
        std::size_t ret = 0;
        for (auto i = 0u; i < count; ++i)
        {
            if (!m_subMaterials[i]->setActivePass(pass)) continue;
            m_subMaterials[i]->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
            m_subMaterials[i]->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
            m_subMaterials[i]->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(m_worldMatrix)))));
            m_subMaterials[i]->bind();
            setBones(m_subMaterials[i]->getShader(), m_subMaterials[i]->getSkinID());
            auto vao = m_vaoBindings.find(m_subMaterials[i]->getShader().getNativeHandle());
            vao->second.bind();
            const auto subMesh = m_mesh.getSubMesh(i);
            glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMesh->getIndexBufferID()));
            glCheck(glDrawElements(static_cast<GLenum>(subMesh->getPrimitiveType()), subMesh->getIndexCount(), static_cast<GLenum>(subMesh->getIndexFormat()), 0));
            vao->second.unbind();
            ret = 1;
        }
        return ret;
    }
    else
    {
        if(!m_material->setActivePass(pass)) return 0;
        m_material->getShader().setUniform("u_worldMatrix", sf::Glsl::Mat4(glm::value_ptr(m_worldMatrix)));
        m_material->getShader().setUniform("u_worldViewMatrix", sf::Glsl::Mat4(glm::value_ptr(worldViewMat)));
        m_material->getShader().setUniform("u_normalMatrix", sf::Glsl::Mat3(glm::value_ptr(glm::inverseTranspose(glm::mat3(m_worldMatrix)))));
        m_material->bind();
        setBones(m_material->getShader(), m_material->getSkinID());
        auto vao = m_vaoBindings.find(m_material->getShader().getNativeHandle());
        vao->second.bind();
        glCheck(glDrawArrays(static_cast<GLenum>(m_mesh.getPrimitiveType()), 0, m_mesh.getVertexCount()));
        vao->second.unbind();
    }
    return 1;
}

void Model::updateVertexAttribs(ShaderID shader, const Material& mat)
{
    //if we don't yet have a VAO for this material
    //create one
    if (m_vaoBindings.count(shader) == 0)
    {
        m_vaoBindings.insert(std::make_pair(shader, VertexAttribBinding(m_mesh, mat)));
    }
}

void Model::removeUnusedAttribs(ShaderID id)
{
    //count instances of old shader used, and remove binding if there are none
    if (m_material->getShader().getNativeHandle() == id) return;
    for (auto& m : m_subMaterials)
    {
        if (m && m->getShader().getNativeHandle() == id)
        {
            return;
        }
    }
    m_vaoBindings.erase(id);
}

void Model::setBones(sf::Shader& shader, UniformID id) const
{
    if (m_skeleton && id > -1)
    {
        glCheck(glUniformMatrix4fv(id, m_currentFrame.size(), GL_FALSE, glm::value_ptr(m_currentFrame[0])));
    }
}

void Model::buildFirstFrame()
{
    if (!m_animations.empty())
    {
        const auto& frame = m_skeleton->getFrame(0);
        auto size = frame.size();
        m_currentFrame.resize(size);

        const auto& boneIndices = m_skeleton->getJointIndices();
        for (auto i = 0u; i < size; ++i)
        {
            if (boneIndices[i] >= 0)
            {
                m_currentFrame[i] = m_currentFrame[boneIndices[i]] * frame[i];
            }
            else
            {
                m_currentFrame[i] = frame[i];
            }
        }
    }
}