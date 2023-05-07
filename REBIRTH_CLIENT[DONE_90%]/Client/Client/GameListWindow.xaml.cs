
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
using System.Windows.Threading;
using static Client.GlobalFunctions;
using static Client.GlobalVariables;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для GameListWindow.xaml
    /// </summary>
    public partial class GameListWindow
    {
        DispatcherTimer dispatcherTimer = new DispatcherTimer();
        int timerupdate = 5;
        public GameListWindow()
        {
            InitializeComponent();
            dispatcherTimer.Tick += DispatcherTimer_Tick;
            dispatcherTimer.Interval = new TimeSpan(0, 0, 1);
            dispatcherTimer.Start();
            GlobalFunctions.SendClientPacket(0x101506, 0, 0, 0, 0, "", "", "", "");
        }

        private void DispatcherTimer_Tick(object sender, EventArgs e)
        {
            this.Title = "Games " + GameList.Items.Count + ". " + timerupdate + " sec to update...";
            timerupdate--;
            if (timerupdate == 0)
            {
                timerupdate = 5;
                GlobalFunctions.SendClientPacket(0x101506, 0, 0, 0, 0, "", "", "", "");
            }
        }

        class GameListItem
        {
            public string GameName { get; set; }
            public string MapName { get; set; }
            public string HostName { get; set; }
            public string Players { get; set; }
            public int MapStructId { get; set; }
            public string InternalName { get; set; }
        }

        public void ClearHostedGameList()
        {
            Application.Current.Dispatcher.Invoke(new Action(() => GameList.Items.Clear()));
            Application.Current.Dispatcher.Invoke(new Action(() => GameList.UpdateLayout()));
        }

        public void AddNewGameToList(string GameName, string MapCode, string HostName, string Players, string MaxPlayers, string InternalName)
        {
            for (int i = 0; i < GlobalVariables.MapHostList.Count; i++)
            {
                if (GlobalVariables.MapHostList[i].MapHost == MapCode)
                {
                    GameListItem gameListItem = new GameListItem();
                    gameListItem.MapName = GlobalVariables.MapHostList[i].MapName;
                    gameListItem.HostName = HostName;
                    gameListItem.Players = "( " + Players + "/" + MaxPlayers + " )";
                    gameListItem.GameName = GameName;
                    gameListItem.MapStructId = i;
                    try
                    {
                        Application.Current.Dispatcher.Invoke(new Action(() => GameList.Items.Add(gameListItem)));
                        // GameList.Items.Add(gameListItem);
                    }
                    catch
                    {

                    }
                    break;
                }
            }
        }



        private void GameList_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {

            if (GameList.SelectedIndex != -1)
            {
                if (!GlobalVariables.GameLobbyOpened)
                {
                    try
                    {
                        // MessageBox.Show(GameList.SelectedIndex.ToString());
                        GameListItem gameListItem = GameList.Items[GameList.SelectedIndex] as GameListItem;
                        // MessageBox.Show(GameList.SelectedIndex.ToString());
                        // MessageBox.Show("Join to game:" + gameListItem.GameName);


                        var s = GlobalVariables.MapHostList[gameListItem.MapStructId];

                        GameLobbyWindow.IsHostMode = false;

                        string MapFullPath = GlobalFunctions.GetMapFullPath(s);

                        if (GlobalFunctions.CheckMapAvaiabled(s))
                        {
                            if (GlobalVariables.gameLobbyWindow != null)
                            {
                                GlobalVariables.gameLobbyWindow.Visibility = Visibility.Hidden;
                                GlobalVariables.gameLobbyWindow.Close();
                                GlobalVariables.gameLobbyWindow = null;
                                GlobalVariables.GameLobbyOpened = false;
                            }
                            GlobalVariables.GameLobbyOpened = true;
                            GlobalVariables.gameLobbyWindow = new GameLobbyWindow(MapFullPath, s, gameListItem.GameName);
                            GlobalFunctions.SendClientPacket(0x101507, 0, 0, 0, 0, gameListItem.GameName, "", "", "");
                            this.Close();
                            CheckForPatches();
                            // GlobalVariables.gameLobbyWindow.Owner = this;
                            GlobalVariables.gameLobbyWindow.ShowDialog();

                        }
                        else
                        {
                            MapDownloadWindow download = new MapDownloadWindow(s.MapFileName, GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + s.MapFileName, s.MapName);
                            //download.Owner = this;
                            download.ShowDialog();
                            MapFullPath = GlobalFunctions.GetMapFullPath(s);

                            if (GlobalFunctions.CheckMapAvaiabled(s))
                            {
                                if (GlobalVariables.gameLobbyWindow != null)
                                {
                                    GlobalVariables.gameLobbyWindow.Visibility = Visibility.Hidden;
                                    GlobalVariables.gameLobbyWindow.Close();
                                    GlobalVariables.gameLobbyWindow = null;
                                    GlobalVariables.GameLobbyOpened = false;
                                }

                                GlobalVariables.GameLobbyOpened = true;
                                GlobalVariables.gameLobbyWindow = new GameLobbyWindow(MapFullPath, s, gameListItem.GameName);
                                GlobalFunctions.SendClientPacket(0x101507, 0, 0, 0, 0, gameListItem.GameName, "", "", "");
                                this.Close();
                                CheckForPatches();
                                // GlobalVariables.gameLobbyWindow.Owner = this;
                                GlobalVariables.gameLobbyWindow.ShowDialog();
                            }
                            else
                            {

                            }
                        }


                    }
                    catch
                    {
                        MessageBox.Show("Fatal Terror");
                        if (GlobalVariables.gameLobbyWindow != null)
                        {
                            GlobalVariables.gameLobbyWindow.Visibility = Visibility.Hidden;
                            GlobalVariables.gameLobbyWindow.Close();
                            GlobalVariables.gameLobbyWindow = null;
                            GlobalVariables.GameLobbyOpened = false;
                        }
                    }
                }
            }
        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {
            GlobalVariables.GameListOpened = false;
            GlobalVariables.gameListWindow = null;


            ClientWindow.CreateGameButtonStatic.Visibility = Visibility.Visible;
            ClientWindow.GameListButtonStatic.Visibility = Visibility.Visible;

            if (GlobalVariables.LoginWindow != null)
                GlobalVariables.LoginWindow.Hide();
        }

        private void RefreshBtn_Click(object sender, RoutedEventArgs e)
        {
            GlobalFunctions.SendClientPacket(0x101506, 0, 0, 0, 0, "", "", "", "");
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

