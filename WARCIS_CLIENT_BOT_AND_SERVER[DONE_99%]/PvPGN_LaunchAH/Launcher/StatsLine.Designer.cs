namespace Launcher
{
    partial class StatsLine
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
            this.PreviewStatsString = new System.Windows.Forms.RichTextBox();
            this.StatsEnterLine = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.availabledStats = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // PreviewStatsString
            // 
            this.PreviewStatsString.BackColor = System.Drawing.Color.LightSlateGray;
            this.PreviewStatsString.DetectUrls = false;
            this.PreviewStatsString.Font = new System.Drawing.Font("Microsoft Sans Serif", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.PreviewStatsString.ForeColor = System.Drawing.Color.White;
            this.PreviewStatsString.Location = new System.Drawing.Point(12, 48);
            this.PreviewStatsString.MaxLength = 1023;
            this.PreviewStatsString.Multiline = false;
            this.PreviewStatsString.Name = "PreviewStatsString";
            this.PreviewStatsString.ReadOnly = true;
            this.PreviewStatsString.ShortcutsEnabled = false;
            this.PreviewStatsString.Size = new System.Drawing.Size(616, 29);
            this.PreviewStatsString.TabIndex = 0;
            this.PreviewStatsString.Text = "PTS:{MMR}   WINS:{WINS}  /  LOSE:{LOSES}   LEAVES:{LEAVES}";
            // 
            // StatsEnterLine
            // 
            this.StatsEnterLine.Location = new System.Drawing.Point(12, 12);
            this.StatsEnterLine.MaxLength = 1023;
            this.StatsEnterLine.Name = "StatsEnterLine";
            this.StatsEnterLine.Size = new System.Drawing.Size(616, 20);
            this.StatsEnterLine.TabIndex = 1;
            this.StatsEnterLine.Text = "PTS:{MMR}   WINS:{WINS}  /  LOSE:{LOSES}   LEAVES:{LEAVES}";
            this.StatsEnterLine.TextChanged += new System.EventHandler(this.StatsEnterLine_TextChanged);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(494, 97);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 2;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // availabledStats
            // 
            this.availabledStats.FormattingEnabled = true;
            this.availabledStats.Items.AddRange(new object[] {
            "MMR",
            "LEAVES",
            "LOSES",
            "WINS",
            "MAXSTREAK",
            "MINSTREAK",
            "STREAK",
            "KILLS",
            "DEATHS",
            "ASSISTS"});
            this.availabledStats.Location = new System.Drawing.Point(32, 97);
            this.availabledStats.Name = "availabledStats";
            this.availabledStats.Size = new System.Drawing.Size(222, 21);
            this.availabledStats.TabIndex = 3;
            this.availabledStats.SelectedIndexChanged += new System.EventHandler(this.availabledStats_SelectedIndexChanged);
            // 
            // StatsLine
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(640, 132);
            this.Controls.Add(this.availabledStats);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.StatsEnterLine);
            this.Controls.Add(this.PreviewStatsString);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "StatsLine";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "StatsLine";
            this.Load += new System.EventHandler(this.StatsLine_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RichTextBox PreviewStatsString;
        private System.Windows.Forms.TextBox StatsEnterLine;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.ComboBox availabledStats;
    }
}