using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ParticleEditor
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            //Application.Run(new MainWindow());

            MainWindow mw = new MainWindow();
            mw.Show();
            while (mw.Visible)
            {
                Application.DoEvents();
                mw.DispatchDrawingEvents();
            }
        }
    }
}
