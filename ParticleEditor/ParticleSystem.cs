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
using System.Diagnostics;

namespace ParticleEditor
{
    class Crosshair : Transformable, Drawable
    {
        private VertexArray m_vertexArray = new VertexArray(PrimitiveType.LinesStrip);
        public Crosshair()
        {
            const float halfWidth = 5f;
            Vertex v = new Vertex(new Vector2f(halfWidth, 0f), SFML.Graphics.Color.White);
            m_vertexArray.Append(v);
            v.Position = new Vector2f(-halfWidth, 0f);
            m_vertexArray.Append(v);
            v.Color = SFML.Graphics.Color.Transparent;
            m_vertexArray.Append(v);
            v.Position = new Vector2f(0f, -halfWidth);
            m_vertexArray.Append(v);
            v.Color = SFML.Graphics.Color.White;
            m_vertexArray.Append(v);
            v.Position = new Vector2f(0f, halfWidth);
            m_vertexArray.Append(v);
        }


        public void Draw(RenderTarget target, RenderStates states)
        {
            states.Transform *= Transform;
            target.Draw(m_vertexArray, states);
        }
    }

    class ParticleSystem : Drawable
    {
        public ParticleSystem()
        {
            colour = SFML.Graphics.Color.White;
            blendMode = BlendMode.Add;
        }

        private Texture m_texture;
        public Texture texture
        {
            get
            {
                return m_texture;
            }
            set
            {
                m_texture = value;
                if (value != null) m_texCoords = new Vector2f(m_texture.Size.X, m_texture.Size.Y);
            }
        }
        public SFML.Graphics.Color colour { get; set; }
        public BlendMode blendMode { get; set; }
        public Shader shader { get; set; }

        private Vector2f m_particleSize = new Vector2f(2f, 2f);
        public Vector2f particleSize { set { m_particleSize = value; } }
        private Vector2f m_position = new Vector2f();
        public Vector2f position { set { m_position = value; } get { return m_position; } }
        private bool m_followParent = false;
        public bool followParent { set { m_followParent = value; } }
        private float m_particleLifetime = 2f;
        public float particleLifetime { set { m_particleLifetime = value; } get { return m_particleLifetime; } }
        private Vector2f m_initialVelocity = new Vector2f();
        public Vector2f initialVelocity { set { m_initialVelocity = value; } get { return m_initialVelocity; } }
        private List<Vector2f> m_randomInitialVelocities = new List<Vector2f>();
        public List<Vector2f> randomInitialVelocities
        {
            set
            {
                m_randomInitialVelocities = value;
                m_randomVelocity = (value == null)? false : true;
            }
            get
            {
                return m_randomInitialVelocities;
            }
        }
        private float m_emitRate = 30f;
        public float emitRate { set { m_emitRate = value; } }
        private List<Vector2f> m_randomInitialPositions = new List<Vector2f>();
        public List<Vector2f> randomInitialPositions
        {
            set
            {
                m_randomInitialPositions = value;
                m_randomPosition = (value == null) ? false : true;
            }
            get
            {
                return m_randomInitialPositions;
            }
        }

        private List<IAffector> m_affectors = new List<IAffector>();
        public void addAffector(IAffector affector)
        {
            m_affectors.Add(affector);
        }
        public List<IAffector> Affectors { get { return m_affectors; } set { m_affectors = value; } }

        private Queue<Particle> m_particles = new Queue<Particle>();
        public bool active { get { return (m_particles.Count > 0); } }

        private bool m_started = false;
        public bool started { get { return m_started; } }


        private float m_releaseCount = 1f;
        private float m_duration = 1;
        private float m_startDelay = 0f;
        private Stopwatch m_durationTimer = new Stopwatch();
        public void start(byte releaseCount, float startDelay, float duration)
        {
            Debug.Assert(releaseCount > 0);
            //Debug.Assert(duration >= 0);

            m_releaseCount = releaseCount;
            m_duration = duration;
            m_durationTimer.Restart();
            m_startDelay = startDelay;

            m_started = true;
        }

        public void stop()
        {
            m_started = false;
        }

        private bool m_needsUpdate = true;
        public void update(float dt)
        {
            //clear dead
            while (m_particles.Count != 0 && m_particles.First().Lifetime <=0)
            {
                m_particles.Dequeue();
            }

            //update existing
            foreach(var p in m_particles)
            {
                p.Lifetime -= dt;
                p.Position += p.Velocity * dt;
                foreach(var a in m_affectors)
                {
                    a.update(p, dt);
                }
            }

            m_needsUpdate = true;

            if (m_started)
            {
                if (m_startDelay > 0)
                {
                    m_startDelay -= dt;
                    return;
                }

                emit(dt);
                if(m_duration > 0)
                {
                    if ((m_durationTimer.ElapsedMilliseconds / 1000f) > m_duration)
                    {
                        m_started = false;
                    }
                }
            }
            m_crosshair.Position = m_position;
        }

        public void move(Vector2f distance)
        {
            m_position += distance;
        }

        private VertexArray m_vertices = new VertexArray(PrimitiveType.Quads);
        private Crosshair m_crosshair = new Crosshair();
        public void Draw(RenderTarget target, RenderStates states)
        {
            if(m_needsUpdate)
            {
                updateVertices();
                m_needsUpdate = false;
            }

            states.Texture = texture;
            states.Shader = shader;
            states.BlendMode = blendMode;
            if (!m_followParent) states.Transform = Transform.Identity;
            target.Draw(m_vertices, states);
            target.Draw(m_crosshair);
        }


        //-----private-----//
        private Random m_random = new Random();
        private float m_accumulator = 0f;
        private bool m_randomPosition = false;
        private void emit(float dt)
        {
            float interval = 1f / m_emitRate;

            m_accumulator += dt;
            while (m_accumulator > interval)
            {
                m_accumulator -= interval;
                for(var i = 0; i < m_releaseCount; ++i)
                {
                    if (m_randomPosition)  addParticle(m_position + m_randomInitialPositions[m_random.Next(0, m_randomInitialPositions.Count)]);
                    else addParticle(m_position);
                }
            }
        }

        private bool m_randomVelocity = false;
        private void addParticle(Vector2f position)
        {
            Particle p = new Particle();
            p.Position = position;
            p.Colour = colour;
            p.Lifetime = m_particleLifetime;
            p.Velocity = (m_randomVelocity) ?
                m_randomInitialVelocities[m_random.Next(0, m_randomInitialVelocities.Count)] : m_initialVelocity;

            m_particles.Enqueue(p);
        }

        private void addVertex(Vector2f position, float u, float v, SFML.Graphics.Color colour)
        {
            Vertex vert = new Vertex();
            vert.Position = position;
            vert.TexCoords = new Vector2f(u, v);
            vert.Color = colour;

            m_vertices.Append(vert);
        }

        private Vector2f m_texCoords = new Vector2f();
        private void updateVertices()
        {
            Vector2f halfSize = m_particleSize / 2f;

            m_vertices.Clear();
            foreach(var p in m_particles)
            {
                var colour = p.Colour;

                //fades particles
                float ratio = p.Lifetime / m_particleLifetime;
                colour.A = (byte)(255f * Math.Max(ratio, 0f));

                var t = p.Transform;
                addVertex(t.TransformPoint(-halfSize.X, -halfSize.Y), 0f, 0f, colour);
                addVertex(t.TransformPoint(halfSize.X, -halfSize.Y), m_texCoords.X, 0f, colour);
                addVertex(t.TransformPoint(halfSize), m_texCoords.X, m_texCoords.Y, colour);
                addVertex(t.TransformPoint(-halfSize.X, halfSize.Y), 0f, m_texCoords.Y, colour);
            }
        }
    }
}
