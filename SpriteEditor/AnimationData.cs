/*********************************************************************
Matt Marchant 2015
http://trederia.blogspot.com

xygine Sprite Editor - Zlib license.

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

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SpriteEditor
{
    class AnimationData
    {
        private List<Animation> m_animations;
        public List<Animation> Animations
        {
            get { return m_animations; }
            set { m_animations = value; }
        }

        private uint m_frameCount;
        public uint FrameCount
        {
            get { return m_frameCount; }
        }
        private Size m_frameSize;
        public Size FrameSize
        {
            get { return m_frameSize; }
        }
        private float m_frameRate;
        public float FrameRate
        {
            get { return m_frameRate; }
        }
        private string m_texture;
        public string Texture
        {
            get { return m_texture; }
        }
        private string m_normalMap;
        public string NormalMap
        {
            get { return m_normalMap; }
        }

        public AnimationData(uint frameCount, Size frameSize, float frameRate, string texture, string normalMap)
        {
            m_frameCount = frameCount;
            m_frameSize = frameSize;
            m_frameRate = frameRate;
            m_texture = texture;
            m_normalMap = normalMap;
            m_animations = new List<Animation>();
        }
    }
}
