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

using SFML.Graphics;
using SFML.Window;

namespace ParticleEditor
{
    class ParticleSystem : Drawable
    {
        public Texture texture { get; set; }
        public SFML.Graphics.Color colour { get; set; }
        public BlendMode blendMode { get; set; }
        public Shader shader { get; set; }

        private Vector2f m_particleSize;
        public Vector2f particleSize { set { m_particleSize = value; } }
        private Vector2f m_position;
        public Vector2f position { set { m_position = value; } }
        private bool m_followParent;
        public bool followParent { set { m_followParent = value; } }
        private float m_particleLifetime;
        public float particleLifetime { set { m_particleLifetime = value; } }
        private Vector2f m_initialVelocity;
        public Vector2f initialVelocity { set { m_initialVelocity = value; } }
        private List<Vector2f> m_randomInitialVelocities;
        public List<Vector2f> randonInitialVelocities { set { m_randomInitialVelocities = value; } }
        private float m_emitRate;
        public float emitRate { set { m_emitRate = value; } }
        private List<Vector2f> m_randomInitialPositions;
        public List<Vector2f> randomInitialPositions { set { m_randomInitialPositions = value; } }

        //TODO add affectors


        private bool m_started = false;
        public bool started { get { return m_started; } }

        public void start()
        {

        }

        public void stop()
        {

        }

        void update(float dt)
        {

        }

        public void move(Vector2f distance)
        {

        }


        public void Draw(RenderTarget target, RenderStates states)
        {
            
        }


        //-----private-----//
        private void emit(float dt)
        {

        }

        private void addParticle(Vector2f position)
        {

        }

        private void addVertex(Vector2f position, float u, float v, SFML.Graphics.Color colour)
        {

        }

        private void updatreVertices()
        {

        }
    }
}
