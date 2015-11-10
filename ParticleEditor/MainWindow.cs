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

            //add sfml control to layout
            splitContainer3.Panel2.Controls.Add(m_sfmlControl);
            m_sfmlControl.Dock = DockStyle.Fill;

            m_sfmlControl.DrawDelegates.Add(drawParticleSystem);
            m_sfmlControl.UpdateDelegates.Add(m_particleSystem.update);
            numericUpDownDuration.Value = -1;

            this.StartPosition = FormStartPosition.CenterScreen;

            comboBoxBlendMode.DataSource = Enum.GetValues(typeof(BlendMode));
            comboBoxBlendMode.SelectedItem = BlendMode.Add;

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

            numericUpDownSizeX.ValueChanged += sizeChanged;
            numericUpDownSizeY.ValueChanged += sizeChanged;

            numericUpDownInitVelX.ValueChanged += velocityChanged;
            numericUpDownInitVelY.ValueChanged += velocityChanged;

            panelColour.Click += colour_Click;
            panelColour.BackColorChanged += PanelColour_BackColorChanged;
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

        /// <summary>
        /// Updates the movement of the particle system
        /// </summary>
        private Stopwatch m_movementTimer = new Stopwatch();
        private float xTarget = 200f;
        private float yTarget = 80f;
        private void updateMotion()
        {
            float frameTime = m_movementTimer.ElapsedMilliseconds / 1000f;
            m_movementTimer.Restart();

            Vector2f velocity = new Vector2f();
            velocity.X = (xTarget - m_particleSystem.position.X);
            velocity.Y = (yTarget - m_particleSystem.position.Y);
            m_particleSystem.position += (velocity * frameTime);

            if (Math.Abs(velocity.X) < 50f) xTarget = -xTarget;
            if (Math.Abs(velocity.Y) < 50f) yTarget = -yTarget;

            //reset button label when finished
            if(!m_particleSystem.started)
            {
                buttonStart.Text = "Start";
            }
        }
    }
}
