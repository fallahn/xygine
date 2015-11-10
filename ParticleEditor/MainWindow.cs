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
using System.Drawing;

using Newtonsoft.Json;
using System.IO;
using System.Collections.Generic;

namespace ParticleEditor
{
    public partial class MainWindow : Form
    {
        private SfmlControl m_sfmlControl = new SfmlControl();
        private ParticleSystem m_particleSystem = new ParticleSystem();

        private string m_filePath = string.Empty;

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

            numericUpDownEmitRate.ValueChanged += NumericUpDownEmitRate_ValueChanged;

            numericUpDownLifetime.ValueChanged += NumericUpDownLifetime_ValueChanged;

            panelColour.Click += colour_Click;
            panelColour.BackColorChanged += PanelColour_BackColorChanged;

            enableMovementToolStripMenuItem.CheckedChanged += EnableMovementToolStripMenuItem_CheckedChanged;
        }

        private void MainWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            //if (m_filePath != string.Empty)
            {
                if (MessageBox.Show("Save current file?", "Confirm", MessageBoxButtons.YesNo) == DialogResult.Yes)
                {
                    saveAsToolStripMenuItem_Click(sender, e);
                }
            }
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
            if (enableMovementToolStripMenuItem.Checked)
            {
                float frameTime = m_movementTimer.ElapsedMilliseconds / 1000f;
                m_movementTimer.Restart();

                Vector2f velocity = new Vector2f();
                velocity.X = (xTarget - m_particleSystem.position.X);
                velocity.Y = (yTarget - m_particleSystem.position.Y);
                m_particleSystem.position += (velocity * frameTime);

                if (Math.Abs(velocity.X) < 50f) xTarget = -xTarget;
                if (Math.Abs(velocity.Y) < 50f) yTarget = -yTarget;
            }

            //reset button label when finished
            if(!m_particleSystem.started)
            {
                buttonStart.Text = "Start";
            }
        }

        private void saveFile()
        {
            Debug.Assert(m_filePath != string.Empty);

            ParticleDefiniton pd = new ParticleDefiniton();
            pd.BlendMode = m_particleSystem.blendMode.ToString();
            pd.Colour = colourToInt(m_particleSystem.colour);
            pd.Delay = (float)numericUpDownStartDelay.Value;
            pd.Duration = (float)numericUpDownDuration.Value;
            pd.EmitRate = (float)numericUpDownEmitRate.Value;
            pd.InitialVelocity = new Point((int)m_particleSystem.initialVelocity.X, (int)m_particleSystem.initialVelocity.Y);
            pd.Lifetime = m_particleSystem.particleLifetime;
            //pd.ParticlePosition = new Point((int)numericUpDownInitVelX.Value, (int)numericUpDownInitVelY.Value);
            pd.ParticleSize = new Size((int)numericUpDownSizeX.Value, (int)numericUpDownSizeY.Value);
            pd.RandomInitialPositions = new List<Point>();
            foreach(var v in m_particleSystem.randomInitialPositions)
            {
                pd.RandomInitialPositions.Add(new Point((int)v.X, (int)v.Y));
            }
            pd.RandomInitialVelocites = new List<Point>();
            foreach (var v in m_particleSystem.randomInitialVelocities)
            {
                pd.RandomInitialVelocites.Add(new Point((int)v.X, (int)v.Y));
            }
            pd.ReleaseCount = (byte)numericUpDownReleaseCount.Value;
            pd.Texture = textBoxTexturePath.Text;

            try
            {
                JsonSerializer srlz = new JsonSerializer();
                srlz.NullValueHandling = NullValueHandling.Ignore;
                srlz.Formatting = Formatting.Indented;

                using (StreamWriter sw = new StreamWriter(m_filePath))
                using (JsonWriter jw = new JsonTextWriter(sw))
                {
                    srlz.Serialize(jw, pd);
                }
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "Output failed");
            }

        }
        private void loadFile()
        {
            Debug.Assert(m_filePath != string.Empty);

            try
            {
                ParticleDefiniton pd;
                JsonSerializer srlz = new JsonSerializer();
                srlz.NullValueHandling = NullValueHandling.Ignore;

                using (StreamReader sr = new StreamReader(m_filePath))
                using (JsonReader jr = new JsonTextReader(sr))
                {
                    pd = srlz.Deserialize<ParticleDefiniton>(jr);
                }

                switch(pd.BlendMode)
                {
                    case "Add":
                        m_particleSystem.blendMode = BlendMode.Add;
                        break;
                    case "Alpha":
                        m_particleSystem.blendMode = BlendMode.Alpha;
                        break;
                    case "Multiply":
                        m_particleSystem.blendMode = BlendMode.Multiply;
                        break;
                    case "None":
                        m_particleSystem.blendMode = BlendMode.None;
                        break;
                }
                comboBoxBlendMode.SelectedItem = m_particleSystem.blendMode;

                m_particleSystem.colour = intToColour(pd.Colour);
                panelColour.BackColor = System.Drawing.Color.FromArgb(m_particleSystem.colour.A, m_particleSystem.colour.R, m_particleSystem.colour.G, m_particleSystem.colour.B);

                numericUpDownStartDelay.Value = (Decimal)pd.Delay;
                numericUpDownDuration.Value = (Decimal)pd.Duration;
                numericUpDownEmitRate.Value = (Decimal)pd.EmitRate;
                numericUpDownInitVelX.Value = (Decimal)pd.InitialVelocity.X;
                numericUpDownInitVelY.Value = (Decimal)pd.InitialVelocity.Y;
                numericUpDownLifetime.Value = (Decimal)pd.Lifetime;
                //numericUpDownSpawnPosX.Value = (Decimal)pd.ParticlePosition.X;
                //numericUpDownSpawnPosY.Value = (Decimal)pd.ParticlePosition.Y;
                numericUpDownSizeX.Value = (Decimal)pd.ParticleSize.Width;
                numericUpDownSizeY.Value = (Decimal)pd.ParticleSize.Height;

                if (pd.RandomInitialPositions.Count > 0)
                {
                    List<Vector2f> randPositions = new List<Vector2f>();
                    foreach(var p in pd.RandomInitialPositions)
                    {
                        randPositions.Add(new Vector2f(p.X, p.Y));
                    }
                    m_particleSystem.randomInitialPositions = randPositions;
                    updateListbox(listBoxSpawnPoints.Items, randPositions);
                }

                if(pd.RandomInitialVelocites.Count > 0)
                {
                    List<Vector2f> randVelocities = new List<Vector2f>();
                    foreach(var v in pd.RandomInitialVelocites)
                    {
                        randVelocities.Add(new Vector2f(v.X, v.Y));
                    }
                    m_particleSystem.randomInitialVelocities = randVelocities;
                    updateListbox(listBoxSpawnVelocities.Items, randVelocities);
                }

                numericUpDownReleaseCount.Value = pd.ReleaseCount;
                if(!string.IsNullOrEmpty(pd.Texture))
                {
                    m_texture = new Texture(pd.Texture);
                    m_particleSystem.texture = m_texture;
                    textBoxTexturePath.Text = Path.GetFileName(pd.Texture);
                    panelTexPreview.BackgroundImage = new System.Drawing.Bitmap(pd.Texture);
                }
                else
                {
                    m_texture = null;
                    m_particleSystem.texture = null;
                    textBoxTexturePath.Text = string.Empty;
                    panelTexPreview.BackgroundImage = null;
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Failed to open particle file");
            }

        }

        private int colourToInt(SFML.Graphics.Color c)
        {
            return (c.R << 24) | (c.G << 16) | (c.B << 8) | 255;
        }

        private SFML.Graphics.Color intToColour(int c)
        {
            SFML.Graphics.Color retVal = new SFML.Graphics.Color();
            retVal.R = (byte)((c >> 24) & 0xFF);
            retVal.G = (byte)((c >> 16) & 0xFF);
            retVal.B = (byte)((c >> 8) & 0xFF);
            retVal.A = (byte)(c & 0xFF);

            return retVal;
        }

        private void updateListbox(ListBox.ObjectCollection dst, List<Vector2f> src)
        {
            dst.Clear();
            string str;
            foreach(var p in src)
            {
                str = p.X.ToString() + ", " + p.Y.ToString();
                dst.Add(str);
            }
        }
    }
}
