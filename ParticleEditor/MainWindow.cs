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
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using SFML.Graphics;
using SFML.Window;
using System.Diagnostics;

namespace ParticleEditor
{
    public partial class MainWindow : Form
    {
        private SfmlControl m_sfmlControl = new SfmlControl();
        private ParticleSystem m_particleSystem = new ParticleSystem();

        private void drawParticleSystem(RenderWindow rw)
        {
            rw.Draw(m_particleSystem);
        }

        public MainWindow()
        {
            InitializeComponent();
            //this.

            //add sfml control to layout
            splitContainer2.Panel1.Controls.Add(m_sfmlControl);
            m_sfmlControl.Dock = DockStyle.Fill;

            m_sfmlControl.DrawDelegates.Add(drawParticleSystem);
            m_sfmlControl.UpdateDelegates.Add(m_particleSystem.update);
            numericUpDownDuration.Value = -1;

            this.StartPosition = FormStartPosition.CenterScreen;

            listBoxSpawnPoints.ContextMenu = new ContextMenu();
            listBoxSpawnPoints.ContextMenu.MenuItems.Add("Delete");
            listBoxSpawnPoints.ContextMenu.MenuItems[0].Click += itemDelete_click;
            listBoxSpawnPoints.ContextMenu.MenuItems.Add("Clear");
            listBoxSpawnPoints.ContextMenu.MenuItems[1].Click += itemsClear_click;

            listBoxSpawnVelocities.ContextMenu = new ContextMenu();
            listBoxSpawnVelocities.ContextMenu.MenuItems.Add("Delete");
            listBoxSpawnVelocities.ContextMenu.MenuItems[0].Click += itemDelete_click;
            listBoxSpawnVelocities.ContextMenu.MenuItems.Add("Clear");
            listBoxSpawnVelocities.ContextMenu.MenuItems[1].Click += itemsClear_click;
        }



        /// <summary>
        /// Updates the sfml control periodically
        /// </summary>
        public void DispatchDrawingEvents()
        {
            updateMotion();

            m_sfmlControl.HandleEvents();
            m_sfmlControl.Draw();
        }

        private Stopwatch m_movementTimer = new Stopwatch();
        private float m_angle = 0f;
        private float m_radius = 50f;
        private float tau = (float)Math.PI * 2f;
        private float m_speed = 200f;
        private float m_xVal = 200f;
        private void updateMotion()
        {
            float frameTime = m_movementTimer.ElapsedMilliseconds / 1000f;
            m_angle += frameTime;

            if (m_angle > tau)
            {
                m_angle -= tau;
            }
            m_movementTimer.Restart();

            if (m_particleSystem.position.X > m_speed
                || m_particleSystem.position.X < -m_speed) m_xVal = -m_xVal;

            Vector2f pos = new Vector2f
            (
                m_particleSystem.position.X + m_xVal * frameTime,
                (float)Math.Cos(m_angle) * m_radius
            );
            m_particleSystem.position = pos;

            //reset button label when finished
            if(!m_particleSystem.started)
            {
                buttonStart.Text = "Start";
            }
        }
    }
}
