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
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using SFML.Window;
using SFML.Graphics;

namespace ParticleEditor
{
    public partial class MainWindow : Form
    {
        private void buttonStart_Click(object sender, EventArgs e)
        {
            if (m_particleSystem.started)
            {
                buttonStart.Text = "Start";
                m_particleSystem.stop();
            }
            else
            {
                buttonStart.Text = "Stop";
                m_particleSystem.start((byte)numericUpDownReleaseCount.Value, (float)numericUpDownStartDelay.Value, (float)numericUpDownDuration.Value);
            }
        }


        private void buttonSpawnPosAdd_Click(object sender, EventArgs e)
        {
            listBoxSpawnPoints.Items.Add(numericUpDownSpawnPosX.Value.ToString()+ ", " + numericUpDownSpawnPosY.Value.ToString());
            var list = parseList(listBoxSpawnPoints.Items);
            m_particleSystem.randomInitialPositions = (list.Count > 0) ? list : null;
        }

        private void buttonSpawnVelAdd_Click(object sender, EventArgs e)
        {
            listBoxSpawnVelocities.Items.Add(numericUpDownSpawnVelX.Value.ToString() + ", " + numericUpDownSpawnVelY.Value.ToString());
            var list = parseList(listBoxSpawnVelocities.Items);
            m_particleSystem.randomInitialVelocities = (list.Count > 0) ? list : null;
        }

        private char[] delim = {','};
        private List<Vector2f> parseList(ListBox.ObjectCollection items)
        {
            List<Vector2f> retVal = new List<Vector2f>();
            foreach(string s in items)
            {
                var pair = s.Split(delim);
                Vector2f v = new Vector2f(float.Parse(pair[0]), float.Parse(pair[1]));
                retVal.Add(v);
            }
            return retVal;
        }

        private void itemDelete_click(object sender, EventArgs e)
        {
            MenuItem item = sender as MenuItem;
            var menu = item.GetContextMenu();
            ListBox box = menu.SourceControl as ListBox;

            if (box.SelectedIndex > -1)
            {
                box.Items.RemoveAt(box.SelectedIndex);
            }

            var list = parseList(box.Items);
            if(box == listBoxSpawnPoints)
            {
                m_particleSystem.randomInitialPositions = (list.Count > 0) ? list : null;
            }
            else if(box == listBoxSpawnVelocities)
            {
                m_particleSystem.randomInitialVelocities = (list.Count > 0) ? list : null;
            }
        }

        private void itemsClear_click(object sender, EventArgs e)
        {
            MenuItem item = sender as MenuItem;
            var menu = item.GetContextMenu();
            ListBox box = menu.SourceControl as ListBox;

            box.Items.Clear();

            var list = parseList(box.Items);
            if (box == listBoxSpawnPoints)
            {
                m_particleSystem.randomInitialPositions = (list.Count > 0) ? list : null;
            }
            else if (box == listBoxSpawnVelocities)
            {
                m_particleSystem.randomInitialVelocities = (list.Count > 0) ? list : null;
            }
        }

        private Texture m_texture;
        private void buttonTextureBrowse_Click(object sender, EventArgs e)
        {
            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "JPEG files|*.jpg|Portable Network Graphic|*.png|Bitmap files|*.bmp";
            if(od.ShowDialog() == DialogResult.OK)
            {
                m_texture = new Texture(od.FileName);
                m_particleSystem.texture = m_texture;
                textBoxTexturePath.Text = Path.GetFileName(od.FileName);
            }
        }

        private void buttonTextureFit_Click(object sender, EventArgs e)
        {
            if(m_particleSystem.texture != null)
            {
                var texSize = m_particleSystem.texture.Size;
                numericUpDownSizeX.Value = Math.Min(texSize.X, numericUpDownSizeX.Maximum);
                numericUpDownSizeY.Value = Math.Min(texSize.Y, numericUpDownSizeY.Maximum);
            }
        }

        private void sizeChanged(object sender, EventArgs e)
        {
            Vector2f size = new Vector2f((float)numericUpDownSizeX.Value, (float)numericUpDownSizeY.Value);
            m_particleSystem.particleSize = size;
        }
    }
}
