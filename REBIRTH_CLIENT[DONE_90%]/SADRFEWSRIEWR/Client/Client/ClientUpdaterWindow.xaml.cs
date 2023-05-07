using FluentFTP;

using System;
using System.Collections.Generic;
using System.Linq;
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
using System.Windows.Threading;
using static Client.GlobalFunctions;
namespace Client
{
    /// <summary>
    /// Логика взаимодействия для ClientUpdaterWindow.xaml
    /// </summary>
    public partial class ClientUpdaterWindow
    {
        public Thread updatethread = null;

        string[] udpatefiles = new string[0] { };
        string[] udpatefilesserver = new string[0] { };
        FtpClient ftpClient = null;
        public ClientUpdaterWindow(FtpClient ftpClient, string[] udpatefiles, string[] udpatefilesserver)
        {
            InitializeComponent();
            UpdateLanguageForCurrentWindow();
            this.udpatefiles = udpatefiles;
            this.udpatefilesserver = udpatefilesserver;
            this.ftpClient = ftpClient;
            updatethread = new Thread(UpdateThread);
            updatethread.IsBackground = true;
            updatethread.Start();
        }

        public void UpdateThread()
        {
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
            Thread.Sleep(500);
            for (int i = 0; i < udpatefiles.Length; i++)
            {
                try
                {
                    Application.Current.Dispatcher.Invoke(new Action(() => ClientUpdaterWindowLoadFile.Content = String.Format(GetLocalizedString("ClientUpdaterWindowLoadFile"), udpatefiles[i])));
                    Application.Current.Dispatcher.Invoke(new Action(() => ClientUpdaterWindowCountFiles.Content = String.Format(GetLocalizedString("ClientUpdaterWindowCountFiles"), i + 1, udpatefilesserver.Length)));
                }
                catch { }
                try
                {
                    ftpClient.DownloadFile(udpatefiles[i] + ".upd", udpatefilesserver[i], FtpLocalExists.Overwrite , FtpVerify.None, progress);
                }
                catch
                {
                    try
                    {
                        Application.Current.Dispatcher.Invoke(new Action(() =>
                        DialogResult = false));
                    }
                    catch
                    {

                    }
                }
            }

            try
            {
                Application.Current.Dispatcher.Invoke(new Action(() =>
                 DialogResult = true));
            }
            catch
            {

            }
        }

        public void UpdateLanguageForCurrentWindow()
        {
            PlatformLabel.Content = GetLocalizedString("PlatformLabel");

            ClientUpdaterWindowCountFiles.Content = String.Format(GetLocalizedString("ClientUpdaterWindowCountFiles"), 0, udpatefilesserver.Length);
        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {
            updatethread.Interrupt();
            updatethread.Abort();

            ftpClient.Disconnect();
            try
            {
                DialogResult = false;
            }
            catch
            {

            }
        }

        private void progressBar_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {

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

