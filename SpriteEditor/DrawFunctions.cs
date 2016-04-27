/*********************************************************************
Matt Marchant 2015 - 2016
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
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using SFML.Graphics;

namespace SpriteEditor
{
    public partial class MainWindow : Form
    {
        AnimatedSprite m_aniSprite = new AnimatedSprite();

        private void DrawSprite(RenderWindow rw)
        {
            rw.Draw(m_aniSprite);
        }
    }
}
