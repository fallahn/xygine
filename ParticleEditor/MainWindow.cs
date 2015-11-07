using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ParticleEditor
{
    public partial class MainWindow : Form
    {
        private SfmlControl m_sfmlControl = new SfmlControl();

        public MainWindow()
        {
            InitializeComponent();

            //add sfml control to layout
            splitContainer2.Panel1.Controls.Add(m_sfmlControl);
            m_sfmlControl.Dock = DockStyle.Fill;

            //TODO add draw delegates for drawing particles
        }

        /// <summary>
        /// Updates the sfml control periodically
        /// </summary>
        public void DispatchDrawingEvents()
        {
            m_sfmlControl.HandleEvents();
            m_sfmlControl.Draw();
        }
    }
}
