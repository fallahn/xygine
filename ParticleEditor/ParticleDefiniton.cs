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
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using SFML.Graphics;
using SFML.Window;

namespace ParticleEditor
{
    /// <summary>
    /// Information used to render a single particle
    /// </summary>
    class Particle : Transformable
    {
        public Vector2f Velocity { get; set; }
        public SFML.Graphics.Color Colour { get; set; }
        new public float Rotation { get; set; }
        public float Lifetime { get; set; }
        FloatRect TextureRect { get; set; }
    }

    //TODO write other project settings such as enabeld movement and back colour
    
    /// <summary>
    /// represents the settings imported / exported between editor and xygine
    /// </summary>
    class ParticleDefiniton
    {
        /// <summary>
        /// How many particles are released when system started
        /// </summary>
        public byte ReleaseCount { get; set; }
        /// <summary>
        /// Delay in seconds before particle system starts to emit
        /// </summary>
        public float Delay { get; set; }
        /// <summary>
        /// Amount of time in seconds for system to release particles.
        /// A negative value means continual release
        /// </summary>
        public float Duration { get; set; }
        /// <summary>
        /// Relative path to a texture on disk
        /// </summary>
        public string Texture { get; set; }
        /// <summary>
        /// Colour with which to mulitply the particles
        /// </summary>
        public Int32 Colour { get; set; }
        /// <summary>
        /// Particle system blend mode. Additive by default
        /// </summary>
        public string BlendMode { get; set; }
        /// <summary>
        /// ID of shader to use when rendering particle systems (unused)
        /// </summary>
        public string Shader { get; set; }
        /// <summary>
        /// Size of particles on release (can be overrided by affectors)
        /// </summary>
        public Size ParticleSize { get; set; }
        /// <summary>
        /// Initial position of particles relative to emitter
        /// </summary>
        public Point ParticlePosition { get; set; }
        /// <summary>
        /// Whether or not a particle should continue to follow its emitter
        /// after it has been spawned
        /// </summary>
        public bool FollowParent { get; set; }
        /// <summary>
        /// Lifetime in seconds of particles once they have been spawned
        /// </summary>
        public float Lifetime { get; set; }
        /// <summary>
        /// Initial velocity (speed & direction) of particles when spawned
        /// </summary>
        public Point InitialVelocity { get; set; }
        /// <summary>
        /// List of initial velocities of which one is picked at random
        /// each time a new particle is spawned. Overrides InitialVelocity
        /// value if this is set
        /// </summary>
        public List<Point> RandomInitialVelocites { get; set; }
        /// <summary>
        /// Number of particles to emit in a second
        /// </summary>
        public float EmitRate { get; set; }
        /// <summary>
        /// List of initial positions of which one is picked at random each
        /// time a new particle is spawned. Overrides the Position property
        /// if this is set.
        /// </summary>
        public List<Point> RandomInitialPositions { get; set; }
        

        public ParticleDefiniton()
        {
            ReleaseCount = 1;
            Delay = 0f;
            Duration = 1f;
            ParticleSize = new Size(4, 4);
            ParticlePosition = new Point();
            FollowParent = false;
            Lifetime = 2f;
            InitialVelocity = new Point(1, 1);
            EmitRate = 30f;

        }
    }
}
