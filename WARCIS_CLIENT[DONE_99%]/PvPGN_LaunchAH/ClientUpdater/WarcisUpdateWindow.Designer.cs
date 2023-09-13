namespace ClientUpdater
{
    partial class WarcisUpdateWindow
    {
        /// <summary>
        /// Обязательная переменная конструктора.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Освободить все используемые ресурсы.
        /// </summary>
        /// <param name="disposing">истинно, если управляемый ресурс должен быть удален; иначе ложно.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Код, автоматически созданный конструктором форм Windows

        /// <summary>
        /// Требуемый метод для поддержки конструктора — не изменяйте 
        /// содержимое этого метода с помощью редактора кода.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.CancelUpdateBtn = new System.Windows.Forms.Button();
            this.ListFilesForDownload = new System.Windows.Forms.ListBox();
            this.UpdateListFiles = new System.Windows.Forms.Timer(this.components);
            this.SuspendLayout();
            // 
            // CancelUpdateBtn
            // 
            this.CancelUpdateBtn.Location = new System.Drawing.Point(300, 140);
            this.CancelUpdateBtn.Name = "CancelUpdateBtn";
            this.CancelUpdateBtn.Size = new System.Drawing.Size(75, 23);
            this.CancelUpdateBtn.TabIndex = 0;
            this.CancelUpdateBtn.Text = "Cancel";
            this.CancelUpdateBtn.UseVisualStyleBackColor = true;
            this.CancelUpdateBtn.Click += new System.EventHandler(this.CancelUpdateBtn_Click);
            // 
            // ListFilesForDownload
            // 
            this.ListFilesForDownload.BackColor = System.Drawing.Color.DimGray;
            this.ListFilesForDownload.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.ListFilesForDownload.ForeColor = System.Drawing.Color.White;
            this.ListFilesForDownload.FormattingEnabled = true;
            this.ListFilesForDownload.Location = new System.Drawing.Point(12, 33);
            this.ListFilesForDownload.Name = "ListFilesForDownload";
            this.ListFilesForDownload.Size = new System.Drawing.Size(363, 95);
            this.ListFilesForDownload.TabIndex = 2;
            // 
            // UpdateListFiles
            // 
            this.UpdateListFiles.Tick += new System.EventHandler(this.UpdateListFiles_Tick);
            // 
            // UpdateWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackgroundImage = global::ClientUpdater.Properties.Resources.ClientUpdaterFont;
            this.ClientSize = new System.Drawing.Size(388, 175);
            this.Controls.Add(this.ListFilesForDownload);
            this.Controls.Add(this.CancelUpdateBtn);
            this.DoubleBuffered = true;
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "UpdateWindow";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Warcis Client Updater (Warcraft III)";
            this.Load += new System.EventHandler(this.UpdateWindow_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button CancelUpdateBtn;
        private System.Windows.Forms.ListBox ListFilesForDownload;
        private System.Windows.Forms.Timer UpdateListFiles;
    }


}

