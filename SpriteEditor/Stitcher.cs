using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace SpriteEditor
{
    public partial class Stitcher : Form
    {
        private struct ListItem
        {
            public string Path { get; set; }
            public string Name { get; set; }
            public ListItem(string path, string name)
            {
                Path = path;
                Name = name;
            }
        }


        public Stitcher()
        {
            InitializeComponent();

            this.StartPosition = FormStartPosition.CenterParent;

            listBoxFiles.DisplayMember = "Name";
            listBoxFiles.ValueMember = "Path";
        }

        private void buttonAdd_Click(object sender, EventArgs e)
        {
            OpenFileDialog od = new OpenFileDialog();
            od.Filter = "png files|*.png|jpg files|*.jpg";
            od.Multiselect = true;
            if(od.ShowDialog() == DialogResult.OK)
            {
                foreach(var s in od.FileNames)
                {
                    listBoxFiles.Items.Add(new ListItem(s, Path.GetFileName(s)));
                }
            }
        }

        private void buttonRemove_Click(object sender, EventArgs e)
        {
            if(listBoxFiles.SelectedIndex > -1)
            {
                listBoxFiles.Items.RemoveAt(listBoxFiles.SelectedIndex);
            }
        }

        private void buttonClear_Click(object sender, EventArgs e)
        {
            listBoxFiles.Items.Clear();
        }

        private void buttonUp_Click(object sender, EventArgs e)
        {
            MoveItem(-1);
        }

        private void buttonDown_Click(object sender, EventArgs e)
        {
            MoveItem(1);
        }

        private void MoveItem(int direction)
        {
            if (listBoxFiles.SelectedItem == null || listBoxFiles.SelectedIndex < 0)
                return; //no selected item - nothing to do

            int newIndex = listBoxFiles.SelectedIndex + direction;

            if (newIndex < 0 || newIndex >= listBoxFiles.Items.Count)
                return; //index out of range - nothing to do

            object selected = listBoxFiles.SelectedItem;

            listBoxFiles.Items.Remove(selected);
            listBoxFiles.Items.Insert(newIndex, selected);
            listBoxFiles.SetSelected(newIndex, true);
        }

        private void buttonStitch_Click(object sender, EventArgs e)
        {
            if(listBoxFiles.Items.Count > 0)
            {
                SaveFileDialog sd = new SaveFileDialog();
                sd.Filter = "png files|*.png";

                if (sd.ShowDialog() == DialogResult.OK)
                {
                    //read all images into memory
                    List<Bitmap> images = new List<Bitmap>();
                    Bitmap finalImage = null;

                    try
                    {
                        int width = 0;
                        int tempWidth = 0;
                        int height = 0;

                        var files = listBoxFiles.Items;
                        decimal colCount = 0;
                        foreach(ListItem file in files)
                        {
                            Bitmap bitmap = new Bitmap(file.Path);

                            if(colCount == 0)
                            {
                                height += bitmap.Height;
                            }

                            tempWidth += bitmap.Width;
                            colCount++;

                            if(colCount == numericUpDownColCount.Value)
                            {
                                if (tempWidth > width) width = tempWidth;
                                tempWidth = 0;
                                colCount = 0;
                            }

                            images.Add(bitmap);
                        }

                        //create a bitmap to hold the combined image
                        finalImage = new Bitmap(width, height);

                        //get a graphics object from the image so we can draw on it
                        using (Graphics g = Graphics.FromImage(finalImage))
                        {
                            //set background color
                            g.Clear(Color.Transparent);

                            //go through each image and draw it on the final image
                            int offsetX = 0;
                            int offsetY = 0;
                            int tempY = 0;
                            colCount = 0;
                            foreach (Bitmap image in images)
                            {
                                g.DrawImage(image, new Rectangle(offsetX, offsetY, image.Width, image.Height));
                                offsetX += image.Width;
                                colCount++;

                                if(image.Height > tempY)
                                {
                                    tempY += image.Height;
                                }

                                if(colCount == numericUpDownColCount.Value)
                                {
                                    offsetX = 0;
                                    colCount = 0;

                                    offsetY += tempY;
                                    tempY = 0;
                                }
                            }
                        }

                        finalImage.Save(sd.FileName);
                        MessageBox.Show("Saved image successfully!");
                    }
                    catch (Exception)
                    {
                        if (finalImage != null)
                            finalImage.Dispose();
                        throw;
                    }
                    finally
                    {
                        //clean up memory
                        foreach (Bitmap image in images)
                        {
                            image.Dispose();
                        }
                    }
                }
            }
        }
    }
}
