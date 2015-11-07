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
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Newtonsoft.Json;

namespace SpriteEditor
{
    public partial class MainWindow : Form
    {
        private SfmlControl m_sfmlControl = new SfmlControl();
        private List<Animation> m_listboxData = new List<Animation>();
        private string m_currentPath;
        private bool m_modified = false;

        public MainWindow()
        {
            InitializeComponent();

            splitContainer2.Panel1.Controls.Add(m_sfmlControl);
            m_sfmlControl.Dock = DockStyle.Fill;
            m_sfmlControl.DrawDelegates.Add(this.DrawSprite);
        }

        /// <summary>
        /// Updates the sfml control periodically
        /// </summary>
        public void DispatchDrawingEvents()
        {
            m_sfmlControl.HandleEvents();
            m_sfmlControl.Draw();

            if (!m_aniSprite.Playing)
                buttonPlayPause.Text = "Play";
        }


        //----event handlers----//
        private void openImageToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConfirmClose();

            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "PNG Files|*.png|JPG Files|*.jpg";

            if(od.ShowDialog() == DialogResult.OK)
            {
                LoadTexture(od.FileName);
            }
        }
        
        private void openJSONToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConfirmClose();

            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "Animation Files|*.cra";
            if (od.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    AnimationData data;
                    JsonSerializer js = new JsonSerializer();
                    js.NullValueHandling = NullValueHandling.Ignore;

                    using (StreamReader sr = new StreamReader(od.FileName))
                    using (JsonReader jr = new JsonTextReader(sr))
                    {
                        data = js.Deserialize<AnimationData>(jr);
                    }

                    var path = Path.GetDirectoryName(od.FileName) + "\\";
                    LoadTexture(path + data.Texture);

                    numericUpDownFrameWidth.Value = (Decimal)data.FrameSize.Width;
                    numericUpDownFrameHeight.Value = (Decimal)data.FrameSize.Height;
                    numericUpDownFrameRate.Value = (Decimal)data.FrameRate;

                    textBoxNormalMap.Text = data.NormalMap;

                    m_listboxData = data.Animations;
                    listBoxAnimations.DataSource = null;
                    listBoxAnimations.DataSource = m_listboxData;
                    listBoxAnimations.DisplayMember = "Name";

                    m_aniSprite.FrameCount = data.FrameCount;
                    m_aniSprite.FrameRate = data.FrameRate;
                    m_aniSprite.FrameSize = new SFML.Window.Vector2i(data.FrameSize.Width, data.FrameSize.Height);

                    m_currentPath = od.FileName;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Loading JSON");
                }
            }
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(m_currentPath == string.Empty || m_currentPath == null)
            {
                saveAsToolStripMenuItem_Click(this, EventArgs.Empty);
            }
            else
            {
                SaveData();
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sd = new SaveFileDialog();
            sd.Filter = "Animation Files|*.cra";
            if(sd.ShowDialog() == DialogResult.OK)
            {
                m_currentPath = sd.FileName;
                SaveData();
            }
        }

        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConfirmClose();
            this.Close();
        }

        private void backgroundColourToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var currentColour = m_sfmlControl.BackgroundColour;
            
            ColorDialog cd = new ColorDialog();
            cd.Color = Color.FromArgb(currentColour.R, currentColour.G, currentColour.B);
            cd.FullOpen = true;
            if(cd.ShowDialog() == DialogResult.OK)
            {
                currentColour.R = cd.Color.R;
                currentColour.G = cd.Color.G;
                currentColour.B = cd.Color.B;
                m_sfmlControl.BackgroundColour = currentColour;
            }
        }

        private void numericUpDownFrameWidth_ValueChanged(object sender, EventArgs e)
        {
            m_aniSprite.FrameSize = new SFML.Window.Vector2i((int)numericUpDownFrameWidth.Value, (int)numericUpDownFrameHeight.Value);
            CalcFrameCount();
        }

        private void numericUpDownFrameHeight_ValueChanged(object sender, EventArgs e)
        {
            m_aniSprite.FrameSize = new SFML.Window.Vector2i((int)numericUpDownFrameWidth.Value, (int)numericUpDownFrameHeight.Value);
            CalcFrameCount();
        }
        
        private void numericUpDownFrameCount_ValueChanged(object sender, EventArgs e)
        {
            m_aniSprite.FrameCount = (uint)numericUpDownFrameCount.Value;
            m_modified = true;
        }

        private void numericUpDownFrameRate_ValueChanged(object sender, EventArgs e)
        {
            m_aniSprite.FrameRate = (float)numericUpDownFrameRate.Value;
            m_modified = true;
        }

        private void buttonPlayPause_Click(object sender, EventArgs e)
        {
            if (m_aniSprite.Playing)
            {
                m_aniSprite.SetPaused(true);
                buttonPlayPause.Text = "Play";
                labelCurrentFrame.Text = "Current Frame: " + m_aniSprite.CurrentFrame.ToString();
            }
            else
            {
                if (listBoxAnimations.SelectedIndex < 0)
                {
                    m_aniSprite.Loop = true;
                    m_aniSprite.Play(0, m_aniSprite.FrameCount - 1);
                }
                else
                {
                    m_aniSprite.Play(m_listboxData[listBoxAnimations.SelectedIndex]);
                }
                buttonPlayPause.Text = "Pause";
            }
        }

        private void buttonPrev_Click(object sender, EventArgs e)
        {
            m_aniSprite.FrameBack();
            labelCurrentFrame.Text = "Current Frame: " + m_aniSprite.CurrentFrame.ToString();
        }

        private void buttonNext_Click(object sender, EventArgs e)
        {
            m_aniSprite.FrameForward();
            labelCurrentFrame.Text = "Current Frame: " + m_aniSprite.CurrentFrame.ToString();
        }
        
        private void buttonDivTwo_Click(object sender, EventArgs e)
        {
            if(radioButtonWidth.Checked && numericUpDownFrameWidth.Value > 4)
            {
                numericUpDownFrameWidth.Value /= 2;
            }
            else if(radioButtonHeight.Checked && numericUpDownFrameHeight.Value > 4)
            {
                numericUpDownFrameHeight.Value /= 2;
            }
        }

        private void buttonDivFour_Click(object sender, EventArgs e)
        {
            if (radioButtonWidth.Checked && numericUpDownFrameWidth.Value > 8)
            {
                numericUpDownFrameWidth.Value /= 4;
            }
            else if (radioButtonHeight.Checked && numericUpDownFrameHeight.Value > 8)
            {
                numericUpDownFrameHeight.Value /= 4;
            }
        }

        private void buttonBrowseNormalmap_Click(object sender, EventArgs e)
        {
            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "PNG Files|*.png|JPG Files|*.jpg|Targa Files|*.tga";
            if(od.ShowDialog() == DialogResult.OK)
            {
                textBoxNormalMap.Text = Path.GetFileName(od.FileName);
                m_modified = true;
            }
        }

        //animtion data
        private void numericUpDownAnimStart_ValueChanged(object sender, EventArgs e)
        {
            var maxValue = numericUpDownFrameCount.Value - 1;
            if(numericUpDownAnimStart.Value > maxValue)
            {
                numericUpDownAnimStart.Value = maxValue;
            }

            if (numericUpDownAnimStart.Value > numericUpDownAnimEnd.Value)
            {
                numericUpDownAnimStart.Value = numericUpDownAnimEnd.Value;
            }
        }

        private void numericUpDownAnimEnd_ValueChanged(object sender, EventArgs e)
        {
            Decimal maxIndex = numericUpDownFrameCount.Value - 1;
            if (numericUpDownAnimEnd.Value > maxIndex)
            {
                numericUpDownAnimEnd.Value = maxIndex;
            }

            if(numericUpDownAnimEnd.Value < numericUpDownAnimStart.Value)
            {
                numericUpDownAnimEnd.Value = numericUpDownAnimStart.Value;
            }
        }

        private void buttonAddAnim_Click(object sender, EventArgs e)
        {
            if (textBoxAnimName.Text != string.Empty)
            { 
                //check name doesn't exist
                if (m_listboxData.Exists(new Predicate<Animation>(x => x.Name == textBoxAnimName.Text))) return;
                
                Animation anim = new Animation(textBoxAnimName.Text.ToLower(), (uint)numericUpDownAnimStart.Value, (uint)numericUpDownAnimEnd.Value, checkBoxLoop.Checked);
                m_listboxData.Add(anim);
                listBoxAnimations.DataSource = null;
                listBoxAnimations.DataSource = m_listboxData;
                listBoxAnimations.DisplayMember = "Name";

                m_modified = true;
            }
        }

        private void buttonRemoveAnim_Click(object sender, EventArgs e)
        {
            if(listBoxAnimations.SelectedIndex >= 0)
            {
                m_listboxData.RemoveAt(listBoxAnimations.SelectedIndex);
                listBoxAnimations.DataSource = null;
                listBoxAnimations.DataSource = m_listboxData;
                listBoxAnimations.DisplayMember = "Name";

                m_modified = true;
            }
        }

        private void listBoxAnimations_SelectedIndexChanged(object sender, EventArgs e)
        {
            var i = listBoxAnimations.SelectedIndex;
            if(i > -1)
            {
                numericUpDownAnimStart.Value = (Decimal)m_listboxData[i].Start;
                numericUpDownAnimEnd.Value = (Decimal)m_listboxData[i].End;
                checkBoxLoop.Checked = m_listboxData[i].Loop;
                textBoxAnimName.Text = m_listboxData[i].Name;

                if(m_aniSprite.Playing)
                {
                    m_aniSprite.Play(m_listboxData[i]);
                }
            }
        }

        //----misc----//
        private void ResetControls()
        {
            numericUpDownAnimEnd.Value = 0;
            numericUpDownAnimStart.Value = 0;
            numericUpDownFrameCount.Value = 1;
            numericUpDownFrameHeight.Value = (Decimal)m_aniSprite.Texture.Size.Y;
            numericUpDownFrameRate.Value = 12;
            numericUpDownFrameWidth.Value = (Decimal)m_aniSprite.Texture.Size.X;

            labelCurrentFrame.Text = "Current Frame:";
            m_currentPath = string.Empty;
            m_listboxData.Clear();
            listBoxAnimations.DataSource = null;
            listBoxAnimations.DataSource = m_listboxData;
            listBoxAnimations.DisplayMember = "Name";

            m_modified = false;
        }

        private void CalcFrameCount()
        {
            if (m_aniSprite.Texture != null)
            {
                var frameX = (uint)(m_aniSprite.Texture.Size.X / m_aniSprite.FrameSize.X);
                var frameY = (uint)(m_aniSprite.Texture.Size.Y / m_aniSprite.FrameSize.Y);

                var value = frameX * frameY;
                numericUpDownFrameCount.Value = (Decimal)value;

                //go through existing animations and make sure ranges are clamped within frame count
                for(var i = 0; i < m_listboxData.Count; ++i)
                {
                    if(m_listboxData[i].End >= value)
                    {
                        var anim = m_listboxData[i];
                        anim.End = value - 1;
                        m_listboxData[i] = anim;
                    }

                    if(m_listboxData[i].Start >= value ||
                        m_listboxData[i].Start > m_listboxData[i].End)
                    {
                        var anim = m_listboxData[i];
                        anim.Start = value - 1;
                        m_listboxData[i] = anim;
                    }
                }

                m_modified = true;
            }
        }

        private void SaveData()
        {
            Debug.Assert(m_currentPath != null && m_currentPath != string.Empty);

            string normalMap = (textBoxNormalMap.Text == string.Empty || textBoxNormalMap.Text == null) ? "none.png" : textBoxNormalMap.Text;

            AnimationData data = new AnimationData(m_aniSprite.FrameCount,
                new Size(m_aniSprite.FrameSize.X, m_aniSprite.FrameSize.Y),
                m_aniSprite.FrameRate, this.Text, normalMap);

            //create a default animation
            if(m_listboxData.Count < 1)
            {
                m_listboxData.Insert(0, new Animation("default", 0, m_aniSprite.FrameCount - 1));
            }

            data.Animations = m_listboxData;

            try
            {
                //save json file
                JsonSerializer srlz = new JsonSerializer();
                srlz.NullValueHandling = NullValueHandling.Ignore;
                srlz.Formatting = Formatting.Indented;

                using (StreamWriter sw = new StreamWriter(m_currentPath))
                using (JsonWriter jw = new JsonTextWriter(sw))
                {
                    srlz.Serialize(jw, data);
                }

                m_modified = false;
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message, "Saving JSON Data");
            }
        }

        private void LoadTexture(string path)
        {
            SFML.Graphics.Texture tex = new SFML.Graphics.Texture(path);
            if (tex.Size.X <= 2048 && tex.Size.Y <= 2048)
            {
                m_aniSprite = new AnimatedSprite(tex);
                m_sfmlControl.UpdateDelegates.Clear();
                m_sfmlControl.UpdateDelegates.Add(m_aniSprite.Update);
                ResetControls();

                m_aniSprite.FrameRate = (uint)numericUpDownFrameRate.Value;
                m_aniSprite.FrameCount = (uint)numericUpDownFrameCount.Value;
                m_aniSprite.FrameSize = new SFML.Window.Vector2i((int)numericUpDownFrameWidth.Value, (int)numericUpDownFrameHeight.Value);

                this.Text = Path.GetFileName(path);

                m_modified = true;
            }
            else
            {
                MessageBox.Show("Maximum texture size is 2048 x 2048", "Texture Too Big");
            }
        }

        private void ConfirmClose()
        {
            if (m_modified && MessageBox.Show("Save Unsaved changes?", "Confirm", MessageBoxButtons.YesNo) == DialogResult.Yes)
            {
                saveAsToolStripMenuItem_Click(this, EventArgs.Empty);
            }
        }
    }
}
