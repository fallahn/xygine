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

#ifndef XY_SKELETON_HPP_
#define XY_SKELETON_HPP_

#include <xygine/Config.hpp>

#include <glm/mat4x4.hpp>

#include <vector>
#include <string>

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
            \brief Sets the skeleton to which this animation is applied
            */
            void setSkeleton(const Skeleton*);
            /*!
            \brief Updates the animation based on the give frame time
            \param float current frame time
            \param std::vector<glm::mat4>& Destination for updated frame
            \param bool Low-res. When true low resolution updates are performed
            using only keyframes with no interpolation. For instance when an
            animation is updated in the distance using low-res updates can
            improve performance.
            */
            void update(float, std::vector<glm::mat4>&, bool = false);

            /*!
            \brief Starts the animation playing from the given frame.
            \param startFrame Offset value from this animations first
            frame. For example if the animation starts at frame 20 then
            passing in a value of 1 will play from frame 21.
            */
            void play(std::size_t);

            /*!
            \brief Returns the first frame of this animation
            */
            std::size_t getStartFrame() const { return m_startFrame; }

            /*!
            \brief Returns the current frame of this animation
            */
            std::size_t getCurrentFrame() const { return m_currentFrameIndex; }

        private:
            std::size_t m_frameCount;
            std::size_t m_startFrame;
            std::size_t m_endFrame;
            float m_frameRate;
            std::string m_name;
            bool m_looped;
            bool m_playing;

            const Skeleton* m_skeleton;
            float m_currentTime;
            float m_frameTime;
            std::size_t m_currentFrameIndex;
            std::size_t m_nextFrameIndex;

            void interpolate(const std::vector<glm::mat4>&, const std::vector<glm::mat4>&, float, std::vector<glm::mat4>&);
            void copy(const std::vector<glm::mat4>&, std::vector<glm::mat4>&);
        };

        /*!
        \brief Constructor.
        \param jointIndices Index array in to joint list
        \param keyFrames vector of frames representing key frames in the animation of this skeleton
        */
        Skeleton(const std::vector<std::int32_t>& jointIndices, const std::vector<std::vector<glm::mat4>>& keyFrames);
        ~Skeleton() = default;
        Skeleton(Skeleton&&) noexcept = default;
        Skeleton& operator = (Skeleton&&) = default;

        /*!
        \brief Returns the keyframe at the given index
        */
        const std::vector<glm::mat4>& getFrame(std::size_t) const;

        /*!
        \brief Returns a reference to the lists of joint indices
        */
        const std::vector<std::int32_t>& getJointIndices() const { return m_jointIndices; }

    private:
        std::vector<std::int32_t> m_jointIndices;
        std::vector<std::vector<glm::mat4>> m_keyFrames;
        
    };
}

#endif //XY_SKELETON_HPP_