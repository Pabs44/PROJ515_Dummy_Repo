namespace Test2
{
    partial class Form1
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.neckTbox = new System.Windows.Forms.TextBox();
            this.chestTbox = new System.Windows.Forms.TextBox();
            this.waistTbox = new System.Windows.Forms.TextBox();
            this.hipsTbox = new System.Windows.Forms.TextBox();
            this.sendVals = new System.Windows.Forms.Button();
            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.label5 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // neckTbox
            // 
            this.neckTbox.Location = new System.Drawing.Point(442, 123);
            this.neckTbox.Name = "neckTbox";
            this.neckTbox.Size = new System.Drawing.Size(100, 26);
            this.neckTbox.TabIndex = 0;
            // 
            // chestTbox
            // 
            this.chestTbox.Location = new System.Drawing.Point(442, 177);
            this.chestTbox.Name = "chestTbox";
            this.chestTbox.Size = new System.Drawing.Size(100, 26);
            this.chestTbox.TabIndex = 1;
            // 
            // waistTbox
            // 
            this.waistTbox.Location = new System.Drawing.Point(442, 239);
            this.waistTbox.Name = "waistTbox";
            this.waistTbox.Size = new System.Drawing.Size(100, 26);
            this.waistTbox.TabIndex = 2;
            // 
            // hipsTbox
            // 
            this.hipsTbox.Location = new System.Drawing.Point(442, 299);
            this.hipsTbox.Name = "hipsTbox";
            this.hipsTbox.Size = new System.Drawing.Size(100, 26);
            this.hipsTbox.TabIndex = 3;
            // 
            // sendVals
            // 
            this.sendVals.Location = new System.Drawing.Point(619, 207);
            this.sendVals.Name = "sendVals";
            this.sendVals.Size = new System.Drawing.Size(75, 35);
            this.sendVals.TabIndex = 4;
            this.sendVals.Text = "Send";
            this.sendVals.UseVisualStyleBackColor = true;
            this.sendVals.Click += new System.EventHandler(this.sendVals_Click);
            // 
            // serialPort1
            // 
            this.serialPort1.PortName = "COM4";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(324, 126);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(49, 20);
            this.label1.TabIndex = 5;
            this.label1.Text = "Neck";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(324, 183);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(56, 20);
            this.label2.TabIndex = 6;
            this.label2.Text = "Chest";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label3.Location = new System.Drawing.Point(326, 242);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(54, 20);
            this.label3.TabIndex = 7;
            this.label3.Text = "Waist";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label4.Location = new System.Drawing.Point(326, 302);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(45, 20);
            this.label4.TabIndex = 8;
            this.label4.Text = "Hips";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = ((System.Drawing.Image)(resources.GetObject("pictureBox1.Image")));
            this.pictureBox1.InitialImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.InitialImage")));
            this.pictureBox1.Location = new System.Drawing.Point(27, 30);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(236, 368);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 9;
            this.pictureBox1.TabStop = false;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("Microsoft Tai Le", 20F, ((System.Drawing.FontStyle)((System.Drawing.FontStyle.Bold | System.Drawing.FontStyle.Underline))), System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label5.Location = new System.Drawing.Point(296, 30);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(146, 51);
            this.label5.TabIndex = 10;
            this.label5.Text = "Test 2:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.sendVals);
            this.Controls.Add(this.hipsTbox);
            this.Controls.Add(this.waistTbox);
            this.Controls.Add(this.chestTbox);
            this.Controls.Add(this.neckTbox);
            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Name = "Form1";
            this.Text = "Form1";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox neckTbox;
        private System.Windows.Forms.TextBox chestTbox;
        private System.Windows.Forms.TextBox waistTbox;
        private System.Windows.Forms.TextBox hipsTbox;
        private System.Windows.Forms.Button sendVals;
        private System.IO.Ports.SerialPort serialPort1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Label label5;
    }
}

