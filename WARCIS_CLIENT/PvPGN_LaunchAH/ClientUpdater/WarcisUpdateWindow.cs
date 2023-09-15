using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Net;
using System.Threading;

namespace ClientUpdater
{
    public partial class WarcisUpdateWindow : Form
    {
        private ProgressBar FullDownloadProgress;

        public WarcisUpdateWindow()
        {
            // 
            // FullDownloadProgress
            // 

            foreach (string file in Directory.GetFiles(Directory.GetCurrentDirectory(), "*.del").Where(item => item.EndsWith(".del")))
            {
                try
                {
                    File.Delete(file);
                }
                catch
                {
                    File.Move(file, file + ".del");
                }
            }



            InitializeComponent();

            this.FullDownloadProgress = new NewProgressBar();
            this.FullDownloadProgress.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.FullDownloadProgress.Cursor = System.Windows.Forms.Cursors.Default;
            this.FullDownloadProgress.Location = new System.Drawing.Point(12, 140);
            this.FullDownloadProgress.MarqueeAnimationSpeed = 50;
            this.FullDownloadProgress.Name = "FullDownloadProgress";
            this.FullDownloadProgress.Size = new System.Drawing.Size(283, 23);
            this.FullDownloadProgress.Step = 5;
            this.FullDownloadProgress.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.FullDownloadProgress.TabIndex = 1;
            this.FullDownloadProgress.Visible = true;
            this.FullDownloadProgress.Enabled = true;
            this.Controls.Add(this.FullDownloadProgress);

        }

        bool StartBatchFile()
        {
            if (BatchFileFixForWindowsXP.Count > 0)
            {
                BatchFileFixForWindowsXP.Add("start ClientUpdater.exe");
                BatchFileFixForWindowsXP.Add("del UpdaterAutoFix.bat");
                BatchFileFixForWindowsXP.Add("exit UpdaterAutoFix.bat");
                
                File.WriteAllLines("UpdaterAutoFix.bat", BatchFileFixForWindowsXP.ToArray());
                System.Diagnostics.Process.Start("UpdaterAutoFix.bat");
                return true;
            }
            return false;
        }

        void StartLauncherAndTerminateUpdater()
        {
            try
            {
                if (!StartBatchFile())
                    Process.Start("Launcher.exe", "-noupdate");
            }
            catch
            {
                MessageBox.Show("Невозможно запустить файл Launcher.exe\nубедитесь что вы не додумались запустить ClientUpdater через архив!", "ERROR!");
            }
            Application.Exit();
        }

        private void CancelUpdateBtn_Click(object sender, EventArgs e)
        {
            try
            {
                myWebClient.CancelAsync();
            }
            catch
            {

            }
            if (Directory.Exists(Program.UpdateDirPath))
            {
                try
                {
                    Directory.Delete(Program.UpdateDirPath, true);
                }
                catch
                {
                    MessageBox.Show("Update error. Can not remove temp dir (UpdateDir)!");
                }
            }

            StartLauncherAndTerminateUpdater();
        }

        string GetMd5HashFile(string filename)
        {
            try
            {
                using (var md5 = MD5.Create())
                {
                    using (var stream = File.OpenRead(filename))
                    {
                        return BitConverter.ToString(md5.ComputeHash(stream)).Replace("-", "‌​").ToLower();
                    }
                }
            }
            catch
            {

            }
            return "0";
        }

        const string GetFileAndHash = @"^(.*?),(.*?)$";

        void DownloadProgressCallback(object sender, DownloadProgressChangedEventArgs e)
        {
            FullDownloadProgress.Value = e.ProgressPercentage;

            FullDownloadProgress.Refresh();
        }

        List<string> FilesForDownload = new List<string>();
        List<string> dwnlistfiles = new List<string>();


        bool StartDownload = false;

        public AsyncCompletedEventHandler DownloadFileCompleted()
        {

            Action<object, AsyncCompletedEventArgs> action = (sender, e) =>
            {
                StartDownload = true;
                if (e.Error != null)
                {
                    if (!e.Cancelled)
                        MessageBox.Show("Update problem #2");
                    else
                        MessageBox.Show("Update canceled");
                    StartDownload = false;
                    StartLauncherAndTerminateUpdater();
                    return;
                }

                 FilesForDownload.RemoveAt(0);

                downloadedfile = string.Empty;
            };



            return new AsyncCompletedEventHandler(action);
        }


        string FileListPath = string.Empty;
        string downloadedfile = string.Empty;

        WebClient myWebClient = new WebClient();

        private void UpdateWindow_Load(object sender, EventArgs e)
        {

            myWebClient.Proxy = new WebProxy();

            if (Directory.Exists(Program.UpdateDirPath))
            {
                try
                {
                    Directory.Delete(Program.UpdateDirPath, true);
                }
                catch
                {
                    MessageBox.Show("Update error. Can not remove temp dir (UpdateDir)!");
                }
            }

            FileListPath = Program.UpdateDirPath + "\\filelist.txt";
            downloadedfile = Program.UpdateDirPath + "\\filelist.txt";

            if (!Directory.Exists(Program.UpdateDirPath))
                Directory.CreateDirectory(Program.UpdateDirPath);
            myWebClient.DownloadProgressChanged += new DownloadProgressChangedEventHandler(DownloadProgressCallback);
            myWebClient.DownloadFileCompleted += DownloadFileCompleted();
            int errorid = 0;
            try
            {
                errorid = 1;
                myWebClient.DownloadFile(new Uri(Program.UpdateServer + "filelist.txt"), FileListPath);

                errorid = 2;
                string[] FileListRead = File.ReadAllLines(downloadedfile);
                errorid = 3;

                foreach (string s in FileListRead)
                {
                    Match getfileandmd5 = Regex.Match(s, GetFileAndHash);
                    if (getfileandmd5.Success)
                    {
                        string FileName = getfileandmd5.Groups[1].Value;
                        string FileMd5 = getfileandmd5.Groups[2].Value;
                        if (FileMd5 != GetMd5HashFile(FileName))
                        {
                            FilesForDownload.Add(FileName);
                            dwnlistfiles.Add(FileName);
                        }
                        if (File.Exists("updatefilelist.txt"))
                        {
                            try
                            {
                                File.AppendAllText("updatefilelist.txt", FileName + "," + GetMd5HashFile(FileName) + "\n");
                            }
                            catch
                            {

                            }
                        }

                    }
                }
                errorid = 4;

                UpdateListFiles.Enabled = true;
                downloadedfile = string.Empty;
                UpdateListFiles.Start();

                errorid = 5;
            }
            catch
            {
                MessageBox.Show("Update problem #1 (" + errorid + ")");
                StartLauncherAndTerminateUpdater();
            }
        }

        List<string> BatchFileFixForWindowsXP = new List<string>();

        void AddNewMoveAction(string fileforupdate)
        {
            if (BatchFileFixForWindowsXP.Count == 0)
            {
                BatchFileFixForWindowsXP.Add("PING 1.1.1.1 -n 1 -w 3000 >NUL");
            }
            BatchFileFixForWindowsXP.Add("del " + fileforupdate);
            BatchFileFixForWindowsXP.Add("move " + fileforupdate + " " + fileforupdate + ".del");
            BatchFileFixForWindowsXP.Add("move " + Program.UpdateDirPath + "\\" + fileforupdate + " " + fileforupdate);
        }

        private void UpdateListFiles_Tick(object sender, EventArgs e)
        {
            ListFilesForDownload.BeginUpdate();
            ListFilesForDownload.Items.Clear();
            ListFilesForDownload.Items.AddRange(FilesForDownload.ToArray());
            ListFilesForDownload.EndUpdate();

            if (downloadedfile == string.Empty && FilesForDownload.Count > 0)
            {
                string s = FilesForDownload[0];
                downloadedfile = Program.UpdateDirPath + "\\" + s;
                myWebClient.DownloadFileAsync(new Uri(Program.UpdateServer + s), downloadedfile);
            }
            else if (downloadedfile == string.Empty)
            {
                foreach (string s in dwnlistfiles)
                {
                    if (File.Exists(s))
                    {
                        try
                        {
                            File.Delete(s);
                        }
                        catch
                        {
                            try
                            {
                                File.Move(s, s + ".del");
                            }
                            catch
                            {
                                AddNewMoveAction(s);
                            }
                        }
                    }
                    try
                    {
                        if (!File.Exists(s) && File.Exists(Program.UpdateDirPath + "\\" + s))
                        {
                            File.Copy(Program.UpdateDirPath + "\\" + s, s);
                        }
                    }
                    catch
                    {

                    }
                }
                UpdateListFiles.Enabled = false;
                StartLauncherAndTerminateUpdater();
            }
        }
    }

    public class NewProgressBar : ProgressBar
    {
        public NewProgressBar()
        {
            this.SetStyle(ControlStyles.UserPaint, true);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            Rectangle rec = e.ClipRectangle;

            rec.Width = (int)(rec.Width * ((double)Value / Maximum)) - 4;
            if (ProgressBarRenderer.IsSupported)
                ProgressBarRenderer.DrawHorizontalBar(e.Graphics, e.ClipRectangle);
            rec.Height = rec.Height - 4;
            e.Graphics.FillRectangle(Brushes.DarkBlue, 2, 2, rec.Width, rec.Height);
        }
    }
}
