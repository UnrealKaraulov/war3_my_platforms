
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.IO;
using System.Windows.Forms;
using static Client.GlobalVariables;
using static Client.GlobalFunctions;
namespace Client
{
    /// <summary>
    /// Логика взаимодействия для War3ProxySettings.xaml
    /// </summary>
    public partial class War3ProxySettings
    {
        public War3ProxySettings()
        {
            InitializeComponent();
            WindowModeCheckBox.IsChecked = GlobalConfiguration["General"]["WindowModeCheck"].ToBool();
            VideoDriver.SelectedIndex = GlobalConfiguration["General"]["VideoDriver"].ToInt();
            War3PathTextBox.Text = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false);
            ServerTextBox.Text = GlobalConfiguration[GetRegionName()]["ServerIP"].GetString(true, false);
            DefaultChannelName.Text = GlobalConfiguration["General"]["Channel"].GetString(true, false);
        }

        private void Server_TextChanged(object sender, TextChangedEventArgs e)
        {
           // GlobalConfiguration[GetRegionName()]["ServerIP"] = ServerTextBox.Text;
        }

        private void War3Path_TextChanged(object sender, TextChangedEventArgs e)
        {
             
        }
        

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new FolderBrowserDialog();
           
            if (dlg.ShowDialog(this.GetIWin32Window()) == System.Windows.Forms.DialogResult.OK)
            {
                GlobalConfiguration["General"]["PathToWc3"] = dlg.SelectedPath + @"\";
                War3PathTextBox.Text = dlg.SelectedPath + @"\";
            }
        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {

        }

        private void WindowModeCheckBox_Checked(object sender, RoutedEventArgs e)
        {
            GlobalConfiguration["General"]["WindowModeCheck"]  = (bool)WindowModeCheckBox.IsChecked;
        }

        private void OpenglModeCheckBox_Checked(object sender, RoutedEventArgs e)
        {
           
        }

        private void VideoDriver_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (VideoDriver.IsDropDownOpen)
            {
                VideoDriver.IsDropDownOpen = false;


                GlobalConfiguration["General"]["VideoDriver"] = VideoDriver.SelectedIndex;
            }
        }

        private void DefaultChannelName_TextChanged(object sender, TextChangedEventArgs e)
        {
            GlobalConfiguration["General"]["Channel"] = DefaultChannelName.Text;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            System.Windows.Application curApp = System.Windows.Application.Current;
            Window mainWindow = curApp.MainWindow;
            this.Left = mainWindow.Left + (mainWindow.Width - this.ActualWidth) / 2;
            this.Top = mainWindow.Top + (mainWindow.Height - this.ActualHeight) / 2;
        }

        private void War3PathTextBox_MouseUp(object sender, MouseButtonEventArgs e)
        {
            var dlg = new FolderBrowserDialog();

            if (dlg.ShowDialog(this.GetIWin32Window()) == System.Windows.Forms.DialogResult.OK)
            {
                GlobalConfiguration["General"]["PathToWc3"] = dlg.SelectedPath + @"\";
                War3PathTextBox.Text = dlg.SelectedPath + @"\";
            }
        }
    }
}
