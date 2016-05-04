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
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;

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
            m_sfmlControl.MouseClick += sfmlMouse_Click;
            m_sfmlControl.DrawDelegates.Add(drawParticleSystem);
            m_sfmlControl.UpdateDelegates.Add(m_particleSystem.update);

            comboBoxBlendMode.DataSource = Enum.GetValues(typeof(BlendMode));
            comboBoxBlendMode.SelectedItem = BlendMode.Add;

            comboBoxAffectors.DataSource = Enum.GetValues(typeof(AffectorType));
            comboBoxAffectors.SelectedItem = AffectorType.Force;

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

            listBoxAffectors.ContextMenu = new ContextMenu();
            listBoxAffectors.ContextMenu.MenuItems.Add("Delete");
            listBoxAffectors.ContextMenu.MenuItems[0].Click += itemDelete_click;
            listBoxAffectors.ContextMenu.MenuItems.Add("Clear");
            listBoxAffectors.ContextMenu.MenuItems[1].Click += itemsClear_click;

            numericUpDownSizeX.ValueChanged += sizeChanged;
            numericUpDownSizeY.ValueChanged += sizeChanged;

            numericUpDownInitVelX.ValueChanged += velocityChanged;
            numericUpDownInitVelY.ValueChanged += velocityChanged;

            numericUpDownEmitRate.ValueChanged += NumericUpDownEmitRate_ValueChanged;
            numericUpDownLifetime.ValueChanged += NumericUpDownLifetime_ValueChanged;

            numericUpDownVelAffectorX.ValueChanged += NumericUpDownVelAffector_ValueChanged;
            numericUpDownVelAffectorY.ValueChanged += NumericUpDownVelAffector_ValueChanged;

            panelColour.Click += colour_Click;
            panelColour.BackColorChanged += PanelColour_BackColorChanged;

            enableMovementToolStripMenuItem.CheckedChanged += EnableMovementToolStripMenuItem_CheckedChanged;

            loadAssetPaths();
        }

        private List<string> m_AssetPaths = new List<string>();
        private void loadAssetPaths()
        {
            try
            {
                StreamReader sr = new StreamReader("settings.set");
                string line;
                while((line = sr.ReadLine()) != null)
                {
                    if(Directory.Exists(line)) m_AssetPaths.Add(line);
                }
                sr.Close();
            }
            catch
            {
                m_AssetPaths.Add(Directory.GetCurrentDirectory()); //just add current working dir
            }
        }
        private void saveAssetPaths()
        {
            try
            {
                StreamWriter sw = new StreamWriter("settings.set");
                foreach (string str in m_AssetPaths)
                {
                    sw.WriteLine(str);
                }
                sw.Close();
            }
            catch(Exception e)
            {
                MessageBox.Show(e.Message, "Failed to write settings file");
            }
        }

        private void sfmlMouse_Click(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_particleSystem.position = m_sfmlControl.pixelToCoords(e.Location);
                m_defaultPos = m_particleSystem.position;
            }
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
            saveAssetPaths();
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
            if (m_particleSystem.started && enableMovementToolStripMenuItem.Checked)
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
                m_movementTimer.Restart();
                m_particleSystem.position = m_defaultPos;
            }
        }

        private void saveFile()
        {
            Debug.Assert(m_filePath != string.Empty);

            ParticleDefiniton pd = new ParticleDefiniton();
            pd.Editor = new EditorSettings();
            pd.Editor.BackgroundColour = colourToInt(m_sfmlControl.BackgroundColour);
            pd.Editor.EmitterPosition = new Point((int)m_defaultPos.X, (int)m_defaultPos.Y);
            pd.Editor.EnableMovement = enableMovementToolStripMenuItem.Checked;

            pd.BlendMode = m_particleSystem.blendMode.ToString();
            pd.Colour = colourToInt(m_particleSystem.colour);
            pd.Delay = (float)numericUpDownStartDelay.Value;
            pd.Duration = (float)numericUpDownDuration.Value;
            pd.EmitRate = (float)numericUpDownEmitRate.Value;
            pd.InitialVelocity = new Point((int)m_particleSystem.initialVelocity.X, (int)m_particleSystem.initialVelocity.Y);
            pd.Lifetime = m_particleSystem.particleLifetime;
            pd.ParticleSize = new Size((int)numericUpDownSizeX.Value, (int)numericUpDownSizeY.Value);
            pd.RandomInitialPositions = new List<Point>();
            if (m_particleSystem.randomInitialPositions != null)
            {
                foreach (var v in m_particleSystem.randomInitialPositions)
                {
                    pd.RandomInitialPositions.Add(new Point((int)v.X, (int)v.Y));
                }
            }

            pd.RandomInitialVelocities = new List<Point>();
            if (m_particleSystem.randomInitialVelocities != null)
            {
                foreach (var v in m_particleSystem.randomInitialVelocities)
                {
                    pd.RandomInitialVelocities.Add(new Point((int)v.X, (int)v.Y));
                }
            }
            pd.Affectors = new List<AffectorDefinition>();
            foreach(var a in m_particleSystem.Affectors)
            {
                AffectorDefinition ad = new AffectorDefinition();
                ad.Type = a.type().ToString();
                ad.Data = new List<float>();
                switch (a.type())
                {
                    case AffectorType.Colour:
                        ColourAffector ca = (ColourAffector)a;
                        ad.Data.Add(colourToInt(ca.StartColour));
                        ad.Data.Add(colourToInt(ca.EndColour));
                        ad.Data.Add(ca.Duration);
                        break;
                    case AffectorType.Force:
                        var force = ((ForceAffector)a).Force;
                        ad.Data.Add(force.X);
                        ad.Data.Add(force.Y);
                        break;
                    case AffectorType.Rotation:
                        ad.Data.Add(((RotationAffector)a).Rotation);
                        break;
                    case AffectorType.Scale:
                        var scale = ((ScaleAffector)a).Scale;
                        ad.Data.Add(scale.X);
                        ad.Data.Add(scale.Y);
                        break;
                    case AffectorType.Velocity:
                        var vscale = ((VelocityAffector)a).Scale;
                        ad.Data.Add(vscale.X);
                        ad.Data.Add(vscale.Y);
                        break;
                }
                pd.Affectors.Add(ad);
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

                m_sfmlControl.BackgroundColour = intToColour(pd.Editor.BackgroundColour);
                enableMovementToolStripMenuItem.Checked = pd.Editor.EnableMovement;
                m_defaultPos = new Vector2f(pd.Editor.EmitterPosition.X, pd.Editor.EmitterPosition.Y);
                m_particleSystem.position = m_defaultPos;

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

                numericUpDownStartDelay.Value = (decimal)pd.Delay;
                numericUpDownDuration.Value = (decimal)pd.Duration;
                numericUpDownEmitRate.Value = (decimal)pd.EmitRate;
                numericUpDownInitVelX.Value = pd.InitialVelocity.X;
                numericUpDownInitVelY.Value = pd.InitialVelocity.Y;
                numericUpDownLifetime.Value = (decimal)pd.Lifetime;
                numericUpDownSizeX.Value = pd.ParticleSize.Width;
                numericUpDownSizeY.Value = pd.ParticleSize.Height;

                m_particleSystem.randomInitialPositions = null;
                listBoxSpawnPoints.Items.Clear();
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

                m_particleSystem.randomInitialVelocities = null;
                listBoxSpawnVelocities.Items.Clear();
                if(pd.RandomInitialVelocities.Count > 0)
                {
                    List<Vector2f> randVelocities = new List<Vector2f>();
                    foreach(var v in pd.RandomInitialVelocities)
                    {
                        randVelocities.Add(new Vector2f(v.X, v.Y));
                    }
                    m_particleSystem.randomInitialVelocities = randVelocities;
                    updateListbox(listBoxSpawnVelocities.Items, randVelocities);
                }

                listBoxAffectors.Items.Clear();
                m_particleSystem.Affectors.Clear();
                if(pd.Affectors.Count > 0)
                {
                    foreach(var ad in pd.Affectors)
                    {
                        if (ad.Type == AffectorType.Colour.ToString())
                        {
                            SFML.Graphics.Color start = intToColour((int)ad.Data[0]);
                            SFML.Graphics.Color end = intToColour((int)ad.Data[1]);
                            ColourAffector ca = new ColourAffector(start, end, ad.Data[2]);
                            m_particleSystem.Affectors.Add(ca);
                            m_particleSystem.colour = start;
                        }
                        else if (ad.Type == AffectorType.Force.ToString())
                        {
                            ForceAffector fa = new ForceAffector(new Vector2f(ad.Data[0], ad.Data[1]));
                            m_particleSystem.Affectors.Add(fa);
                        }
                        else if (ad.Type == AffectorType.Rotation.ToString())
                        {
                            RotationAffector ra = new RotationAffector(ad.Data[0]);
                            m_particleSystem.Affectors.Add(ra);
                        }
                        else if (ad.Type == AffectorType.Scale.ToString())
                        {
                            ScaleAffector sa = new ScaleAffector(new Vector2f(ad.Data[0], ad.Data[1]));
                            m_particleSystem.Affectors.Add(sa);
                        }
                        else if(ad.Type == AffectorType.Velocity.ToString())
                        {
                            VelocityAffector va = new VelocityAffector(new Vector2f(ad.Data[0], ad.Data[1]));
                            m_particleSystem.addAffector(va);
                        }
                        listBoxAffectors.Items.Add(ad.Type);
                    }
                }

                numericUpDownReleaseCount.Value = pd.ReleaseCount;
                if(!string.IsNullOrEmpty(pd.Texture))
                {
                    string path = pd.Texture;
                    if (!File.Exists(path))
                    {
                        //try reconstructing from known asset paths
                        path = path.Replace('/', '\\');
                        bool loaded = false;
                        foreach (string str in m_AssetPaths)
                        {
                            string temp = str + "\\" + path;
                            if (File.Exists(temp))
                            {
                                m_texture = new Texture(temp);
                                m_particleSystem.texture = m_texture;
                                textBoxTexturePath.Text = pd.Texture;
                                panelTexPreview.BackgroundImage = new Bitmap(temp);
                                loaded = true;
                                break;
                            }
                        }
                        if (!loaded)
                        {
                            //last ditch, try working dir
                            string temp = Path.GetFileName(pd.Texture);
                            if (File.Exists(temp))
                            {
                                m_texture = new Texture(temp);
                                m_particleSystem.texture = m_texture;
                                textBoxTexturePath.Text = pd.Texture;
                                panelTexPreview.BackgroundImage = new Bitmap(temp);
                            }
                        }
                    }
                    else
                    {
                        m_texture = new Texture(pd.Texture);
                        m_particleSystem.texture = m_texture;
                        textBoxTexturePath.Text = pd.Texture;
                        panelTexPreview.BackgroundImage = new Bitmap(pd.Texture);
                    }
                    fitPreviewImage();
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
                MessageBox.Show("make sure asset paths are valid\n(Options->Add Asset Folder)");
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

        private void fitPreviewImage()
        {
            var previewImg = panelTexPreview.BackgroundImage;
            if (panelTexPreview.Width < previewImg.Width ||
                panelTexPreview.Height < previewImg.Height)
            {
                //resize
                if (previewImg.Width > previewImg.Height)
                {
                    float ratio = (float)previewImg.Height / previewImg.Width;
                    previewImg = resizeImage(previewImg, panelTexPreview.Width, (int)(panelTexPreview.Width * ratio));
                }
                else
                {
                    float ratio = (float)previewImg.Width / previewImg.Height;
                    previewImg = resizeImage(previewImg, (int)(panelTexPreview.Height * ratio), panelTexPreview.Height);
                }
            }
            panelTexPreview.BackgroundImage = previewImg;
        }

        private Bitmap resizeImage(System.Drawing.Image image, int width, int height)
        {
            var destRect = new Rectangle(0, 0, width, height);
            var destImage = new Bitmap(width, height);

            destImage.SetResolution(image.HorizontalResolution, image.VerticalResolution);

            using (var graphics = Graphics.FromImage(destImage))
            {
                graphics.CompositingMode = CompositingMode.SourceCopy;
                graphics.CompositingQuality = CompositingQuality.HighQuality;
                graphics.InterpolationMode = InterpolationMode.HighQualityBicubic;
                graphics.SmoothingMode = SmoothingMode.HighQuality;
                graphics.PixelOffsetMode = PixelOffsetMode.HighQuality;

                using (var wrapMode = new ImageAttributes())
                {
                    wrapMode.SetWrapMode(WrapMode.TileFlipXY);
                    graphics.DrawImage(image, destRect, 0, 0, image.Width, image.Height, GraphicsUnit.Pixel, wrapMode);
                }
            }

            return destImage;
        }

    }
}
