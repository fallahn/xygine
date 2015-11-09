namespace ParticleEditor
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.label14 = new System.Windows.Forms.Label();
            this.buttonTextureFit = new System.Windows.Forms.Button();
            this.label12 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.numericUpDownSizeY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSizeX = new System.Windows.Forms.NumericUpDown();
            this.label11 = new System.Windows.Forms.Label();
            this.buttonTextureBrowse = new System.Windows.Forms.Button();
            this.textBoxTexturePath = new System.Windows.Forms.TextBox();
            this.buttonStart = new System.Windows.Forms.Button();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.numericUpDownDuration = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownStartDelay = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownReleaseCount = new System.Windows.Forms.NumericUpDown();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.numericUpDownSpawnPosY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSpawnPosX = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.numericUpDownSpawnVelY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownSpawnVelX = new System.Windows.Forms.NumericUpDown();
            this.buttonSpawnVelAdd = new System.Windows.Forms.Button();
            this.listBoxSpawnVelocities = new System.Windows.Forms.ListBox();
            this.label3 = new System.Windows.Forms.Label();
            this.buttonSpawnPosAdd = new System.Windows.Forms.Button();
            this.listBoxSpawnPoints = new System.Windows.Forms.ListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSizeY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSizeX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDuration)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownStartDelay)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownReleaseCount)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosX)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnVelY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnVelX)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.IsSplitterFixed = true;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.splitContainer2);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.label9);
            this.splitContainer1.Panel2.Controls.Add(this.label10);
            this.splitContainer1.Panel2.Controls.Add(this.numericUpDownSpawnPosY);
            this.splitContainer1.Panel2.Controls.Add(this.numericUpDownSpawnPosX);
            this.splitContainer1.Panel2.Controls.Add(this.label8);
            this.splitContainer1.Panel2.Controls.Add(this.label7);
            this.splitContainer1.Panel2.Controls.Add(this.numericUpDownSpawnVelY);
            this.splitContainer1.Panel2.Controls.Add(this.numericUpDownSpawnVelX);
            this.splitContainer1.Panel2.Controls.Add(this.buttonSpawnVelAdd);
            this.splitContainer1.Panel2.Controls.Add(this.listBoxSpawnVelocities);
            this.splitContainer1.Panel2.Controls.Add(this.label3);
            this.splitContainer1.Panel2.Controls.Add(this.buttonSpawnPosAdd);
            this.splitContainer1.Panel2.Controls.Add(this.listBoxSpawnPoints);
            this.splitContainer1.Panel2.Controls.Add(this.label2);
            this.splitContainer1.Panel2.Controls.Add(this.button1);
            this.splitContainer1.Panel2.Controls.Add(this.listBox1);
            this.splitContainer1.Panel2.Controls.Add(this.label1);
            this.splitContainer1.Size = new System.Drawing.Size(794, 549);
            this.splitContainer1.SplitterDistance = 543;
            this.splitContainer1.TabIndex = 0;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.IsSplitterFixed = true;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.menuStrip1);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.label14);
            this.splitContainer2.Panel2.Controls.Add(this.buttonTextureFit);
            this.splitContainer2.Panel2.Controls.Add(this.label12);
            this.splitContainer2.Panel2.Controls.Add(this.label13);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownSizeY);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownSizeX);
            this.splitContainer2.Panel2.Controls.Add(this.label11);
            this.splitContainer2.Panel2.Controls.Add(this.buttonTextureBrowse);
            this.splitContainer2.Panel2.Controls.Add(this.textBoxTexturePath);
            this.splitContainer2.Panel2.Controls.Add(this.buttonStart);
            this.splitContainer2.Panel2.Controls.Add(this.label6);
            this.splitContainer2.Panel2.Controls.Add(this.label5);
            this.splitContainer2.Panel2.Controls.Add(this.label4);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownDuration);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownStartDelay);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownReleaseCount);
            this.splitContainer2.Size = new System.Drawing.Size(543, 549);
            this.splitContainer2.SplitterDistance = 302;
            this.splitContainer2.TabIndex = 0;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(543, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(125, 22);
            this.openToolStripMenuItem.Text = "Open";
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(125, 22);
            this.saveToolStripMenuItem.Text = "Save";
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(125, 22);
            this.saveAsToolStripMenuItem.Text = "Save As...";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(280, 188);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(65, 13);
            this.label14.TabIndex = 18;
            this.label14.Text = "Particle Size";
            // 
            // buttonTextureFit
            // 
            this.buttonTextureFit.Location = new System.Drawing.Point(426, 210);
            this.buttonTextureFit.Name = "buttonTextureFit";
            this.buttonTextureFit.Size = new System.Drawing.Size(105, 23);
            this.buttonTextureFit.TabIndex = 17;
            this.buttonTextureFit.Text = "Fit to Texture";
            this.buttonTextureFit.UseVisualStyleBackColor = true;
            this.buttonTextureFit.Click += new System.EventHandler(this.buttonTextureFit_Click);
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(354, 215);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(14, 13);
            this.label12.TabIndex = 16;
            this.label12.Text = "Y";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(277, 215);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(14, 13);
            this.label13.TabIndex = 15;
            this.label13.Text = "X";
            // 
            // numericUpDownSizeY
            // 
            this.numericUpDownSizeY.Location = new System.Drawing.Point(374, 213);
            this.numericUpDownSizeY.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownSizeY.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDownSizeY.Name = "numericUpDownSizeY";
            this.numericUpDownSizeY.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSizeY.TabIndex = 14;
            this.numericUpDownSizeY.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            // 
            // numericUpDownSizeX
            // 
            this.numericUpDownSizeX.Location = new System.Drawing.Point(297, 213);
            this.numericUpDownSizeX.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownSizeX.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.numericUpDownSizeX.Name = "numericUpDownSizeX";
            this.numericUpDownSizeX.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSizeX.TabIndex = 13;
            this.numericUpDownSizeX.Value = new decimal(new int[] {
            2,
            0,
            0,
            0});
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(9, 188);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(46, 13);
            this.label11.TabIndex = 9;
            this.label11.Text = "Texture:";
            // 
            // buttonTextureBrowse
            // 
            this.buttonTextureBrowse.Location = new System.Drawing.Point(236, 210);
            this.buttonTextureBrowse.Name = "buttonTextureBrowse";
            this.buttonTextureBrowse.Size = new System.Drawing.Size(26, 23);
            this.buttonTextureBrowse.TabIndex = 8;
            this.buttonTextureBrowse.Text = "...";
            this.buttonTextureBrowse.UseVisualStyleBackColor = true;
            this.buttonTextureBrowse.Click += new System.EventHandler(this.buttonTextureBrowse_Click);
            // 
            // textBoxTexturePath
            // 
            this.textBoxTexturePath.Location = new System.Drawing.Point(12, 212);
            this.textBoxTexturePath.Name = "textBoxTexturePath";
            this.textBoxTexturePath.ReadOnly = true;
            this.textBoxTexturePath.Size = new System.Drawing.Size(218, 20);
            this.textBoxTexturePath.TabIndex = 7;
            // 
            // buttonStart
            // 
            this.buttonStart.Location = new System.Drawing.Point(456, 11);
            this.buttonStart.Name = "buttonStart";
            this.buttonStart.Size = new System.Drawing.Size(75, 23);
            this.buttonStart.TabIndex = 6;
            this.buttonStart.Text = "Start";
            this.buttonStart.UseVisualStyleBackColor = true;
            this.buttonStart.Click += new System.EventHandler(this.buttonStart_Click);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(280, 16);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(47, 13);
            this.label6.TabIndex = 5;
            this.label6.Text = "Duration";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(141, 16);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(59, 13);
            this.label5.TabIndex = 4;
            this.label5.Text = "Start Delay";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(3, 16);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(77, 13);
            this.label4.TabIndex = 3;
            this.label4.Text = "Release Count";
            // 
            // numericUpDownDuration
            // 
            this.numericUpDownDuration.DecimalPlaces = 3;
            this.numericUpDownDuration.Location = new System.Drawing.Point(333, 14);
            this.numericUpDownDuration.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.numericUpDownDuration.Name = "numericUpDownDuration";
            this.numericUpDownDuration.Size = new System.Drawing.Size(57, 20);
            this.numericUpDownDuration.TabIndex = 2;
            // 
            // numericUpDownStartDelay
            // 
            this.numericUpDownStartDelay.DecimalPlaces = 2;
            this.numericUpDownStartDelay.Location = new System.Drawing.Point(206, 14);
            this.numericUpDownStartDelay.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.numericUpDownStartDelay.Name = "numericUpDownStartDelay";
            this.numericUpDownStartDelay.Size = new System.Drawing.Size(56, 20);
            this.numericUpDownStartDelay.TabIndex = 1;
            // 
            // numericUpDownReleaseCount
            // 
            this.numericUpDownReleaseCount.Location = new System.Drawing.Point(86, 14);
            this.numericUpDownReleaseCount.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownReleaseCount.Name = "numericUpDownReleaseCount";
            this.numericUpDownReleaseCount.Size = new System.Drawing.Size(41, 20);
            this.numericUpDownReleaseCount.TabIndex = 0;
            this.numericUpDownReleaseCount.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(87, 329);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(14, 13);
            this.label9.TabIndex = 16;
            this.label9.Text = "Y";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(10, 329);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(14, 13);
            this.label10.TabIndex = 15;
            this.label10.Text = "X";
            // 
            // numericUpDownSpawnPosY
            // 
            this.numericUpDownSpawnPosY.Location = new System.Drawing.Point(107, 327);
            this.numericUpDownSpawnPosY.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnPosY.Name = "numericUpDownSpawnPosY";
            this.numericUpDownSpawnPosY.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSpawnPosY.TabIndex = 14;
            // 
            // numericUpDownSpawnPosX
            // 
            this.numericUpDownSpawnPosX.Location = new System.Drawing.Point(30, 327);
            this.numericUpDownSpawnPosX.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnPosX.Name = "numericUpDownSpawnPosX";
            this.numericUpDownSpawnPosX.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSpawnPosX.TabIndex = 13;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(86, 518);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(14, 13);
            this.label8.TabIndex = 12;
            this.label8.Text = "Y";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(9, 518);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(14, 13);
            this.label7.TabIndex = 11;
            this.label7.Text = "X";
            // 
            // numericUpDownSpawnVelY
            // 
            this.numericUpDownSpawnVelY.Location = new System.Drawing.Point(106, 516);
            this.numericUpDownSpawnVelY.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnVelY.Name = "numericUpDownSpawnVelY";
            this.numericUpDownSpawnVelY.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSpawnVelY.TabIndex = 10;
            // 
            // numericUpDownSpawnVelX
            // 
            this.numericUpDownSpawnVelX.Location = new System.Drawing.Point(29, 516);
            this.numericUpDownSpawnVelX.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.numericUpDownSpawnVelX.Name = "numericUpDownSpawnVelX";
            this.numericUpDownSpawnVelX.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownSpawnVelX.TabIndex = 9;
            // 
            // buttonSpawnVelAdd
            // 
            this.buttonSpawnVelAdd.Location = new System.Drawing.Point(160, 513);
            this.buttonSpawnVelAdd.Name = "buttonSpawnVelAdd";
            this.buttonSpawnVelAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonSpawnVelAdd.TabIndex = 8;
            this.buttonSpawnVelAdd.Text = "Add";
            this.buttonSpawnVelAdd.UseVisualStyleBackColor = true;
            this.buttonSpawnVelAdd.Click += new System.EventHandler(this.buttonSpawnVelAdd_Click);
            // 
            // listBoxSpawnVelocities
            // 
            this.listBoxSpawnVelocities.FormattingEnabled = true;
            this.listBoxSpawnVelocities.Location = new System.Drawing.Point(3, 386);
            this.listBoxSpawnVelocities.Name = "listBoxSpawnVelocities";
            this.listBoxSpawnVelocities.Size = new System.Drawing.Size(233, 121);
            this.listBoxSpawnVelocities.TabIndex = 7;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 370);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(131, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Random Spawn Velocities";
            // 
            // buttonSpawnPosAdd
            // 
            this.buttonSpawnPosAdd.Location = new System.Drawing.Point(161, 324);
            this.buttonSpawnPosAdd.Name = "buttonSpawnPosAdd";
            this.buttonSpawnPosAdd.Size = new System.Drawing.Size(75, 23);
            this.buttonSpawnPosAdd.TabIndex = 5;
            this.buttonSpawnPosAdd.Text = "Add";
            this.buttonSpawnPosAdd.UseVisualStyleBackColor = true;
            this.buttonSpawnPosAdd.Click += new System.EventHandler(this.buttonSpawnPosAdd_Click);
            // 
            // listBoxSpawnPoints
            // 
            this.listBoxSpawnPoints.FormattingEnabled = true;
            this.listBoxSpawnPoints.Location = new System.Drawing.Point(4, 197);
            this.listBoxSpawnPoints.Name = "listBoxSpawnPoints";
            this.listBoxSpawnPoints.Size = new System.Drawing.Size(232, 121);
            this.listBoxSpawnPoints.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(4, 181);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(115, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Random Spawn Points";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(160, 150);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 2;
            this.button1.Text = "Add";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // listBox1
            // 
            this.listBox1.FormattingEnabled = true;
            this.listBox1.Location = new System.Drawing.Point(3, 23);
            this.listBox1.Name = "listBox1";
            this.listBox1.Size = new System.Drawing.Size(232, 121);
            this.listBox1.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(3, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(49, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Affectors";
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(794, 549);
            this.Controls.Add(this.splitContainer1);
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(810, 584);
            this.MinimumSize = new System.Drawing.Size(810, 584);
            this.Name = "MainWindow";
            this.Text = "Particle Editor";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel1.PerformLayout();
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSizeY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSizeX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownDuration)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownStartDelay)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownReleaseCount)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnPosX)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnVelY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownSpawnVelX)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.ListBox listBox1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonSpawnPosAdd;
        private System.Windows.Forms.ListBox listBoxSpawnPoints;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button buttonSpawnVelAdd;
        private System.Windows.Forms.ListBox listBoxSpawnVelocities;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown numericUpDownDuration;
        private System.Windows.Forms.NumericUpDown numericUpDownStartDelay;
        private System.Windows.Forms.NumericUpDown numericUpDownReleaseCount;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.Button buttonStart;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnVelY;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnVelX;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnPosY;
        private System.Windows.Forms.NumericUpDown numericUpDownSpawnPosX;
        private System.Windows.Forms.Button buttonTextureBrowse;
        private System.Windows.Forms.TextBox textBoxTexturePath;
        private System.Windows.Forms.Button buttonTextureFit;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label13;
        private System.Windows.Forms.NumericUpDown numericUpDownSizeY;
        private System.Windows.Forms.NumericUpDown numericUpDownSizeX;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label14;
    }
}

