using FluentFTP;

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

using System.Diagnostics;
using System.Diagnostics.Contracts;

using static Client.GlobalFunctions;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для MapDownloadWindow.xaml
    /// </summary>
    public partial class MapDownloadWindow
    {
        string MapFileName = "";
        string MapOutFileName = "";
        public bool DownloadStart = false;
        public bool DownloadSuccess = false;
        FtpClient MapFtpAccount = new FtpClient(GetStringByRegion("MapDownloaderFtp"), 21, new NetworkCredential("ftpmaps", "ftpmaps"));


        public MapDownloadWindow(string mapfile, string mapout, string mapname)
        {
            InitializeComponent();
            MapFileName = "/" + mapfile;
            MapDownloadLabel.Text = "Download map : " + mapname;
            MapOutFileName = mapout;
            DownloadSuccess = false;
            try
            {
                MapFtpAccount.Connect();
            }
            catch
            {
                MessageBox.Show("No access to map download server!");
                this.Close();
            }

            long filesize = MapFtpAccount.GetFileSize(MapFileName) / 1024;
            if (filesize / 1024 <= 1)
            {
                MapDownloadLabel.Text += ".\nFile size: " + filesize + " KBytes";
            }
            else
            {
                MapDownloadLabel.Text += ".\nFile size: " + (filesize / 1024) + "MB";
            }
           
        }

        private void DownloadThread()
        {
            // create an FTP client
            
            //client.Credentials = new NetworkCredential("mapdata", "QQVw2MRg0dzoJIdzMreVILo7I");

            // begin connecting to the server
          
            Action<FtpProgress> progress = new Action<FtpProgress>(x => {

                Application.Current.Dispatcher.Invoke(new Action(() =>
                {
                    // When progress in unknown, -1 will be sent
                    if (x.Progress < 0)
                    {

                    }
                    else
                    {
                        progressBar.Value = x.Progress;
                        progressBar.UpdateLayout();
                    }
                }));
            });


            try
            {
                File.Delete(MapOutFileName + ".tmp");
            }
            catch
            {

            }

            try
            {
                File.Delete(MapOutFileName);
            }
            catch
            {

            }

            try
            {
                MapFtpAccount.DownloadFile(MapOutFileName + ".tmp", MapFileName, FtpLocalExists.Overwrite, FtpVerify.None, progress);
                DownloadSuccess = true;
                File.Move(MapOutFileName + ".tmp", MapOutFileName);
            }
            catch
            {
                MessageBox.Show("Error no access to out file:" + MapOutFileName + ".tmp");
            }

            Application.Current.Dispatcher.Invoke(new Action(() => Close()));
        }
        Thread mDownloadThread = null;

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (!DownloadStart)
            {
                CancelButton.IsEnabled = false;
                DownloadButton.IsEnabled = false;
                DownloadStart = true;
                mDownloadThread = new Thread(DownloadThread);
                mDownloadThread.IsBackground = true;
                mDownloadThread.Start();
            }


        }

        private void Button2_Click(object sender, RoutedEventArgs e)
        {
           
            // remove exist crcmiss, remove map, move map to crcmiss

            try
            {
                File.Delete(MapOutFileName + ".crcmiss");
            }
            catch
            {

            }

            try
            {
                File.Delete(MapOutFileName);
            }
            catch
            {

            }

            if (File.Exists(MapOutFileName))
            {
                try
                {
                    File.Move(MapOutFileName, MapOutFileName + ".crcmiss");
                }
                catch
                {

                }
            }

            this.Close();
        }


        private void Window_Closed(object sender, EventArgs e)
        {
            try
            {
                MapFtpAccount.Disconnect();
            }
            catch
            { }
            try
            {
                MapFtpAccount.Dispose();
            }
            catch
            { }
            MapFtpAccount = null;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Application curApp = Application.Current;
            Window mainWindow = curApp.MainWindow;
            this.Left = mainWindow.Left + (mainWindow.Width - this.ActualWidth) / 2;
            this.Top = mainWindow.Top + (mainWindow.Height - this.ActualHeight) / 2;
        }
    }


}
