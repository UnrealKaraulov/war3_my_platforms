
using System;
using System.Collections.Generic;
using System.IO;
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
using static Client.GlobalFunctions;
using static Client.GlobalVariables;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для HostMenuWindow.xaml
    /// </summary>
    public partial class HostMenuWindow
    {


        public HostMenuWindow()
        {
            InitializeComponent();

            // foreach (var map in GlobalVariables.MapHostList)
            // {
            // MessageBox.Show("Add new map:" + map.MapName);
            //    this.MapListView.Items.Add(map);
            // }

            MapListView.Items.Clear();
            MapListView.ItemsSource = GlobalVariables.MapHostList;

            SelectedGameName.Text = "Game by " + GlobalVariables.Username;
        }


        // packets : refresh_game add_player del_player ready_player (or after 15 sec join) start_game

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            // Send packet game create
            // Wait for response, open gamelobby with needed settings

            if (MapListView.SelectedItem == null)
            {
                MessageBox.Show("Please select map first!");
                return;
            }

            GlobalVariables.MapHostStruct tmpMapHostStruct = MapListView.SelectedItem as GlobalVariables.MapHostStruct;

            bool foundmap = false;

            for (int i = 0; i < GlobalVariables.MapHostList.Count; i++)
            {
                if (GlobalVariables.MapHostList[i].MapFileName == tmpMapHostStruct.MapFileName)
                {
                    tmpMapHostStruct = GlobalVariables.MapHostList[i];
                    foundmap = GlobalFunctions.CheckMapAvaiabled(tmpMapHostStruct);

                    if (foundmap)
                        break;
                }
            }

            if (!foundmap)
            {
                string fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName;
                try
                {
                    File.Delete(fullpath + ".crcmiss");
                }
                catch
                {

                }
                try
                {
                    File.Delete(fullpath);
                }
                catch
                {
                    try
                    {
                        File.Move(fullpath, fullpath + ".crcmiss");
                    }
                    catch
                    {


                    }
                }
                MapDownloadWindow download = new MapDownloadWindow(tmpMapHostStruct.MapFileName, GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName, tmpMapHostStruct.MapName);
                //download.Owner = this;
                download.ShowDialog();

                if (!download.DownloadSuccess && download.DownloadStart)
                {
                    try
                    {
                        File.Delete(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName + ".crcmiss");
                    }
                    catch
                    {

                    }
                    try
                    {
                        File.Delete(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName);
                    }
                    catch
                    {
                        try
                        {
                            File.Move(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName,
                                 GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName + ".crcmiss");
                        }
                        catch
                        {

                        }
                    }
                    MessageBox.Show("FATAL ERROR. CLIENT CLOSE DOWNLOAD WINDOW. FILE IS CORRUPT!");
                    return;
                }

                if (!GlobalFunctions.CheckMapAvaiabled(tmpMapHostStruct))
                {
                    MessageBox.Show("Please download selected map first!", "FILE IS CORRUPT!");
                    return;
                }
            }

            string MapFullPath = GlobalFunctions.GetMapFullPath(tmpMapHostStruct);

            if (!GlobalVariables.GameLobbyOpened)
            {
                GlobalVariables.GameLobbyOpened = true;
                GameLobbyWindow.IsHostMode = true;
                string gamename = SelectedGameName.Text;
                DefaultToUtf8(ref gamename);
                if (gamename.Length < 3)
                {
                    MessageBox.Show("Короткий ч...имя игры!");
                    this.Close();
                }
                else
                {
                    CheckForPatches();
                    GlobalVariables.gameLobbyWindow = new GameLobbyWindow(MapFullPath, tmpMapHostStruct, gamename);
                    this.Close();
                    //GlobalVariables.gameLobbyWindow.Owner = this;
                    ClientWindow.CreateGameButtonStatic.Visibility = Visibility.Hidden;
                    ClientWindow.GameListButtonStatic.Visibility = Visibility.Hidden;
                    GlobalVariables.gameLobbyWindow.Show();
                }
            }
            else
            {
                MessageBox.Show("Please unhost your map first!");
            }

        }

        private void ListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

        }

        private void Window_Closed(object sender, EventArgs e)
        {
            GlobalVariables.HostMenuOpened = false;
            if (GlobalVariables.LoginWindow != null)
                GlobalVariables.LoginWindow.Hide();
            ClientWindow.CreateGameButtonStatic.Visibility = Visibility.Visible;
            ClientWindow.GameListButtonStatic.Visibility = Visibility.Visible;
        }

        private void MapListView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            Button_Click(null, new RoutedEventArgs());
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
