namespace ParticleEditor
{
    partial class AffectorWindow
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
            this.panelForce = new System.Windows.Forms.Panel();
            this.buttonOk = new System.Windows.Forms.Button();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.numericUpDownForceY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownForceX = new System.Windows.Forms.NumericUpDown();
            this.label1 = new System.Windows.Forms.Label();
            this.panelColour = new System.Windows.Forms.Panel();
            this.button1 = new System.Windows.Forms.Button();
            this.numericUpDownColour = new System.Windows.Forms.NumericUpDown();
            this.panelColourEnd = new System.Windows.Forms.Panel();
            this.panelColourStart = new System.Windows.Forms.Panel();
            this.label2 = new System.Windows.Forms.Label();
            this.panelRotation = new System.Windows.Forms.Panel();
            this.button2 = new System.Windows.Forms.Button();
            this.numericUpDownRotation = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.panelScale = new System.Windows.Forms.Panel();
            this.button3 = new System.Windows.Forms.Button();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.numericUpDownScaleY = new System.Windows.Forms.NumericUpDown();
            this.numericUpDownScaleX = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.panelForce.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownForceY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownForceX)).BeginInit();
            this.panelColour.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownColour)).BeginInit();
            this.panelRotation.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownRotation)).BeginInit();
            this.panelScale.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScaleY)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScaleX)).BeginInit();
            this.SuspendLayout();
            // 
            // panelForce
            // 
            this.panelForce.Controls.Add(this.buttonOk);
            this.panelForce.Controls.Add(this.label8);
            this.panelForce.Controls.Add(this.label7);
            this.panelForce.Controls.Add(this.numericUpDownForceY);
            this.panelForce.Controls.Add(this.numericUpDownForceX);
            this.panelForce.Controls.Add(this.label1);
            this.panelForce.Location = new System.Drawing.Point(13, 13);
            this.panelForce.Name = "panelForce";
            this.panelForce.Size = new System.Drawing.Size(259, 49);
            this.panelForce.TabIndex = 1;
            this.panelForce.Visible = false;
            // 
            // buttonOk
            // 
            this.buttonOk.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.buttonOk.Location = new System.Drawing.Point(181, 18);
            this.buttonOk.Name = "buttonOk";
            this.buttonOk.Size = new System.Drawing.Size(75, 23);
            this.buttonOk.TabIndex = 20;
            this.buttonOk.Text = "OK";
            this.buttonOk.UseVisualStyleBackColor = true;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(81, 23);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(14, 13);
            this.label8.TabIndex = 18;
            this.label8.Text = "Y";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(4, 23);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(14, 13);
            this.label7.TabIndex = 16;
            this.label7.Text = "X";
            // 
            // numericUpDownForceY
            // 
            this.numericUpDownForceY.Location = new System.Drawing.Point(101, 21);
            this.numericUpDownForceY.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.numericUpDownForceY.Name = "numericUpDownForceY";
            this.numericUpDownForceY.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownForceY.TabIndex = 19;
            // 
            // numericUpDownForceX
            // 
            this.numericUpDownForceX.Location = new System.Drawing.Point(24, 21);
            this.numericUpDownForceX.Maximum = new decimal(new int[] {
            500,
            0,
            0,
            0});
            this.numericUpDownForceX.Minimum = new decimal(new int[] {
            500,
            0,
            0,
            -2147483648});
            this.numericUpDownForceX.Name = "numericUpDownForceX";
            this.numericUpDownForceX.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownForceX.TabIndex = 17;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(4, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Force";
            // 
            // panelColour
            // 
            this.panelColour.Controls.Add(this.button1);
            this.panelColour.Controls.Add(this.numericUpDownColour);
            this.panelColour.Controls.Add(this.panelColourEnd);
            this.panelColour.Controls.Add(this.panelColourStart);
            this.panelColour.Controls.Add(this.label2);
            this.panelColour.Location = new System.Drawing.Point(12, 68);
            this.panelColour.Name = "panelColour";
            this.panelColour.Size = new System.Drawing.Size(257, 59);
            this.panelColour.TabIndex = 2;
            this.panelColour.Visible = false;
            // 
            // button1
            // 
            this.button1.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button1.Location = new System.Drawing.Point(179, 33);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 21;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            // 
            // numericUpDownColour
            // 
            this.numericUpDownColour.DecimalPlaces = 1;
            this.numericUpDownColour.Increment = new decimal(new int[] {
            5,
            0,
            0,
            65536});
            this.numericUpDownColour.Location = new System.Drawing.Point(112, 36);
            this.numericUpDownColour.Maximum = new decimal(new int[] {
            60,
            0,
            0,
            0});
            this.numericUpDownColour.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDownColour.Name = "numericUpDownColour";
            this.numericUpDownColour.Size = new System.Drawing.Size(53, 20);
            this.numericUpDownColour.TabIndex = 3;
            this.numericUpDownColour.Value = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            // 
            // panelColourEnd
            // 
            this.panelColourEnd.BackColor = System.Drawing.Color.White;
            this.panelColourEnd.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelColourEnd.Location = new System.Drawing.Point(60, 16);
            this.panelColourEnd.Name = "panelColourEnd";
            this.panelColourEnd.Size = new System.Drawing.Size(46, 40);
            this.panelColourEnd.TabIndex = 2;
            this.panelColourEnd.Click += new System.EventHandler(this.panelColourEnd_Click);
            // 
            // panelColourStart
            // 
            this.panelColourStart.BackColor = System.Drawing.Color.White;
            this.panelColourStart.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelColourStart.Location = new System.Drawing.Point(8, 16);
            this.panelColourStart.Name = "panelColourStart";
            this.panelColourStart.Size = new System.Drawing.Size(46, 40);
            this.panelColourStart.TabIndex = 1;
            this.panelColourStart.Click += new System.EventHandler(this.panelColourStart_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(5, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(37, 13);
            this.label2.TabIndex = 0;
            this.label2.Text = "Colour";
            // 
            // panelRotation
            // 
            this.panelRotation.Controls.Add(this.button2);
            this.panelRotation.Controls.Add(this.numericUpDownRotation);
            this.panelRotation.Controls.Add(this.label3);
            this.panelRotation.Location = new System.Drawing.Point(12, 133);
            this.panelRotation.Name = "panelRotation";
            this.panelRotation.Size = new System.Drawing.Size(257, 59);
            this.panelRotation.TabIndex = 3;
            this.panelRotation.Visible = false;
            // 
            // button2
            // 
            this.button2.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button2.Location = new System.Drawing.Point(179, 23);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 21;
            this.button2.Text = "OK";
            this.button2.UseVisualStyleBackColor = true;
            // 
            // numericUpDownRotation
            // 
            this.numericUpDownRotation.DecimalPlaces = 1;
            this.numericUpDownRotation.Location = new System.Drawing.Point(25, 26);
            this.numericUpDownRotation.Maximum = new decimal(new int[] {
            360,
            0,
            0,
            0});
            this.numericUpDownRotation.Minimum = new decimal(new int[] {
            360,
            0,
            0,
            -2147483648});
            this.numericUpDownRotation.Name = "numericUpDownRotation";
            this.numericUpDownRotation.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownRotation.TabIndex = 1;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(5, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(47, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Rotation";
            // 
            // panelScale
            // 
            this.panelScale.Controls.Add(this.button3);
            this.panelScale.Controls.Add(this.label5);
            this.panelScale.Controls.Add(this.label6);
            this.panelScale.Controls.Add(this.numericUpDownScaleY);
            this.panelScale.Controls.Add(this.numericUpDownScaleX);
            this.panelScale.Controls.Add(this.label4);
            this.panelScale.Location = new System.Drawing.Point(13, 198);
            this.panelScale.Name = "panelScale";
            this.panelScale.Size = new System.Drawing.Size(256, 59);
            this.panelScale.TabIndex = 4;
            this.panelScale.Visible = false;
            // 
            // button3
            // 
            this.button3.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.button3.Location = new System.Drawing.Point(178, 18);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 24;
            this.button3.Text = "OK";
            this.button3.UseVisualStyleBackColor = true;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(81, 23);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(14, 13);
            this.label5.TabIndex = 22;
            this.label5.Text = "Y";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(4, 23);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(14, 13);
            this.label6.TabIndex = 20;
            this.label6.Text = "X";
            // 
            // numericUpDownScaleY
            // 
            this.numericUpDownScaleY.DecimalPlaces = 1;
            this.numericUpDownScaleY.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDownScaleY.Location = new System.Drawing.Point(101, 21);
            this.numericUpDownScaleY.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownScaleY.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            -2147483648});
            this.numericUpDownScaleY.Name = "numericUpDownScaleY";
            this.numericUpDownScaleY.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownScaleY.TabIndex = 23;
            this.numericUpDownScaleY.Value = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            // 
            // numericUpDownScaleX
            // 
            this.numericUpDownScaleX.DecimalPlaces = 1;
            this.numericUpDownScaleX.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.numericUpDownScaleX.Location = new System.Drawing.Point(24, 21);
            this.numericUpDownScaleX.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownScaleX.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            -2147483648});
            this.numericUpDownScaleX.Name = "numericUpDownScaleX";
            this.numericUpDownScaleX.Size = new System.Drawing.Size(46, 20);
            this.numericUpDownScaleX.TabIndex = 21;
            this.numericUpDownScaleX.Value = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(4, 0);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(34, 13);
            this.label4.TabIndex = 0;
            this.label4.Text = "Scale";
            // 
            // AffectorWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 267);
            this.Controls.Add(this.panelRotation);
            this.Controls.Add(this.panelColour);
            this.Controls.Add(this.panelForce);
            this.Controls.Add(this.panelScale);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AffectorWindow";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Add Affector";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AffectorWindow_FormClosing);
            this.panelForce.ResumeLayout(false);
            this.panelForce.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownForceY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownForceX)).EndInit();
            this.panelColour.ResumeLayout(false);
            this.panelColour.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownColour)).EndInit();
            this.panelRotation.ResumeLayout(false);
            this.panelRotation.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownRotation)).EndInit();
            this.panelScale.ResumeLayout(false);
            this.panelScale.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScaleY)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScaleX)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Panel panelForce;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.NumericUpDown numericUpDownForceY;
        private System.Windows.Forms.NumericUpDown numericUpDownForceX;
        private System.Windows.Forms.Panel panelColour;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Panel panelRotation;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Panel panelScale;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.NumericUpDown numericUpDownScaleY;
        private System.Windows.Forms.NumericUpDown numericUpDownScaleX;
        private System.Windows.Forms.NumericUpDown numericUpDownRotation;
        private System.Windows.Forms.Panel panelColourEnd;
        private System.Windows.Forms.Panel panelColourStart;
        private System.Windows.Forms.NumericUpDown numericUpDownColour;
        private System.Windows.Forms.Button buttonOk;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.Button button3;
    }
}