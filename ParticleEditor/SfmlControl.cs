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
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using SFML.Graphics;
using SFML.Window;

namespace ParticleEditor
{
    public delegate void DrawDelegate(RenderWindow rw);
    public delegate void UpdateDelegate(float dt);

    partial class SfmlControl : Control
    {
        private RenderWindow m_renderWindow;
        private Color m_clearColour = new Color();
        public Color BackgroundColour
        {
            get { return m_clearColour; }
            set { m_clearColour = value; }
        }

        //delegates for custom drawing
        private List<DrawDelegate> m_drawDelegates = new List<DrawDelegate>();
        public List<DrawDelegate> DrawDelegates
        {
            get { return m_drawDelegates; }
        }

        private List<UpdateDelegate> m_updateDelegates = new List<UpdateDelegate>();
        public List<UpdateDelegate> UpdateDelegates
        {
            get { return m_updateDelegates; }
        }

        public SfmlControl()
        {
            //dllNotFoundException? copy the csfml dlls to the output folder
            m_renderWindow = new RenderWindow(this.Handle);
            m_renderWindow.SetVerticalSyncEnabled(true);

            //add resize event handler
            this.Resize += SfmlControl_Resize;
            SfmlControl_Resize(this, EventArgs.Empty);
        }

        void SfmlControl_Resize(object sender, EventArgs e)
        {
            Vector2f size = new Vector2f(this.Size.Width, this.Size.Height);
            SFML.Graphics.View v = new SFML.Graphics.View(new Vector2f(), size);
            m_renderWindow.SetView(v);
        }

        Stopwatch clock = new Stopwatch();
        public void HandleEvents()
        {
            m_renderWindow.DispatchEvents();

            float dt = clock.Elapsed.Milliseconds / 1000f;
            foreach(UpdateDelegate u in m_updateDelegates)
            {
                u(dt);
            }
            clock.Restart();
        }

        public void Draw()
        {
            m_renderWindow.Clear(m_clearColour);
            foreach (DrawDelegate d in m_drawDelegates)
                d(m_renderWindow);
            m_renderWindow.Display();
        }

        //----control overrides-----//
        protected override void OnPaint(PaintEventArgs pe)
        {
            //base.OnPaint(pe);
        }

        protected override void OnPaintBackground(PaintEventArgs pevent)
        {
            base.OnPaintBackground(pevent);
        }
    }
}
