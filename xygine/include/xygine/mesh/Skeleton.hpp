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

#ifndef XY_SKELETON_HPP_
#define XY_SKELETON_HPP_

#include <xygine/Config.hpp>

#include <glm/mat4x4.hpp>

#include <vector>

namespace xy
{
    /*!
    \brief Represents a hierachy of joints used to animated 3D meshes.
    */
    class XY_EXPORT_API Skeleton final
    {
    public:
        /*!
        \brief struct containing information on how to interpolate
        a given series of keyframes in a Skeleton which make up an
        animation.
        */
        struct XY_EXPORT_API Animation final
        {
        public:
            /*!
            \brief Constructor.
            \param frameCount Number of frames in this animation
            \param startFrame First frame of the animation
            \param frameRate Number of frames to display per second
            \param name Name of the animation clip
            \param looped true to loop the animation, false to stop
            once the animation has finished playing.
            */
            Animation(std::size_t frameCount, std::size_t startFrame, float frameRate, const std::string& name, bool looped = true);
            ~Animation() = default;

            /*!
            \brief Returns the current frame.
            The current frame is the calculated interpolated frame based
            on the current animation time and the nearest two key frames.
            \returns Pointer to array of matrices to send directly to skinning shader
            */
            const std::vector<glm::mat4>& getCurrentFrame() const;
            /*!
            \brief Sets the skeleton to which this animation is applied
            */
            void setSkeleton(Skeleton*);
            /*!
            \brief Updates the animation based on the give frame time
            */
            void update(float);

        private:
            std::size_t m_frameCount;
            std::size_t m_startFrame;
            std::size_t m_endFrame;
            float m_frameRate;
            std::string m_name;
            bool m_looped;

            std::vector<glm::mat4> m_currentFrame; //<TODO pass this in to update as it's shared with all animations
            Skeleton* m_skeleton;
            float m_currentTime;
            float m_frameTime;
            std::size_t m_currentFrameIndex;
            std::size_t m_nextFrameIndex;

            void buildFirstFrame();
            void interpolate(const std::vector<glm::mat4>&, const std::vector<glm::mat4>&, float);
        };

        /*!
        \brief Constructor.
        \param jointIndices Index array in to joint list
        \param keyFrames vector of frames representing key frames in the animation of this skeleton
        */
        Skeleton(const std::vector<std::int32_t>& jointIndices, const std::vector<std::vector<glm::mat4>>& keyFrames);
        ~Skeleton() = default;

        /*!
        \brief Returns the keyframe at the given index
        */
        const std::vector<glm::mat4>& getFrame(std::size_t) const;

    private:
        std::vector<std::int32_t> m_jointIndices;
        std::vector<std::vector<glm::mat4>> m_keyFrames;
        
    };
}

#endif //XY_SKELETON_HPP_