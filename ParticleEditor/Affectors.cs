/*********************************************************************
Matt Marchant 2015
http://trederia.blogspot.com

xygine Particle Editor - Zlib license.

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
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using SFML.Window;

namespace ParticleEditor
{
    enum AffectorType
    {
        Force,
        Colour,
        Rotation,
        Scale
    }

    interface IAffector
    {
        void update(Particle p, float dt);
        AffectorType type();
    }

    class ForceAffector : IAffector
    {
        private Vector2f m_force;
        ForceAffector(Vector2f force)
        {
            m_force = force;
        }

        public void update(Particle p, float dt)
        {
            p.Velocity += m_force * dt;
        }

        public AffectorType type()
        {
            return AffectorType.Force;
        }
    }

    class ColourAffector : IAffector
    {
        private SFML.Graphics.Color m_start;
        private SFML.Graphics.Color m_end;
        private float m_duration;

        ColourAffector(SFML.Graphics.Color start, SFML.Graphics.Color end, float duration)
        {
            m_start = start; m_end = end; m_duration = duration;
        }
        
        public AffectorType type()
        {
            return AffectorType.Colour;
        }

        public void update(Particle p, float dt)
        {
            float ratio = (m_duration - p.Lifetime) / m_duration;
            ratio = Math.Max(0f, Math.Min(ratio, 1f));

            SFML.Graphics.Color c = new SFML.Graphics.Color
            (
                lerp(m_start.R, m_end.R, ratio),
                lerp(m_start.G, m_end.G, ratio),
                lerp(m_start.B, m_end.B, ratio)
            );
            p.Colour = c;
        }

        private byte lerp(byte start, byte end, float distance)
        {
            float val = (end - start) * distance;
            return (byte)(Convert.ToByte(val) + start);
        }
    }

    class RotationAffector : IAffector
    {
        private float m_rotation;
        RotationAffector(float rotation)
        {
            m_rotation = rotation;
        }

        public AffectorType type()
        {
            return AffectorType.Rotation;
        }

        public void update(Particle p, float dt)
        {
            p.Rotation += m_rotation * dt;
        }
    }

    class ScaleAffector : IAffector
    {
        private Vector2f m_scale;
        ScaleAffector(Vector2f scale)
        {
            m_scale = scale;
        }
        
        public AffectorType type()
        {
            return AffectorType.Scale;
        }

        public void update(Particle p, float dt)
        {
            p.Scale += (m_scale * dt);
        }
    }
}
