namespace Launcher
{
    partial class Launcher
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
            this.TimedWorker = new System.Windows.Forms.Timer(this.components);
            this.SelectAccountColorDialog = new System.Windows.Forms.ColorDialog();
            this.LauncherTabPages = new MaterialSkin.Controls.MaterialTabControl();
            this.GeneralPage = new System.Windows.Forms.TabPage();
            this.panel1 = new System.Windows.Forms.Panel();
            this.DownloadRussianVersion = new MaterialSkin.Controls.MaterialCheckBox();
            this.War3Download = new MaterialSkin.Controls.MaterialRaisedButton();
            this.WarcisPicture = new System.Windows.Forms.PictureBox();
            this.War3Button = new MaterialSkin.Controls.MaterialRaisedButton();
            this.ChangeWar3Path = new MaterialSkin.Controls.MaterialRaisedButton();
            this.materialLabel2 = new MaterialSkin.Controls.MaterialLabel();
            this.materialLabel1 = new MaterialSkin.Controls.MaterialLabel();
            this.AutoLoginCheckBox = new MaterialSkin.Controls.MaterialCheckBox();
            this.AutologinUsername = new MaterialSkin.Controls.MaterialSingleLineTextField();
            this.AutologinPassword = new MaterialSkin.Controls.MaterialSingleLineTextField();
            this.SettingsPage = new System.Windows.Forms.TabPage();
            this.SettingsPageTabSelector = new MaterialSkin.Controls.MaterialTabSelector();
            this.SettingsPageTab = new MaterialSkin.Controls.MaterialTabControl();
            this.VideoSettings = new System.Windows.Forms.TabPage();
            this.FPSLabel = new MaterialSkin.Controls.MaterialLabel();
            this.GrayScaleWorld = new MaterialSkin.Controls.MaterialCheckBox();
            this.ActivateOpenglMode = new MaterialSkin.Controls.MaterialCheckBox();
            this.UseNewD3dMode = new MaterialSkin.Controls.MaterialCheckBox();
            this.EnableVSYNC = new MaterialSkin.Controls.MaterialCheckBox();
            this.NeedClipCursor = new MaterialSkin.Controls.MaterialCheckBox();
            this.InWindowMode = new MaterialSkin.Controls.MaterialCheckBox();
            this.UseFullScrenSwitcher = new MaterialSkin.Controls.MaterialCheckBox();
            this.AvailableFpsList = new System.Windows.Forms.ComboBox();
            this.GlobalColorEffect = new System.Windows.Forms.ComboBox();
            this.UserCustomizeSettings = new System.Windows.Forms.TabPage();
            this.materialLabel6 = new MaterialSkin.Controls.MaterialLabel();
            this.materialLabel5 = new MaterialSkin.Controls.MaterialLabel();
            this.MaxPreloadTime = new MaterialSkin.Controls.MaterialSingleLineTextField();
            this.UseDefaultErrorCatch = new MaterialSkin.Controls.MaterialCheckBox();
            this.BugReportWithoutUser = new MaterialSkin.Controls.MaterialCheckBox();
            this.MinimapRightClickWithAlt = new MaterialSkin.Controls.MaterialCheckBox();
            this.ShiftNumpadTrick = new MaterialSkin.Controls.MaterialCheckBox();
            this.UseCustomMpq = new MaterialSkin.Controls.MaterialCheckBox();
            this.WarkeyEnabled = new MaterialSkin.Controls.MaterialCheckBox();
            this.WarKeyBtn = new MaterialSkin.Controls.MaterialRaisedButton();
            this.SelectChatNameColor = new MaterialSkin.Controls.MaterialRaisedButton();
            this.SelectStatsLineColor = new MaterialSkin.Controls.MaterialRaisedButton();
            this.SelectChatColor = new MaterialSkin.Controls.MaterialRaisedButton();
            this.SelectAccountColor = new MaterialSkin.Controls.MaterialRaisedButton();
            this.LauncherSettings = new System.Windows.Forms.TabPage();
            this.LangLabel = new MaterialSkin.Controls.MaterialLabel();
            this.ChangeStyleBtn = new MaterialSkin.Controls.MaterialRaisedButton();
            this.ChangeThemeBtn = new MaterialSkin.Controls.MaterialRaisedButton();
            this.LauncherLanguage = new System.Windows.Forms.ComboBox();
            this.HelpPage = new System.Windows.Forms.TabPage();
            this.materialLabel3 = new MaterialSkin.Controls.MaterialLabel();
            this.materialLabel4 = new MaterialSkin.Controls.MaterialLabel();
            this.label1 = new MaterialSkin.Controls.MaterialLabel();
            this.VeryGoodInfo = new MaterialSkin.Controls.MaterialRaisedButton();
            this.NewsPage = new System.Windows.Forms.TabPage();
            this.WarcisNews = new System.Windows.Forms.WebBrowser();
            this.MapUploader = new System.Windows.Forms.TabPage();
            this.MapUploadPage = new System.Windows.Forms.WebBrowser();
            this.materialTabSelector1 = new MaterialSkin.Controls.MaterialTabSelector();
            this.AudioSettings = new System.Windows.Forms.TabPage();
            this.EnableVoiceChat = new MaterialSkin.Controls.MaterialCheckBox();
            this.EnableMicInput = new MaterialSkin.Controls.MaterialCheckBox();
            this.materialLabel7 = new MaterialSkin.Controls.MaterialLabel();
            this.MicQuality = new System.Windows.Forms.ComboBox();
            this.LauncherTabPages.SuspendLayout();
            this.GeneralPage.SuspendLayout();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.WarcisPicture)).BeginInit();
            this.SettingsPage.SuspendLayout();
            this.SettingsPageTab.SuspendLayout();
            this.VideoSettings.SuspendLayout();
            this.UserCustomizeSettings.SuspendLayout();
            this.LauncherSettings.SuspendLayout();
            this.HelpPage.SuspendLayout();
            this.NewsPage.SuspendLayout();
            this.MapUploader.SuspendLayout();
            this.AudioSettings.SuspendLayout();
            this.SuspendLayout();
            // 
            // TimedWorker
            // 
            this.TimedWorker.Enabled = true;
            this.TimedWorker.Interval = 10;
            this.TimedWorker.Tick += new System.EventHandler(this.TimedWorker_Tick);
            // 
            // SelectAccountColorDialog
            // 
            this.SelectAccountColorDialog.AnyColor = true;
            this.SelectAccountColorDialog.FullOpen = true;
            this.SelectAccountColorDialog.ShowHelp = true;
            this.SelectAccountColorDialog.SolidColorOnly = true;
            this.SelectAccountColorDialog.HelpRequest += new System.EventHandler(this.SelectAccountColorDialog_HelpRequest);
            // 
            // LauncherTabPages
            // 
            this.LauncherTabPages.Controls.Add(this.GeneralPage);
            this.LauncherTabPages.Controls.Add(this.SettingsPage);
            this.LauncherTabPages.Controls.Add(this.HelpPage);
            this.LauncherTabPages.Controls.Add(this.NewsPage);
            this.LauncherTabPages.Controls.Add(this.MapUploader);
            this.LauncherTabPages.Depth = 0;
            this.LauncherTabPages.Location = new System.Drawing.Point(0, 110);
            this.LauncherTabPages.Margin = new System.Windows.Forms.Padding(0);
            this.LauncherTabPages.MouseState = MaterialSkin.MouseState.HOVER;
            this.LauncherTabPages.Name = "LauncherTabPages";
            this.LauncherTabPages.SelectedIndex = 0;
            this.LauncherTabPages.Size = new System.Drawing.Size(640, 370);
            this.LauncherTabPages.TabIndex = 20;
            // 
            // GeneralPage
            // 
            this.GeneralPage.BackColor = System.Drawing.Color.LightGray;
            this.GeneralPage.Controls.Add(this.panel1);
            this.GeneralPage.Controls.Add(this.WarcisPicture);
            this.GeneralPage.Controls.Add(this.War3Button);
            this.GeneralPage.Controls.Add(this.ChangeWar3Path);
            this.GeneralPage.Controls.Add(this.materialLabel2);
            this.GeneralPage.Controls.Add(this.materialLabel1);
            this.GeneralPage.Controls.Add(this.AutoLoginCheckBox);
            this.GeneralPage.Controls.Add(this.AutologinUsername);
            this.GeneralPage.Controls.Add(this.AutologinPassword);
            this.GeneralPage.Location = new System.Drawing.Point(4, 22);
            this.GeneralPage.Name = "GeneralPage";
            this.GeneralPage.Padding = new System.Windows.Forms.Padding(3);
            this.GeneralPage.Size = new System.Drawing.Size(632, 344);
            this.GeneralPage.TabIndex = 0;
            this.GeneralPage.Text = "WARCIS";
            this.GeneralPage.Click += new System.EventHandler(this.GeneralPage_Click);
            // 
            // panel1
            // 
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.DownloadRussianVersion);
            this.panel1.Controls.Add(this.War3Download);
            this.panel1.Location = new System.Drawing.Point(201, 104);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(428, 67);
            this.panel1.TabIndex = 13;
            // 
            // DownloadRussianVersion
            // 
            this.DownloadRussianVersion.AutoSize = true;
            this.DownloadRussianVersion.Checked = true;
            this.DownloadRussianVersion.CheckState = System.Windows.Forms.CheckState.Checked;
            this.DownloadRussianVersion.Depth = 0;
            this.DownloadRussianVersion.Font = new System.Drawing.Font("Roboto", 10F);
            this.DownloadRussianVersion.Location = new System.Drawing.Point(246, 18);
            this.DownloadRussianVersion.Margin = new System.Windows.Forms.Padding(0);
            this.DownloadRussianVersion.MouseLocation = new System.Drawing.Point(-1, -1);
            this.DownloadRussianVersion.MouseState = MaterialSkin.MouseState.HOVER;
            this.DownloadRussianVersion.Name = "DownloadRussianVersion";
            this.DownloadRussianVersion.Ripple = true;
            this.DownloadRussianVersion.Size = new System.Drawing.Size(138, 30);
            this.DownloadRussianVersion.TabIndex = 5;
            this.DownloadRussianVersion.TabStop = false;
            this.DownloadRussianVersion.Text = "Русскую версию";
            this.DownloadRussianVersion.UseVisualStyleBackColor = true;
            // 
            // War3Download
            // 
            this.War3Download.AutoSize = true;
            this.War3Download.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.War3Download.BackColor = System.Drawing.Color.Fuchsia;
            this.War3Download.Depth = 0;
            this.War3Download.Icon = null;
            this.War3Download.Location = new System.Drawing.Point(56, 14);
            this.War3Download.MouseState = MaterialSkin.MouseState.HOVER;
            this.War3Download.Name = "War3Download";
            this.War3Download.Primary = true;
            this.War3Download.Size = new System.Drawing.Size(174, 36);
            this.War3Download.TabIndex = 1;
            this.War3Download.TabStop = false;
            this.War3Download.Text = "Скачать Warcraft III";
            this.War3Download.UseVisualStyleBackColor = false;
            this.War3Download.Click += new System.EventHandler(this.War3Download_Click);
            // 
            // WarcisPicture
            // 
            this.WarcisPicture.BackColor = System.Drawing.Color.Transparent;
            this.WarcisPicture.BackgroundImage = global::Launcher.Properties.Resources.WarCis_str;
            this.WarcisPicture.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.WarcisPicture.Location = new System.Drawing.Point(76, 6);
            this.WarcisPicture.Name = "WarcisPicture";
            this.WarcisPicture.Size = new System.Drawing.Size(358, 72);
            this.WarcisPicture.TabIndex = 12;
            this.WarcisPicture.TabStop = false;
            this.WarcisPicture.Click += new System.EventHandler(this.WarcisPicture_Click);
            // 
            // War3Button
            // 
            this.War3Button.AutoSize = true;
            this.War3Button.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.War3Button.Depth = 0;
            this.War3Button.Icon = null;
            this.War3Button.Location = new System.Drawing.Point(266, 292);
            this.War3Button.MouseState = MaterialSkin.MouseState.HOVER;
            this.War3Button.Name = "War3Button";
            this.War3Button.Primary = true;
            this.War3Button.Size = new System.Drawing.Size(192, 36);
            this.War3Button.TabIndex = 1;
            this.War3Button.Text = "Запустить Warcraft III";
            this.War3Button.UseVisualStyleBackColor = true;
            this.War3Button.Click += new System.EventHandler(this.War3Button_Click);
            // 
            // ChangeWar3Path
            // 
            this.ChangeWar3Path.AutoSize = true;
            this.ChangeWar3Path.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ChangeWar3Path.Depth = 0;
            this.ChangeWar3Path.Icon = null;
            this.ChangeWar3Path.Location = new System.Drawing.Point(76, 292);
            this.ChangeWar3Path.MouseState = MaterialSkin.MouseState.HOVER;
            this.ChangeWar3Path.Name = "ChangeWar3Path";
            this.ChangeWar3Path.Primary = true;
            this.ChangeWar3Path.Size = new System.Drawing.Size(184, 36);
            this.ChangeWar3Path.TabIndex = 2;
            this.ChangeWar3Path.Text = "Изменить путь к игре";
            this.ChangeWar3Path.UseVisualStyleBackColor = true;
            this.ChangeWar3Path.Click += new System.EventHandler(this.ChangeWar3Path_Click);
            // 
            // materialLabel2
            // 
            this.materialLabel2.AutoSize = true;
            this.materialLabel2.Depth = 0;
            this.materialLabel2.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel2.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel2.Location = new System.Drawing.Point(213, 213);
            this.materialLabel2.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel2.Name = "materialLabel2";
            this.materialLabel2.Size = new System.Drawing.Size(79, 19);
            this.materialLabel2.TabIndex = 11;
            this.materialLabel2.Text = "Password:";
            // 
            // materialLabel1
            // 
            this.materialLabel1.AutoSize = true;
            this.materialLabel1.Depth = 0;
            this.materialLabel1.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel1.Location = new System.Drawing.Point(72, 213);
            this.materialLabel1.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel1.Name = "materialLabel1";
            this.materialLabel1.Size = new System.Drawing.Size(81, 19);
            this.materialLabel1.TabIndex = 11;
            this.materialLabel1.Text = "Username:";
            // 
            // AutoLoginCheckBox
            // 
            this.AutoLoginCheckBox.AutoSize = true;
            this.AutoLoginCheckBox.Depth = 0;
            this.AutoLoginCheckBox.Font = new System.Drawing.Font("Roboto", 10F);
            this.AutoLoginCheckBox.Location = new System.Drawing.Point(419, 241);
            this.AutoLoginCheckBox.Margin = new System.Windows.Forms.Padding(0);
            this.AutoLoginCheckBox.MouseLocation = new System.Drawing.Point(-1, -1);
            this.AutoLoginCheckBox.MouseState = MaterialSkin.MouseState.HOVER;
            this.AutoLoginCheckBox.Name = "AutoLoginCheckBox";
            this.AutoLoginCheckBox.Ripple = true;
            this.AutoLoginCheckBox.Size = new System.Drawing.Size(93, 30);
            this.AutoLoginCheckBox.TabIndex = 5;
            this.AutoLoginCheckBox.Text = "AutoLogin";
            this.AutoLoginCheckBox.UseVisualStyleBackColor = true;
            this.AutoLoginCheckBox.CheckedChanged += new System.EventHandler(this.AutoLoginCheckBox_CheckedChanged);
            // 
            // AutologinUsername
            // 
            this.AutologinUsername.Depth = 0;
            this.AutologinUsername.Hint = "Enter username:";
            this.AutologinUsername.Location = new System.Drawing.Point(76, 246);
            this.AutologinUsername.MaxLength = 16;
            this.AutologinUsername.MouseState = MaterialSkin.MouseState.HOVER;
            this.AutologinUsername.Name = "AutologinUsername";
            this.AutologinUsername.PasswordChar = '\0';
            this.AutologinUsername.SelectedText = "";
            this.AutologinUsername.SelectionLength = 0;
            this.AutologinUsername.SelectionStart = 0;
            this.AutologinUsername.Size = new System.Drawing.Size(135, 23);
            this.AutologinUsername.TabIndex = 3;
            this.AutologinUsername.TabStop = false;
            this.AutologinUsername.UseSystemPasswordChar = false;
            this.AutologinUsername.TextChanged += new System.EventHandler(this.AutologinUsername_TextChanged);
            // 
            // AutologinPassword
            // 
            this.AutologinPassword.Depth = 0;
            this.AutologinPassword.Hint = "Enter password here";
            this.AutologinPassword.Location = new System.Drawing.Point(217, 246);
            this.AutologinPassword.MaxLength = 45;
            this.AutologinPassword.MouseState = MaterialSkin.MouseState.HOVER;
            this.AutologinPassword.Name = "AutologinPassword";
            this.AutologinPassword.PasswordChar = '*';
            this.AutologinPassword.SelectedText = "";
            this.AutologinPassword.SelectionLength = 0;
            this.AutologinPassword.SelectionStart = 0;
            this.AutologinPassword.Size = new System.Drawing.Size(186, 23);
            this.AutologinPassword.TabIndex = 4;
            this.AutologinPassword.TabStop = false;
            this.AutologinPassword.UseSystemPasswordChar = false;
            this.AutologinPassword.TextChanged += new System.EventHandler(this.AutologinPassword_TextChanged);
            // 
            // SettingsPage
            // 
            this.SettingsPage.BackColor = System.Drawing.Color.LightGray;
            this.SettingsPage.Controls.Add(this.SettingsPageTabSelector);
            this.SettingsPage.Controls.Add(this.SettingsPageTab);
            this.SettingsPage.ForeColor = System.Drawing.Color.Coral;
            this.SettingsPage.Location = new System.Drawing.Point(4, 22);
            this.SettingsPage.Name = "SettingsPage";
            this.SettingsPage.Padding = new System.Windows.Forms.Padding(3);
            this.SettingsPage.Size = new System.Drawing.Size(632, 344);
            this.SettingsPage.TabIndex = 1;
            this.SettingsPage.Text = "SETTINGS";
            // 
            // SettingsPageTabSelector
            // 
            this.SettingsPageTabSelector.BackColor = System.Drawing.Color.SteelBlue;
            this.SettingsPageTabSelector.BaseTabControl = this.SettingsPageTab;
            this.SettingsPageTabSelector.Depth = 0;
            this.SettingsPageTabSelector.Location = new System.Drawing.Point(0, 3);
            this.SettingsPageTabSelector.MouseState = MaterialSkin.MouseState.HOVER;
            this.SettingsPageTabSelector.Name = "SettingsPageTabSelector";
            this.SettingsPageTabSelector.Size = new System.Drawing.Size(635, 47);
            this.SettingsPageTabSelector.TabIndex = 21;
            this.SettingsPageTabSelector.Text = "SelectLauncherPage";
            // 
            // SettingsPageTab
            // 
            this.SettingsPageTab.Controls.Add(this.VideoSettings);
            this.SettingsPageTab.Controls.Add(this.UserCustomizeSettings);
            this.SettingsPageTab.Controls.Add(this.LauncherSettings);
            this.SettingsPageTab.Controls.Add(this.AudioSettings);
            this.SettingsPageTab.Depth = 0;
            this.SettingsPageTab.Location = new System.Drawing.Point(-4, 50);
            this.SettingsPageTab.MouseState = MaterialSkin.MouseState.HOVER;
            this.SettingsPageTab.Name = "SettingsPageTab";
            this.SettingsPageTab.SelectedIndex = 0;
            this.SettingsPageTab.Size = new System.Drawing.Size(639, 297);
            this.SettingsPageTab.TabIndex = 30;
            // 
            // VideoSettings
            // 
            this.VideoSettings.BackColor = System.Drawing.Color.WhiteSmoke;
            this.VideoSettings.Controls.Add(this.FPSLabel);
            this.VideoSettings.Controls.Add(this.GrayScaleWorld);
            this.VideoSettings.Controls.Add(this.ActivateOpenglMode);
            this.VideoSettings.Controls.Add(this.UseNewD3dMode);
            this.VideoSettings.Controls.Add(this.EnableVSYNC);
            this.VideoSettings.Controls.Add(this.NeedClipCursor);
            this.VideoSettings.Controls.Add(this.InWindowMode);
            this.VideoSettings.Controls.Add(this.UseFullScrenSwitcher);
            this.VideoSettings.Controls.Add(this.AvailableFpsList);
            this.VideoSettings.Controls.Add(this.GlobalColorEffect);
            this.VideoSettings.Location = new System.Drawing.Point(4, 22);
            this.VideoSettings.Name = "VideoSettings";
            this.VideoSettings.Padding = new System.Windows.Forms.Padding(3);
            this.VideoSettings.Size = new System.Drawing.Size(631, 271);
            this.VideoSettings.TabIndex = 0;
            this.VideoSettings.Text = "Video";
            // 
            // FPSLabel
            // 
            this.FPSLabel.AutoSize = true;
            this.FPSLabel.Depth = 0;
            this.FPSLabel.Font = new System.Drawing.Font("Roboto", 11F);
            this.FPSLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.FPSLabel.Location = new System.Drawing.Point(336, 104);
            this.FPSLabel.MouseState = MaterialSkin.MouseState.HOVER;
            this.FPSLabel.Name = "FPSLabel";
            this.FPSLabel.Size = new System.Drawing.Size(39, 19);
            this.FPSLabel.TabIndex = 19;
            this.FPSLabel.Text = "FPS:";
            // 
            // GrayScaleWorld
            // 
            this.GrayScaleWorld.AutoSize = true;
            this.GrayScaleWorld.Depth = 0;
            this.GrayScaleWorld.Font = new System.Drawing.Font("Roboto", 10F);
            this.GrayScaleWorld.Location = new System.Drawing.Point(17, 105);
            this.GrayScaleWorld.Margin = new System.Windows.Forms.Padding(0);
            this.GrayScaleWorld.MouseLocation = new System.Drawing.Point(-1, -1);
            this.GrayScaleWorld.MouseState = MaterialSkin.MouseState.HOVER;
            this.GrayScaleWorld.Name = "GrayScaleWorld";
            this.GrayScaleWorld.Ripple = true;
            this.GrayScaleWorld.Size = new System.Drawing.Size(144, 30);
            this.GrayScaleWorld.TabIndex = 6;
            this.GrayScaleWorld.Text = "Global color effect";
            this.GrayScaleWorld.UseVisualStyleBackColor = true;
            // 
            // ActivateOpenglMode
            // 
            this.ActivateOpenglMode.AutoSize = true;
            this.ActivateOpenglMode.Depth = 0;
            this.ActivateOpenglMode.Font = new System.Drawing.Font("Roboto", 10F);
            this.ActivateOpenglMode.Location = new System.Drawing.Point(17, 75);
            this.ActivateOpenglMode.Margin = new System.Windows.Forms.Padding(0);
            this.ActivateOpenglMode.MouseLocation = new System.Drawing.Point(-1, -1);
            this.ActivateOpenglMode.MouseState = MaterialSkin.MouseState.HOVER;
            this.ActivateOpenglMode.Name = "ActivateOpenglMode";
            this.ActivateOpenglMode.Ripple = true;
            this.ActivateOpenglMode.Size = new System.Drawing.Size(192, 30);
            this.ActivateOpenglMode.TabIndex = 5;
            this.ActivateOpenglMode.Text = "Запуск в Opengl режиме.";
            this.ActivateOpenglMode.UseVisualStyleBackColor = true;
            // 
            // UseNewD3dMode
            // 
            this.UseNewD3dMode.AutoSize = true;
            this.UseNewD3dMode.Checked = true;
            this.UseNewD3dMode.CheckState = System.Windows.Forms.CheckState.Checked;
            this.UseNewD3dMode.Depth = 0;
            this.UseNewD3dMode.Font = new System.Drawing.Font("Roboto", 10F);
            this.UseNewD3dMode.Location = new System.Drawing.Point(17, 15);
            this.UseNewD3dMode.Margin = new System.Windows.Forms.Padding(0);
            this.UseNewD3dMode.MouseLocation = new System.Drawing.Point(-1, -1);
            this.UseNewD3dMode.MouseState = MaterialSkin.MouseState.HOVER;
            this.UseNewD3dMode.Name = "UseNewD3dMode";
            this.UseNewD3dMode.Ripple = true;
            this.UseNewD3dMode.Size = new System.Drawing.Size(180, 30);
            this.UseNewD3dMode.TabIndex = 3;
            this.UseNewD3dMode.Text = "Использовать Directx 9";
            this.UseNewD3dMode.UseVisualStyleBackColor = true;
            this.UseNewD3dMode.CheckedChanged += new System.EventHandler(this.UseNewD3dMode_CheckedChanged);
            // 
            // EnableVSYNC
            // 
            this.EnableVSYNC.AutoSize = true;
            this.EnableVSYNC.Checked = true;
            this.EnableVSYNC.CheckState = System.Windows.Forms.CheckState.Checked;
            this.EnableVSYNC.Depth = 0;
            this.EnableVSYNC.Font = new System.Drawing.Font("Roboto", 10F);
            this.EnableVSYNC.Location = new System.Drawing.Point(17, 45);
            this.EnableVSYNC.Margin = new System.Windows.Forms.Padding(0);
            this.EnableVSYNC.MouseLocation = new System.Drawing.Point(-1, -1);
            this.EnableVSYNC.MouseState = MaterialSkin.MouseState.HOVER;
            this.EnableVSYNC.Name = "EnableVSYNC";
            this.EnableVSYNC.Ripple = true;
            this.EnableVSYNC.Size = new System.Drawing.Size(143, 30);
            this.EnableVSYNC.TabIndex = 3;
            this.EnableVSYNC.Text = "Включить VSYNC";
            this.EnableVSYNC.UseVisualStyleBackColor = true;
            this.EnableVSYNC.CheckedChanged += new System.EventHandler(this.EnableVSYNC_CheckedChanged);
            // 
            // NeedClipCursor
            // 
            this.NeedClipCursor.AutoSize = true;
            this.NeedClipCursor.Depth = 0;
            this.NeedClipCursor.Font = new System.Drawing.Font("Roboto", 10F);
            this.NeedClipCursor.Location = new System.Drawing.Point(345, 72);
            this.NeedClipCursor.Margin = new System.Windows.Forms.Padding(0);
            this.NeedClipCursor.MouseLocation = new System.Drawing.Point(-1, -1);
            this.NeedClipCursor.MouseState = MaterialSkin.MouseState.HOVER;
            this.NeedClipCursor.Name = "NeedClipCursor";
            this.NeedClipCursor.Ripple = true;
            this.NeedClipCursor.Size = new System.Drawing.Size(200, 30);
            this.NeedClipCursor.TabIndex = 4;
            this.NeedClipCursor.Text = "Удерживать мышь в окне";
            this.NeedClipCursor.UseVisualStyleBackColor = true;
            // 
            // InWindowMode
            // 
            this.InWindowMode.AutoSize = true;
            this.InWindowMode.Depth = 0;
            this.InWindowMode.Font = new System.Drawing.Font("Roboto", 10F);
            this.InWindowMode.Location = new System.Drawing.Point(345, 45);
            this.InWindowMode.Margin = new System.Windows.Forms.Padding(0);
            this.InWindowMode.MouseLocation = new System.Drawing.Point(-1, -1);
            this.InWindowMode.MouseState = MaterialSkin.MouseState.HOVER;
            this.InWindowMode.Name = "InWindowMode";
            this.InWindowMode.Ripple = true;
            this.InWindowMode.Size = new System.Drawing.Size(137, 30);
            this.InWindowMode.TabIndex = 4;
            this.InWindowMode.Text = "Оконный режим";
            this.InWindowMode.UseVisualStyleBackColor = true;
            // 
            // UseFullScrenSwitcher
            // 
            this.UseFullScrenSwitcher.AutoSize = true;
            this.UseFullScrenSwitcher.Depth = 0;
            this.UseFullScrenSwitcher.Font = new System.Drawing.Font("Roboto", 10F);
            this.UseFullScrenSwitcher.Location = new System.Drawing.Point(345, 15);
            this.UseFullScrenSwitcher.Margin = new System.Windows.Forms.Padding(0);
            this.UseFullScrenSwitcher.MouseLocation = new System.Drawing.Point(-1, -1);
            this.UseFullScrenSwitcher.MouseState = MaterialSkin.MouseState.HOVER;
            this.UseFullScrenSwitcher.Name = "UseFullScrenSwitcher";
            this.UseFullScrenSwitcher.Ripple = true;
            this.UseFullScrenSwitcher.Size = new System.Drawing.Size(248, 30);
            this.UseFullScrenSwitcher.TabIndex = 2;
            this.UseFullScrenSwitcher.Text = "Полноэкранный режим: Alt+Enter";
            this.UseFullScrenSwitcher.UseVisualStyleBackColor = true;
            // 
            // AvailableFpsList
            // 
            this.AvailableFpsList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.AvailableFpsList.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.AvailableFpsList.FormattingEnabled = true;
            this.AvailableFpsList.Location = new System.Drawing.Point(381, 105);
            this.AvailableFpsList.Name = "AvailableFpsList";
            this.AvailableFpsList.Size = new System.Drawing.Size(66, 21);
            this.AvailableFpsList.TabIndex = 7;
            this.AvailableFpsList.SelectedIndexChanged += new System.EventHandler(this.AvailableFpsList_SelectedIndexChanged);
            // 
            // GlobalColorEffect
            // 
            this.GlobalColorEffect.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.GlobalColorEffect.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.GlobalColorEffect.FormattingEnabled = true;
            this.GlobalColorEffect.Items.AddRange(new object[] {
            "NO EFFECT",
            "GRAYSCALE WORLD",
            "BLUE2GREEN",
            "BLUE2RED",
            "SEPIA",
            "LIGHT TERRAIN",
            "DARK TERRAIN",
            "BROKEN WORLD",
            "DARK WORLD",
            "BLOOD WORLD",
            "GREEN WORLD",
            "UNDERWORLD v1"});
            this.GlobalColorEffect.Location = new System.Drawing.Point(174, 108);
            this.GlobalColorEffect.Name = "GlobalColorEffect";
            this.GlobalColorEffect.Size = new System.Drawing.Size(135, 21);
            this.GlobalColorEffect.TabIndex = 7;
            this.GlobalColorEffect.SelectedIndexChanged += new System.EventHandler(this.GlobalColorEffect_SelectedIndexChanged);
            // 
            // UserCustomizeSettings
            // 
            this.UserCustomizeSettings.BackColor = System.Drawing.Color.WhiteSmoke;
            this.UserCustomizeSettings.Controls.Add(this.materialLabel6);
            this.UserCustomizeSettings.Controls.Add(this.materialLabel5);
            this.UserCustomizeSettings.Controls.Add(this.MaxPreloadTime);
            this.UserCustomizeSettings.Controls.Add(this.UseDefaultErrorCatch);
            this.UserCustomizeSettings.Controls.Add(this.BugReportWithoutUser);
            this.UserCustomizeSettings.Controls.Add(this.MinimapRightClickWithAlt);
            this.UserCustomizeSettings.Controls.Add(this.ShiftNumpadTrick);
            this.UserCustomizeSettings.Controls.Add(this.UseCustomMpq);
            this.UserCustomizeSettings.Controls.Add(this.WarkeyEnabled);
            this.UserCustomizeSettings.Controls.Add(this.WarKeyBtn);
            this.UserCustomizeSettings.Controls.Add(this.SelectChatNameColor);
            this.UserCustomizeSettings.Controls.Add(this.SelectStatsLineColor);
            this.UserCustomizeSettings.Controls.Add(this.SelectChatColor);
            this.UserCustomizeSettings.Controls.Add(this.SelectAccountColor);
            this.UserCustomizeSettings.Location = new System.Drawing.Point(4, 22);
            this.UserCustomizeSettings.Name = "UserCustomizeSettings";
            this.UserCustomizeSettings.Size = new System.Drawing.Size(631, 271);
            this.UserCustomizeSettings.TabIndex = 2;
            this.UserCustomizeSettings.Text = "Персональные";
            this.UserCustomizeSettings.Click += new System.EventHandler(this.UserCustomizeSettings_Click);
            // 
            // materialLabel6
            // 
            this.materialLabel6.AutoSize = true;
            this.materialLabel6.Depth = 0;
            this.materialLabel6.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel6.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel6.Location = new System.Drawing.Point(408, 239);
            this.materialLabel6.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel6.Name = "materialLabel6";
            this.materialLabel6.Size = new System.Drawing.Size(58, 19);
            this.materialLabel6.TabIndex = 44;
            this.materialLabel6.Text = "секунд";
            // 
            // materialLabel5
            // 
            this.materialLabel5.AutoSize = true;
            this.materialLabel5.Depth = 0;
            this.materialLabel5.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel5.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel5.Location = new System.Drawing.Point(34, 239);
            this.materialLabel5.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel5.Name = "materialLabel5";
            this.materialLabel5.Size = new System.Drawing.Size(323, 19);
            this.materialLabel5.TabIndex = 43;
            this.materialLabel5.Text = "Максимальное время на предзагрузку карт:";
            // 
            // MaxPreloadTime
            // 
            this.MaxPreloadTime.Depth = 0;
            this.MaxPreloadTime.Hint = "";
            this.MaxPreloadTime.Location = new System.Drawing.Point(363, 239);
            this.MaxPreloadTime.MaxLength = 3;
            this.MaxPreloadTime.MouseState = MaterialSkin.MouseState.HOVER;
            this.MaxPreloadTime.Name = "MaxPreloadTime";
            this.MaxPreloadTime.PasswordChar = '\0';
            this.MaxPreloadTime.SelectedText = "";
            this.MaxPreloadTime.SelectionLength = 0;
            this.MaxPreloadTime.SelectionStart = 0;
            this.MaxPreloadTime.Size = new System.Drawing.Size(39, 23);
            this.MaxPreloadTime.TabIndex = 42;
            this.MaxPreloadTime.TabStop = false;
            this.MaxPreloadTime.Text = "30";
            this.MaxPreloadTime.UseSystemPasswordChar = false;
            // 
            // UseDefaultErrorCatch
            // 
            this.UseDefaultErrorCatch.AutoSize = true;
            this.UseDefaultErrorCatch.BackColor = System.Drawing.Color.Transparent;
            this.UseDefaultErrorCatch.Depth = 0;
            this.UseDefaultErrorCatch.Font = new System.Drawing.Font("Roboto", 10F);
            this.UseDefaultErrorCatch.ForeColor = System.Drawing.SystemColors.ControlText;
            this.UseDefaultErrorCatch.Location = new System.Drawing.Point(33, 203);
            this.UseDefaultErrorCatch.Margin = new System.Windows.Forms.Padding(0);
            this.UseDefaultErrorCatch.MouseLocation = new System.Drawing.Point(-1, -1);
            this.UseDefaultErrorCatch.MouseState = MaterialSkin.MouseState.HOVER;
            this.UseDefaultErrorCatch.Name = "UseDefaultErrorCatch";
            this.UseDefaultErrorCatch.Ripple = true;
            this.UseDefaultErrorCatch.Size = new System.Drawing.Size(386, 30);
            this.UseDefaultErrorCatch.TabIndex = 41;
            this.UseDefaultErrorCatch.Text = "Использовать стандартный обработчик ошибок Storm";
            this.UseDefaultErrorCatch.UseVisualStyleBackColor = false;
            this.UseDefaultErrorCatch.CheckedChanged += new System.EventHandler(this.UseDefaultErrorCatch_CheckedChanged);
            // 
            // BugReportWithoutUser
            // 
            this.BugReportWithoutUser.AutoSize = true;
            this.BugReportWithoutUser.BackColor = System.Drawing.Color.Transparent;
            this.BugReportWithoutUser.Checked = true;
            this.BugReportWithoutUser.CheckState = System.Windows.Forms.CheckState.Checked;
            this.BugReportWithoutUser.Depth = 0;
            this.BugReportWithoutUser.Font = new System.Drawing.Font("Roboto", 10F);
            this.BugReportWithoutUser.ForeColor = System.Drawing.SystemColors.ControlText;
            this.BugReportWithoutUser.Location = new System.Drawing.Point(33, 173);
            this.BugReportWithoutUser.Margin = new System.Windows.Forms.Padding(0);
            this.BugReportWithoutUser.MouseLocation = new System.Drawing.Point(-1, -1);
            this.BugReportWithoutUser.MouseState = MaterialSkin.MouseState.HOVER;
            this.BugReportWithoutUser.Name = "BugReportWithoutUser";
            this.BugReportWithoutUser.Ripple = true;
            this.BugReportWithoutUser.Size = new System.Drawing.Size(391, 30);
            this.BugReportWithoutUser.TabIndex = 41;
            this.BugReportWithoutUser.Text = "Отправлять сведения об ошибках без предупреждения";
            this.BugReportWithoutUser.UseVisualStyleBackColor = false;
            this.BugReportWithoutUser.CheckedChanged += new System.EventHandler(this.BugReportWithoutUser_CheckedChanged);
            // 
            // MinimapRightClickWithAlt
            // 
            this.MinimapRightClickWithAlt.AutoSize = true;
            this.MinimapRightClickWithAlt.BackColor = System.Drawing.Color.Transparent;
            this.MinimapRightClickWithAlt.Depth = 0;
            this.MinimapRightClickWithAlt.Font = new System.Drawing.Font("Roboto", 10F);
            this.MinimapRightClickWithAlt.ForeColor = System.Drawing.SystemColors.ControlText;
            this.MinimapRightClickWithAlt.Location = new System.Drawing.Point(316, 143);
            this.MinimapRightClickWithAlt.Margin = new System.Windows.Forms.Padding(0);
            this.MinimapRightClickWithAlt.MouseLocation = new System.Drawing.Point(-1, -1);
            this.MinimapRightClickWithAlt.MouseState = MaterialSkin.MouseState.HOVER;
            this.MinimapRightClickWithAlt.Name = "MinimapRightClickWithAlt";
            this.MinimapRightClickWithAlt.Ripple = true;
            this.MinimapRightClickWithAlt.Size = new System.Drawing.Size(248, 30);
            this.MinimapRightClickWithAlt.TabIndex = 41;
            this.MinimapRightClickWithAlt.Text = "Правый клик по миникарте с ALT";
            this.MinimapRightClickWithAlt.UseVisualStyleBackColor = false;
            this.MinimapRightClickWithAlt.CheckedChanged += new System.EventHandler(this.ShiftNumpadTrick_CheckedChanged);
            // 
            // ShiftNumpadTrick
            // 
            this.ShiftNumpadTrick.AutoSize = true;
            this.ShiftNumpadTrick.BackColor = System.Drawing.Color.Transparent;
            this.ShiftNumpadTrick.Checked = true;
            this.ShiftNumpadTrick.CheckState = System.Windows.Forms.CheckState.Checked;
            this.ShiftNumpadTrick.Depth = 0;
            this.ShiftNumpadTrick.Font = new System.Drawing.Font("Roboto", 10F);
            this.ShiftNumpadTrick.ForeColor = System.Drawing.SystemColors.ControlText;
            this.ShiftNumpadTrick.Location = new System.Drawing.Point(316, 107);
            this.ShiftNumpadTrick.Margin = new System.Windows.Forms.Padding(0);
            this.ShiftNumpadTrick.MouseLocation = new System.Drawing.Point(-1, -1);
            this.ShiftNumpadTrick.MouseState = MaterialSkin.MouseState.HOVER;
            this.ShiftNumpadTrick.Name = "ShiftNumpadTrick";
            this.ShiftNumpadTrick.Ripple = true;
            this.ShiftNumpadTrick.Size = new System.Drawing.Size(236, 30);
            this.ShiftNumpadTrick.TabIndex = 41;
            this.ShiftNumpadTrick.Text = "Исправить Shift+Numpad в игре";
            this.ShiftNumpadTrick.UseVisualStyleBackColor = false;
            this.ShiftNumpadTrick.CheckedChanged += new System.EventHandler(this.ShiftNumpadTrick_CheckedChanged);
            // 
            // UseCustomMpq
            // 
            this.UseCustomMpq.AutoSize = true;
            this.UseCustomMpq.BackColor = System.Drawing.Color.Transparent;
            this.UseCustomMpq.Checked = true;
            this.UseCustomMpq.CheckState = System.Windows.Forms.CheckState.Checked;
            this.UseCustomMpq.Depth = 0;
            this.UseCustomMpq.Font = new System.Drawing.Font("Roboto", 10F);
            this.UseCustomMpq.Location = new System.Drawing.Point(33, 143);
            this.UseCustomMpq.Margin = new System.Windows.Forms.Padding(0);
            this.UseCustomMpq.MouseLocation = new System.Drawing.Point(-1, -1);
            this.UseCustomMpq.MouseState = MaterialSkin.MouseState.HOVER;
            this.UseCustomMpq.Name = "UseCustomMpq";
            this.UseCustomMpq.Ripple = true;
            this.UseCustomMpq.Size = new System.Drawing.Size(203, 30);
            this.UseCustomMpq.TabIndex = 40;
            this.UseCustomMpq.Text = "Использовать custom.mpq";
            this.UseCustomMpq.UseVisualStyleBackColor = false;
            // 
            // WarkeyEnabled
            // 
            this.WarkeyEnabled.AutoSize = true;
            this.WarkeyEnabled.BackColor = System.Drawing.Color.Transparent;
            this.WarkeyEnabled.Checked = true;
            this.WarkeyEnabled.CheckState = System.Windows.Forms.CheckState.Checked;
            this.WarkeyEnabled.Depth = 0;
            this.WarkeyEnabled.Font = new System.Drawing.Font("Roboto", 10F);
            this.WarkeyEnabled.Location = new System.Drawing.Point(316, 70);
            this.WarkeyEnabled.Margin = new System.Windows.Forms.Padding(0);
            this.WarkeyEnabled.MouseLocation = new System.Drawing.Point(-1, -1);
            this.WarkeyEnabled.MouseState = MaterialSkin.MouseState.HOVER;
            this.WarkeyEnabled.Name = "WarkeyEnabled";
            this.WarkeyEnabled.Ripple = true;
            this.WarkeyEnabled.Size = new System.Drawing.Size(146, 30);
            this.WarkeyEnabled.TabIndex = 40;
            this.WarkeyEnabled.Text = "Включить WarKey";
            this.WarkeyEnabled.UseVisualStyleBackColor = false;
            this.WarkeyEnabled.CheckedChanged += new System.EventHandler(this.WarkeyEnabled_CheckedChanged);
            // 
            // WarKeyBtn
            // 
            this.WarKeyBtn.AutoSize = true;
            this.WarKeyBtn.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.WarKeyBtn.Depth = 0;
            this.WarKeyBtn.Icon = null;
            this.WarKeyBtn.Location = new System.Drawing.Point(492, 66);
            this.WarKeyBtn.MouseState = MaterialSkin.MouseState.HOVER;
            this.WarKeyBtn.Name = "WarKeyBtn";
            this.WarKeyBtn.Primary = true;
            this.WarKeyBtn.Size = new System.Drawing.Size(76, 36);
            this.WarKeyBtn.TabIndex = 39;
            this.WarKeyBtn.Text = "WarKey";
            this.WarKeyBtn.UseVisualStyleBackColor = true;
            this.WarKeyBtn.Click += new System.EventHandler(this.WarKeyBtn_Click);
            // 
            // SelectChatNameColor
            // 
            this.SelectChatNameColor.AutoSize = true;
            this.SelectChatNameColor.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.SelectChatNameColor.Depth = 0;
            this.SelectChatNameColor.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SelectChatNameColor.Icon = null;
            this.SelectChatNameColor.Location = new System.Drawing.Point(33, 104);
            this.SelectChatNameColor.MouseState = MaterialSkin.MouseState.HOVER;
            this.SelectChatNameColor.Name = "SelectChatNameColor";
            this.SelectChatNameColor.Primary = true;
            this.SelectChatNameColor.Size = new System.Drawing.Size(202, 36);
            this.SelectChatNameColor.TabIndex = 38;
            this.SelectChatNameColor.Text = "Select chat name color.";
            this.SelectChatNameColor.UseVisualStyleBackColor = true;
            this.SelectChatNameColor.Click += new System.EventHandler(this.SelectChatNameColor_Click);
            // 
            // SelectStatsLineColor
            // 
            this.SelectStatsLineColor.AutoSize = true;
            this.SelectStatsLineColor.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.SelectStatsLineColor.Depth = 0;
            this.SelectStatsLineColor.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SelectStatsLineColor.Icon = null;
            this.SelectStatsLineColor.Location = new System.Drawing.Point(316, 22);
            this.SelectStatsLineColor.MouseState = MaterialSkin.MouseState.HOVER;
            this.SelectStatsLineColor.Name = "SelectStatsLineColor";
            this.SelectStatsLineColor.Primary = true;
            this.SelectStatsLineColor.Size = new System.Drawing.Size(252, 36);
            this.SelectStatsLineColor.TabIndex = 37;
            this.SelectStatsLineColor.Text = "Настроить строку статистики";
            this.SelectStatsLineColor.UseVisualStyleBackColor = true;
            this.SelectStatsLineColor.Click += new System.EventHandler(this.SelectStatsLineColor_Click);
            // 
            // SelectChatColor
            // 
            this.SelectChatColor.AutoSize = true;
            this.SelectChatColor.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.SelectChatColor.Depth = 0;
            this.SelectChatColor.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SelectChatColor.Icon = null;
            this.SelectChatColor.Location = new System.Drawing.Point(33, 22);
            this.SelectChatColor.MouseState = MaterialSkin.MouseState.HOVER;
            this.SelectChatColor.Name = "SelectChatColor";
            this.SelectChatColor.Primary = true;
            this.SelectChatColor.Size = new System.Drawing.Size(222, 36);
            this.SelectChatColor.TabIndex = 36;
            this.SelectChatColor.Text = "Select chat message color";
            this.SelectChatColor.UseVisualStyleBackColor = true;
            this.SelectChatColor.Click += new System.EventHandler(this.SelectChatColor_Click);
            // 
            // SelectAccountColor
            // 
            this.SelectAccountColor.AutoSize = true;
            this.SelectAccountColor.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.SelectAccountColor.Depth = 0;
            this.SelectAccountColor.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.SelectAccountColor.Icon = null;
            this.SelectAccountColor.Location = new System.Drawing.Point(33, 62);
            this.SelectAccountColor.MouseState = MaterialSkin.MouseState.HOVER;
            this.SelectAccountColor.Name = "SelectAccountColor";
            this.SelectAccountColor.Primary = true;
            this.SelectAccountColor.Size = new System.Drawing.Size(205, 36);
            this.SelectAccountColor.TabIndex = 35;
            this.SelectAccountColor.Text = "Select lobby name color";
            this.SelectAccountColor.UseVisualStyleBackColor = true;
            this.SelectAccountColor.Click += new System.EventHandler(this.SelectAccountColor_Click);
            // 
            // LauncherSettings
            // 
            this.LauncherSettings.BackColor = System.Drawing.Color.WhiteSmoke;
            this.LauncherSettings.Controls.Add(this.LangLabel);
            this.LauncherSettings.Controls.Add(this.ChangeStyleBtn);
            this.LauncherSettings.Controls.Add(this.ChangeThemeBtn);
            this.LauncherSettings.Controls.Add(this.LauncherLanguage);
            this.LauncherSettings.Location = new System.Drawing.Point(4, 22);
            this.LauncherSettings.Name = "LauncherSettings";
            this.LauncherSettings.Padding = new System.Windows.Forms.Padding(3);
            this.LauncherSettings.Size = new System.Drawing.Size(631, 271);
            this.LauncherSettings.TabIndex = 1;
            this.LauncherSettings.Text = "Warcis Client";
            // 
            // LangLabel
            // 
            this.LangLabel.AutoSize = true;
            this.LangLabel.Depth = 0;
            this.LangLabel.Font = new System.Drawing.Font("Roboto", 11F);
            this.LangLabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.LangLabel.Location = new System.Drawing.Point(128, 27);
            this.LangLabel.MouseState = MaterialSkin.MouseState.HOVER;
            this.LangLabel.Name = "LangLabel";
            this.LangLabel.Size = new System.Drawing.Size(51, 19);
            this.LangLabel.TabIndex = 33;
            this.LangLabel.Text = "Язык:";
            // 
            // ChangeStyleBtn
            // 
            this.ChangeStyleBtn.AutoSize = true;
            this.ChangeStyleBtn.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ChangeStyleBtn.Depth = 0;
            this.ChangeStyleBtn.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.ChangeStyleBtn.Icon = null;
            this.ChangeStyleBtn.Location = new System.Drawing.Point(126, 97);
            this.ChangeStyleBtn.MouseState = MaterialSkin.MouseState.HOVER;
            this.ChangeStyleBtn.Name = "ChangeStyleBtn";
            this.ChangeStyleBtn.Primary = true;
            this.ChangeStyleBtn.Size = new System.Drawing.Size(119, 36);
            this.ChangeStyleBtn.TabIndex = 31;
            this.ChangeStyleBtn.Text = "Change style";
            this.ChangeStyleBtn.UseVisualStyleBackColor = true;
            this.ChangeStyleBtn.Click += new System.EventHandler(this.ChangeStyleBtn_Click);
            // 
            // ChangeThemeBtn
            // 
            this.ChangeThemeBtn.AutoSize = true;
            this.ChangeThemeBtn.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ChangeThemeBtn.Depth = 0;
            this.ChangeThemeBtn.Font = new System.Drawing.Font("Microsoft Sans Serif", 7F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.ChangeThemeBtn.Icon = null;
            this.ChangeThemeBtn.Location = new System.Drawing.Point(126, 55);
            this.ChangeThemeBtn.MouseState = MaterialSkin.MouseState.HOVER;
            this.ChangeThemeBtn.Name = "ChangeThemeBtn";
            this.ChangeThemeBtn.Primary = true;
            this.ChangeThemeBtn.Size = new System.Drawing.Size(107, 36);
            this.ChangeThemeBtn.TabIndex = 32;
            this.ChangeThemeBtn.Text = "Dark / Light";
            this.ChangeThemeBtn.UseVisualStyleBackColor = true;
            this.ChangeThemeBtn.Click += new System.EventHandler(this.ChangeThemeBtn_Click);
            // 
            // LauncherLanguage
            // 
            this.LauncherLanguage.FormattingEnabled = true;
            this.LauncherLanguage.Items.AddRange(new object[] {
            "Русский",
            "English"});
            this.LauncherLanguage.Location = new System.Drawing.Point(185, 28);
            this.LauncherLanguage.Name = "LauncherLanguage";
            this.LauncherLanguage.Size = new System.Drawing.Size(121, 21);
            this.LauncherLanguage.TabIndex = 30;
            this.LauncherLanguage.Text = "Русский";
            this.LauncherLanguage.SelectedIndexChanged += new System.EventHandler(this.LauncherLanguage_SelectedIndexChanged);
            // 
            // HelpPage
            // 
            this.HelpPage.BackColor = System.Drawing.Color.LightGray;
            this.HelpPage.Controls.Add(this.materialLabel3);
            this.HelpPage.Controls.Add(this.materialLabel4);
            this.HelpPage.Controls.Add(this.label1);
            this.HelpPage.Controls.Add(this.VeryGoodInfo);
            this.HelpPage.Location = new System.Drawing.Point(4, 22);
            this.HelpPage.Name = "HelpPage";
            this.HelpPage.Size = new System.Drawing.Size(632, 344);
            this.HelpPage.TabIndex = 2;
            this.HelpPage.Text = "HELP";
            // 
            // materialLabel3
            // 
            this.materialLabel3.AutoSize = true;
            this.materialLabel3.Depth = 0;
            this.materialLabel3.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel3.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel3.Location = new System.Drawing.Point(96, 177);
            this.materialLabel3.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel3.Name = "materialLabel3";
            this.materialLabel3.Size = new System.Drawing.Size(437, 19);
            this.materialLabel3.TabIndex = 17;
            this.materialLabel3.Text = "Press RightCTRL+Enter for enable/disable  fix for Field Of View !";
            // 
            // materialLabel4
            // 
            this.materialLabel4.AutoSize = true;
            this.materialLabel4.Depth = 0;
            this.materialLabel4.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel4.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel4.Location = new System.Drawing.Point(96, 118);
            this.materialLabel4.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel4.Name = "materialLabel4";
            this.materialLabel4.Size = new System.Drawing.Size(351, 19);
            this.materialLabel4.TabIndex = 18;
            this.materialLabel4.Text = "Press and hold ~ to use Voice Chat in Lobby/Game";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Depth = 0;
            this.label1.Font = new System.Drawing.Font("Roboto", 11F);
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.label1.Location = new System.Drawing.Point(96, 147);
            this.label1.MouseState = MaterialSkin.MouseState.HOVER;
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(386, 19);
            this.label1.TabIndex = 18;
            this.label1.Text = "Press RightAlt+Enter for toggle window/fullscreen mode";
            // 
            // VeryGoodInfo
            // 
            this.VeryGoodInfo.AutoSize = true;
            this.VeryGoodInfo.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.VeryGoodInfo.Depth = 0;
            this.VeryGoodInfo.Icon = null;
            this.VeryGoodInfo.Location = new System.Drawing.Point(24, 306);
            this.VeryGoodInfo.MouseState = MaterialSkin.MouseState.HOVER;
            this.VeryGoodInfo.Name = "VeryGoodInfo";
            this.VeryGoodInfo.Primary = true;
            this.VeryGoodInfo.Size = new System.Drawing.Size(132, 36);
            this.VeryGoodInfo.TabIndex = 16;
            this.VeryGoodInfo.Text = "Показать пути";
            this.VeryGoodInfo.UseVisualStyleBackColor = true;
            this.VeryGoodInfo.Click += new System.EventHandler(this.VeryGoodInfo_Click);
            // 
            // NewsPage
            // 
            this.NewsPage.Controls.Add(this.WarcisNews);
            this.NewsPage.Location = new System.Drawing.Point(4, 22);
            this.NewsPage.Name = "NewsPage";
            this.NewsPage.Size = new System.Drawing.Size(632, 344);
            this.NewsPage.TabIndex = 3;
            this.NewsPage.Text = "NEWS";
            this.NewsPage.UseVisualStyleBackColor = true;
            // 
            // WarcisNews
            // 
            this.WarcisNews.AllowNavigation = false;
            this.WarcisNews.Dock = System.Windows.Forms.DockStyle.Fill;
            this.WarcisNews.IsWebBrowserContextMenuEnabled = false;
            this.WarcisNews.Location = new System.Drawing.Point(0, 0);
            this.WarcisNews.MinimumSize = new System.Drawing.Size(20, 20);
            this.WarcisNews.Name = "WarcisNews";
            this.WarcisNews.Size = new System.Drawing.Size(632, 344);
            this.WarcisNews.TabIndex = 0;
            this.WarcisNews.Url = new System.Uri("https://forum.warcis.com/threads/14043/", System.UriKind.Absolute);
            // 
            // MapUploader
            // 
            this.MapUploader.Controls.Add(this.MapUploadPage);
            this.MapUploader.Location = new System.Drawing.Point(4, 22);
            this.MapUploader.Name = "MapUploader";
            this.MapUploader.Size = new System.Drawing.Size(632, 344);
            this.MapUploader.TabIndex = 4;
            this.MapUploader.Text = "Upload map";
            this.MapUploader.UseVisualStyleBackColor = true;
            // 
            // MapUploadPage
            // 
            this.MapUploadPage.AllowNavigation = false;
            this.MapUploadPage.Dock = System.Windows.Forms.DockStyle.Fill;
            this.MapUploadPage.IsWebBrowserContextMenuEnabled = false;
            this.MapUploadPage.Location = new System.Drawing.Point(0, 0);
            this.MapUploadPage.MinimumSize = new System.Drawing.Size(20, 20);
            this.MapUploadPage.Name = "MapUploadPage";
            this.MapUploadPage.ScrollBarsEnabled = false;
            this.MapUploadPage.Size = new System.Drawing.Size(632, 344);
            this.MapUploadPage.TabIndex = 0;
            this.MapUploadPage.Url = new System.Uri("http://193.19.118.57/mapuploader/", System.UriKind.Absolute);
            // 
            // materialTabSelector1
            // 
            this.materialTabSelector1.BackColor = System.Drawing.SystemColors.Control;
            this.materialTabSelector1.BaseTabControl = this.LauncherTabPages;
            this.materialTabSelector1.Depth = 0;
            this.materialTabSelector1.Location = new System.Drawing.Point(0, 64);
            this.materialTabSelector1.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialTabSelector1.Name = "materialTabSelector1";
            this.materialTabSelector1.Size = new System.Drawing.Size(640, 47);
            this.materialTabSelector1.TabIndex = 21;
            this.materialTabSelector1.Text = "SelectLauncherPage";
            // 
            // AudioSettings
            // 
            this.AudioSettings.BackColor = System.Drawing.Color.White;
            this.AudioSettings.Controls.Add(this.MicQuality);
            this.AudioSettings.Controls.Add(this.materialLabel7);
            this.AudioSettings.Controls.Add(this.EnableMicInput);
            this.AudioSettings.Controls.Add(this.EnableVoiceChat);
            this.AudioSettings.Location = new System.Drawing.Point(4, 22);
            this.AudioSettings.Name = "AudioSettings";
            this.AudioSettings.Padding = new System.Windows.Forms.Padding(3);
            this.AudioSettings.Size = new System.Drawing.Size(631, 271);
            this.AudioSettings.TabIndex = 3;
            this.AudioSettings.Text = "Audio";
            // 
            // EnableVoiceChat
            // 
            this.EnableVoiceChat.AutoSize = true;
            this.EnableVoiceChat.Checked = true;
            this.EnableVoiceChat.CheckState = System.Windows.Forms.CheckState.Checked;
            this.EnableVoiceChat.Depth = 0;
            this.EnableVoiceChat.Font = new System.Drawing.Font("Roboto", 10F);
            this.EnableVoiceChat.Location = new System.Drawing.Point(26, 19);
            this.EnableVoiceChat.Margin = new System.Windows.Forms.Padding(0);
            this.EnableVoiceChat.MouseLocation = new System.Drawing.Point(-1, -1);
            this.EnableVoiceChat.MouseState = MaterialSkin.MouseState.HOVER;
            this.EnableVoiceChat.Name = "EnableVoiceChat";
            this.EnableVoiceChat.Ripple = true;
            this.EnableVoiceChat.Size = new System.Drawing.Size(192, 30);
            this.EnableVoiceChat.TabIndex = 4;
            this.EnableVoiceChat.Text = "Включить голосовой чат";
            this.EnableVoiceChat.UseVisualStyleBackColor = true;
            // 
            // EnableMicInput
            // 
            this.EnableMicInput.AutoSize = true;
            this.EnableMicInput.Checked = true;
            this.EnableMicInput.CheckState = System.Windows.Forms.CheckState.Checked;
            this.EnableMicInput.Depth = 0;
            this.EnableMicInput.Font = new System.Drawing.Font("Roboto", 10F);
            this.EnableMicInput.Location = new System.Drawing.Point(272, 19);
            this.EnableMicInput.Margin = new System.Windows.Forms.Padding(0);
            this.EnableMicInput.MouseLocation = new System.Drawing.Point(-1, -1);
            this.EnableMicInput.MouseState = MaterialSkin.MouseState.HOVER;
            this.EnableMicInput.Name = "EnableMicInput";
            this.EnableMicInput.Ripple = true;
            this.EnableMicInput.Size = new System.Drawing.Size(309, 30);
            this.EnableMicInput.TabIndex = 4;
            this.EnableMicInput.Text = "Разговаривать при включенном Caps Lock";
            this.EnableMicInput.UseVisualStyleBackColor = true;
            // 
            // materialLabel7
            // 
            this.materialLabel7.AutoSize = true;
            this.materialLabel7.Depth = 0;
            this.materialLabel7.Font = new System.Drawing.Font("Roboto", 11F);
            this.materialLabel7.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(222)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.materialLabel7.Location = new System.Drawing.Point(33, 67);
            this.materialLabel7.MouseState = MaterialSkin.MouseState.HOVER;
            this.materialLabel7.Name = "materialLabel7";
            this.materialLabel7.Size = new System.Drawing.Size(166, 19);
            this.materialLabel7.TabIndex = 20;
            this.materialLabel7.Text = "Качество микрофона:";
            // 
            // MicQuality
            // 
            this.MicQuality.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.MicQuality.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.MicQuality.FormattingEnabled = true;
            this.MicQuality.Items.AddRange(new object[] {
            "Низкое",
            "Среднее"});
            this.MicQuality.Location = new System.Drawing.Point(216, 65);
            this.MicQuality.Name = "MicQuality";
            this.MicQuality.Size = new System.Drawing.Size(138, 21);
            this.MicQuality.TabIndex = 21;
            // 
            // Launcher
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(640, 480);
            this.Controls.Add(this.materialTabSelector1);
            this.Controls.Add(this.LauncherTabPages);
            this.MaximizeBox = false;
            this.Name = "Launcher";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Тестовый Warcis launcher для запуска игры";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Launcher_FormClosed);
            this.Load += new System.EventHandler(this.Launcher_Load);
            this.LauncherTabPages.ResumeLayout(false);
            this.GeneralPage.ResumeLayout(false);
            this.GeneralPage.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.WarcisPicture)).EndInit();
            this.SettingsPage.ResumeLayout(false);
            this.SettingsPageTab.ResumeLayout(false);
            this.VideoSettings.ResumeLayout(false);
            this.VideoSettings.PerformLayout();
            this.UserCustomizeSettings.ResumeLayout(false);
            this.UserCustomizeSettings.PerformLayout();
            this.LauncherSettings.ResumeLayout(false);
            this.LauncherSettings.PerformLayout();
            this.HelpPage.ResumeLayout(false);
            this.HelpPage.PerformLayout();
            this.NewsPage.ResumeLayout(false);
            this.MapUploader.ResumeLayout(false);
            this.AudioSettings.ResumeLayout(false);
            this.AudioSettings.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.Timer TimedWorker;
        private System.Windows.Forms.ColorDialog SelectAccountColorDialog;
        private MaterialSkin.Controls.MaterialTabControl LauncherTabPages;
        private System.Windows.Forms.TabPage SettingsPage;
        private System.Windows.Forms.TabPage HelpPage;
        private System.Windows.Forms.TabPage NewsPage;
        private MaterialSkin.Controls.MaterialTabSelector materialTabSelector1;
        private MaterialSkin.Controls.MaterialRaisedButton VeryGoodInfo;
        private System.Windows.Forms.TabPage GeneralPage;
        private MaterialSkin.Controls.MaterialRaisedButton War3Button;
        private MaterialSkin.Controls.MaterialRaisedButton ChangeWar3Path;
        private MaterialSkin.Controls.MaterialCheckBox AutoLoginCheckBox;
        private MaterialSkin.Controls.MaterialSingleLineTextField AutologinUsername;
        private MaterialSkin.Controls.MaterialSingleLineTextField AutologinPassword;
        private MaterialSkin.Controls.MaterialCheckBox InWindowMode;
        private MaterialSkin.Controls.MaterialCheckBox UseFullScrenSwitcher;
        private MaterialSkin.Controls.MaterialCheckBox UseNewD3dMode;
        private MaterialSkin.Controls.MaterialCheckBox ActivateOpenglMode;
        private MaterialSkin.Controls.MaterialCheckBox GrayScaleWorld;
        private System.Windows.Forms.WebBrowser WarcisNews;
        private System.Windows.Forms.TabPage MapUploader;
        private System.Windows.Forms.WebBrowser MapUploadPage;
        private MaterialSkin.Controls.MaterialLabel materialLabel2;
        private MaterialSkin.Controls.MaterialLabel materialLabel1;
        private MaterialSkin.Controls.MaterialLabel materialLabel3;
        private MaterialSkin.Controls.MaterialLabel materialLabel4;
        private MaterialSkin.Controls.MaterialLabel label1;
        private System.Windows.Forms.PictureBox WarcisPicture;
        private MaterialSkin.Controls.MaterialCheckBox EnableVSYNC;
        private MaterialSkin.Controls.MaterialTabSelector SettingsPageTabSelector;
        private MaterialSkin.Controls.MaterialTabControl SettingsPageTab;
        private System.Windows.Forms.TabPage VideoSettings;
        private System.Windows.Forms.TabPage UserCustomizeSettings;
        private MaterialSkin.Controls.MaterialCheckBox ShiftNumpadTrick;
        private MaterialSkin.Controls.MaterialCheckBox WarkeyEnabled;
        private MaterialSkin.Controls.MaterialRaisedButton WarKeyBtn;
        private MaterialSkin.Controls.MaterialRaisedButton SelectChatNameColor;
        private MaterialSkin.Controls.MaterialRaisedButton SelectStatsLineColor;
        private MaterialSkin.Controls.MaterialRaisedButton SelectChatColor;
        private MaterialSkin.Controls.MaterialRaisedButton SelectAccountColor;
        private System.Windows.Forms.TabPage LauncherSettings;
        private MaterialSkin.Controls.MaterialLabel LangLabel;
        private MaterialSkin.Controls.MaterialRaisedButton ChangeStyleBtn;
        private MaterialSkin.Controls.MaterialRaisedButton ChangeThemeBtn;
        private System.Windows.Forms.ComboBox LauncherLanguage;
        private MaterialSkin.Controls.MaterialLabel FPSLabel;
        private System.Windows.Forms.ComboBox GlobalColorEffect;
        private System.Windows.Forms.ComboBox AvailableFpsList;
        private MaterialSkin.Controls.MaterialCheckBox BugReportWithoutUser;
        private MaterialSkin.Controls.MaterialCheckBox NeedClipCursor;
        private MaterialSkin.Controls.MaterialCheckBox UseDefaultErrorCatch;
        private MaterialSkin.Controls.MaterialRaisedButton War3Download;
        private MaterialSkin.Controls.MaterialCheckBox MinimapRightClickWithAlt;
        private MaterialSkin.Controls.MaterialCheckBox DownloadRussianVersion;
        private MaterialSkin.Controls.MaterialCheckBox UseCustomMpq;
        private System.Windows.Forms.Panel panel1;
        private MaterialSkin.Controls.MaterialSingleLineTextField MaxPreloadTime;
        private MaterialSkin.Controls.MaterialLabel materialLabel6;
        private MaterialSkin.Controls.MaterialLabel materialLabel5;
        private System.Windows.Forms.TabPage AudioSettings;
        private System.Windows.Forms.ComboBox MicQuality;
        private MaterialSkin.Controls.MaterialLabel materialLabel7;
        private MaterialSkin.Controls.MaterialCheckBox EnableMicInput;
        private MaterialSkin.Controls.MaterialCheckBox EnableVoiceChat;
    }
}