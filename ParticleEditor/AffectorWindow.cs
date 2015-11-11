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
using System.Drawing;
using System.Windows.Forms;

namespace ParticleEditor
{
    public partial class AffectorWindow : Form
    {
        public IAffector Affector { get; set; }

        private AffectorType m_type;
        private Point m_panelLocation = new Point(13, 13);

        public AffectorWindow(AffectorType type)
        {
            InitializeComponent();

            switch (type)
            {
                case AffectorType.Colour:
                    this.Text = "Add Colour Affector";
                    panelColour.Visible = true;
                    panelColour.Location = m_panelLocation;
                    break;
                case AffectorType.Force:
                    this.Text = "Add Force Affector";
                    panelForce.Visible = true;
                    break;
                case AffectorType.Rotation:
                    this.Text = "Add Rotation Affector";
                    panelRotation.Visible = true;
                    panelRotation.Location = m_panelLocation;
                    break;
                case AffectorType.Scale:
                    this.Text = "Add Scale Affector";
                    panelScale.Visible = true;
                    panelScale.Location = m_panelLocation;
                    break;
            }
            m_type = type;
            this.Size = new Size(300, 140);
            this.MinimumSize = this.Size;
            this.MaximumSize = this.Size;
        }

        private void AffectorWindow_FormClosing(object sender, FormClosingEventArgs e)
        {
            switch (m_type)
            {
                case AffectorType.Colour:
                    SFML.Graphics.Color start = new SFML.Graphics.Color();
                    start.R = panelColourStart.BackColor.R;
                    start.G = panelColourStart.BackColor.G;
                    start.B = panelColourStart.BackColor.B;

                    SFML.Graphics.Color end = new SFML.Graphics.Color();
                    end.R = panelColourEnd.BackColor.R;
                    end.G = panelColourEnd.BackColor.G;
                    end.B = panelColourEnd.BackColor.B;

                    Affector = new ColourAffector(start, end, (float)numericUpDownColour.Value);
                    break;
                case AffectorType.Force:
                    Affector = new ForceAffector(new SFML.Window.Vector2f((float)numericUpDownForceX.Value, (float)numericUpDownForceY.Value));
                    break;
                case AffectorType.Rotation:
                    Affector = new RotationAffector((float)numericUpDownRotation.Value);
                    break;
                case AffectorType.Scale:
                    Affector = new ScaleAffector(new SFML.Window.Vector2f((float)numericUpDownScaleX.Value, (float)numericUpDownScaleY.Value));
                    break;
            }
        }

        private void panelColourStart_Click(object sender, EventArgs e)
        {
            panelColourStart.BackColor = getColour();
        }

        private void panelColourEnd_Click(object sender, EventArgs e)
        {
            panelColourEnd.BackColor = getColour();
        }

        private System.Drawing.Color getColour()
        {
            ColorDialog cd = new ColorDialog();
            cd.AllowFullOpen = true;
            cd.AnyColor = true;
            cd.FullOpen = true;

            cd.ShowDialog();

            var colour = cd.Color;
            return colour;
        }
    }
}
