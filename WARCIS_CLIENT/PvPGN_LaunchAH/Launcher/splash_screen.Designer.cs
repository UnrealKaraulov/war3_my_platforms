namespace Launcher
{
    partial class splash_screen
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
            this.StarTimer = new System.Windows.Forms.Timer(this.components);
            this.WarcisPicture = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.WarcisPicture)).BeginInit();
            this.SuspendLayout();
            // 
            // StarTimer
            // 
            this.StarTimer.Enabled = true;
            this.StarTimer.Interval = 1000;
            this.StarTimer.Tick += new System.EventHandler(this.StarTimer_Tick);
            // 
            // WarcisPicture
            // 
            this.WarcisPicture.BackColor = System.Drawing.Color.Transparent;
            this.WarcisPicture.BackgroundImage = global::Launcher.Properties.Resources.WarCis_str;
            this.WarcisPicture.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.WarcisPicture.Location = new System.Drawing.Point(12, 12);
            this.WarcisPicture.Name = "WarcisPicture";
            this.WarcisPicture.Size = new System.Drawing.Size(358, 72);
            this.WarcisPicture.TabIndex = 2;
            this.WarcisPicture.TabStop = false;
            this.WarcisPicture.Click += new System.EventHandler(this.WarcisPicture_Click);
            // 
            // splash_screen
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.White;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.ClientSize = new System.Drawing.Size(382, 101);
            this.Controls.Add(this.WarcisPicture);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "splash_screen";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Warcis Wc3 Loading...";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.splash_screen_Load);
            ((System.ComponentModel.ISupportInitialize)(this.WarcisPicture)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Timer StarTimer;
        private System.Windows.Forms.PictureBox WarcisPicture;
    }
}