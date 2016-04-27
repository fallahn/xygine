/*********************************************************************
Matt Marchant 2015 - 2016
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
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using SFML.Graphics;
using SFML.Window;

namespace SpriteEditor
{
    public struct Animation
    {
        private uint m_startFrame;
        private uint m_endFrame;
        private bool m_loop;
        private string m_name;

        public Animation(string name, uint start, uint end, bool loop = true)
        {
            Debug.Assert(start <= end);
            m_startFrame = start;
            m_endFrame = end;
            m_loop = loop;
            m_name = name;
        }

        public uint Start
        {
            get { return m_startFrame; }
            set { m_startFrame = value; }
        }

        public uint End
        {
            get { return m_endFrame; }
            set { m_endFrame = value; }
        }

        public bool Loop
        {
            get { return m_loop; }
            set { m_loop = value; }
        }

        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }
    }

    public class AnimatedSprite : Transformable, Drawable
    {
        private Sprite m_sprite = new Sprite();
        private Texture m_normalMap = null;
        private Vector2i m_frameSize = new Vector2i();
        private IntRect m_subRect = new IntRect();
        private Vector2u m_textureSize = new Vector2u();

        uint m_frameCount = 0;
        uint m_currentFrame = 0;
        uint m_firstFrame = 0;
        uint m_lastFrame = 0;

        float m_frameRate = 0f;
        float m_elapsedTime = 0f;
        bool m_loop = false;
        bool m_playing = false;

        public SFML.Graphics.Shader BumpShader { get; set; }
        public SFML.Graphics.Texture NormalMap
        {
            get { return m_normalMap; }
            set { m_normalMap = value; }
        }

        public AnimatedSprite() { }

        public AnimatedSprite(Texture t)
        {
            Texture = t;
        }

        public void Update(float dt)
        {
            if(m_playing)
            {
                float frameTime = 1f / m_frameRate;
                m_elapsedTime += dt;

                while (m_elapsedTime >= frameTime && (m_currentFrame < m_lastFrame || m_loop))
                {
                    m_subRect.Left += m_frameSize.X;

                    if(m_subRect.Left + m_subRect.Width > (int)m_textureSize.X)
                    {
                        m_subRect.Left = 0;
                        m_subRect.Top += m_frameSize.Y;
                    }

                    m_elapsedTime -= frameTime;

                    m_currentFrame++;
                    if(m_currentFrame > m_lastFrame)
                    {
                        if(m_loop)
                        {
                            SetFrame(m_firstFrame);
                        }
                        else
                        {
                            m_playing = false;
                        }
                    }
                }
                m_sprite.TextureRect = m_subRect;
            }
        }

        public Texture Texture
        {
            set 
            {
                m_sprite.Texture = value;
                m_textureSize = value.Size;
            }

            get
            {
                return m_sprite.Texture;
            }
        }

        public Vector2i FrameSize
        {
            set
            {
                m_frameSize = value;
                m_subRect.Width = value.X;
                m_subRect.Height = value.Y;
                m_sprite.TextureRect = m_subRect;

                Origin = new Vector2f(value.X / 2, value.Y / 2);
            }

            get
            {
                return m_frameSize;
            }
        }

        public uint FrameCount
        {
            get { return m_frameCount; }
            set { m_frameCount = value; }
        }

        public float FrameRate
        {
            set
            {
                Debug.Assert(value > 0f);
                m_frameRate = value;
            }
            get { return m_frameRate; }
        }

        public bool Loop
        {
            get { return m_loop; }
            set { m_loop = value; }
        }

        public bool Playing
        {
            get { return m_playing; }
        }

        public uint CurrentFrame
        {
            get { return m_currentFrame; }
        }

        public void Play(uint start, uint end)
        {
            Debug.Assert(start < m_frameCount && (end < m_frameCount));

            m_playing = !(start == end);
            m_currentFrame = m_firstFrame = start;
            m_lastFrame = (end < 0) ? m_frameCount - 1 : end;

            //update position of subrect
            SetFrame(start);
        }

        public void Play(Animation anim)
        {
            Loop = anim.Loop;
            Play(anim.Start, anim.End);
        }

        public void SetPaused(bool paused)
        {
            m_playing = !paused;
        }

        public void FrameForward()
        {
            if (m_firstFrame == m_lastFrame) return;
            
            m_currentFrame++;
            if (m_currentFrame > m_lastFrame)
                m_currentFrame = 0;

            SetFrame(m_currentFrame);
        }

        public void FrameBack()
        {
            if (m_firstFrame == m_lastFrame) return;

            m_currentFrame--;
            if (m_currentFrame > m_lastFrame)
                m_currentFrame = m_lastFrame;

            SetFrame(m_currentFrame);
        }

        void Drawable.Draw(RenderTarget target, RenderStates states)
        {
            if(BumpShader != null)
            {
                if (m_normalMap != null)
                {
                    BumpShader.SetParameter("u_diffuseMap", m_sprite.Texture);
                    BumpShader.SetParameter("u_normalMap", m_normalMap);
                    BumpShader.SetParameter("u_inverseWorldViewMatrix", states.Transform.GetInverse());
                }
                states.Shader = BumpShader;
            }

            states.Transform *= this.Transform;
            target.Draw(m_sprite, states);
        }

        private void SetFrame(uint frame)
        {
            Debug.Assert(frame < m_frameCount);

            uint position = m_textureSize.X / (uint)m_frameSize.X;

            Debug.Assert(position > 0);

            var x = frame % position;
            var y = frame / position;

            m_subRect.Left = (int)x * m_frameSize.X;
            m_subRect.Top = (int)y * m_frameSize.Y;

            m_sprite.TextureRect = m_subRect;
            m_currentFrame = frame;
        }
    }
}
