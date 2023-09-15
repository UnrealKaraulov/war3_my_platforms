
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.IO;
using System.Threading;
using static Client.GlobalVariables;
using System.Diagnostics;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для ClientWindow.xaml
    /// </summary>
    public partial class ClientWindow
    {

        DispatcherTimer dispatcherTimer = new DispatcherTimer();
        bool updatechannel = true;
        public ClientWindow()
        {
            InitializeComponent();

            dispatcherTimer.Tick += new EventHandler(UpdateClientPackets);
            dispatcherTimer.Interval = new TimeSpan(0, 0, 1);
            dispatcherTimer.Start();

            ProcessClientCommandInstance = new ProcessClientCommandDelegate(ProcessClientCommand);

            SetCommandsCallback(ProcessClientCommandInstance);

            GlobalFunctions.SendClientPacket(0x101505, 0, 0, 0, 0, "", "", "", "");
            GlobalVariables.HostMenuOpened = false;
            GlobalVariables.GameLobbyOpened = false;
            GlobalVariables.GameListOpened = false;
        }

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void ProcessCommand(string command);

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void JoinChannelCommand(string command);

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetCommandsCallback(ProcessClientCommandDelegate fn);

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void Renew();


        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void AddFakeGame(int games);


        void UpdateClientPackets(object sender, EventArgs e)
        {
            if (updatechannel)
            {
                updatechannel = false;
                try
                {
                    string defaultchannel = GlobalConfiguration["General"]["Channel"].GetString(true, false);
                    if (defaultchannel.Length > 0)
                        ProcessCommand("/join " + defaultchannel);
                    else
                        ProcessCommand("/join main");
                }
                catch
                {

                }

            }
            dispatcherTimer.Interval = new TimeSpan(0, 0, 5);
            GlobalFunctions.SendClientPacket(0x101505, 0, 0, 0, 0, "", "", "", "");
            Renew();
        }

        void SendMessageToServer(string Message)
        {
            Message = new string(Message.Where(c => !char.IsControl(c)).ToArray());
            GlobalFunctions.DefaultToUtf8(ref Message);
            ProcessCommand(Message);

            if (Message.Length > 0 && Message[0] != '/')
                ProcessClientCommand("/addchanneltext", GlobalVariables.Username, Message, "", "", "", "", "", "", "", "", "");
        }

        void ChatInputSendText()
        {
            if (ChatInputTextBox.Text.Length > 0)
            {
                SendMessageToServer(ChatInputTextBox.Text);
                ChatInputTextBox.Clear();
            }
        }

        private void RichTextBox_KeyUp(object sender, KeyEventArgs e)
        {
            if (!e.IsRepeat && e.Key == Key.Enter)
            {
                ChatInputSendText();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            ChatInputSendText();
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void ProcessClientCommandDelegate(string Command, string arg1, string arg2, string arg3, string arg4, string arg5
                                                                        , string arg6, string arg7, string arg8, string arg9, string arg10, string arg11);
        private ProcessClientCommandDelegate ProcessClientCommandInstance = null;

        private const int SW_HIDE = 0;
        private const int SW_SHOW = 5;
        [DllImport("user32.dll")]
        static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

        void ProcessClientCommand(string Command, string arg1, string arg2, string arg3, string arg4, string arg5
            , string arg6, string arg7, string arg8, string arg9, string arg10, string arg11)
        {
            if (!Application.Current.Dispatcher.CheckAccess())
            {
                try
                {
                    Application.Current.Dispatcher.Invoke(new Action<string, string, string, string, string,
                        string, string, string, string, string, string, string>(ProcessClientCommand),
                        Command, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
                }
                catch
                {

                }
                return;
            }


            GlobalFunctions.Utf8ToDefault(ref Command);
            GlobalFunctions.Utf8ToDefault(ref arg1);
            GlobalFunctions.Utf8ToDefault(ref arg2);
            GlobalFunctions.Utf8ToDefault(ref arg3);
            GlobalFunctions.Utf8ToDefault(ref arg4);
            GlobalFunctions.Utf8ToDefault(ref arg5);
            GlobalFunctions.Utf8ToDefault(ref arg6);
            GlobalFunctions.Utf8ToDefault(ref arg7);
            GlobalFunctions.Utf8ToDefault(ref arg8);
            GlobalFunctions.Utf8ToDefault(ref arg9);
            GlobalFunctions.Utf8ToDefault(ref arg10);

            if (Command.Length > 1)
            {
                Command = Command.ToLower().Remove(0, 1);
                File.AppendAllText("OutCommands.txt", Command + "...1\n");

                if (Command == "clearmaplist")
                {
                    GlobalVariables.MapHostList.Clear();
                }
                else if (Command == "addnewmap")
                {
                    var tmpmaphoststruct = new GlobalVariables.MapHostStruct()
                    {
                        availabled = true,
                        MapName = arg1,
                        MapHost = arg2,
                        MapFileName = arg11,
                        MapCategory = arg4,
                        crc32 = uint.Parse(arg5),
                        ForStats = arg6 == "true"
                    };
                    GlobalVariables.MapHostList.Add(tmpmaphoststruct);
                }
                else if (Command == "addtext" && arg1.Length > 0)
                {
                    ChatBox.AppendText(arg1 + "\n");
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "addchanneltext" && arg1.Length > 0 && arg2.Length > 0)
                {
                    ChatBox.AppendText(arg1, "White", true);
                    ChatBox.AppendText(":", "White", false);
                    ChatBox.AppendText(arg2 + "\n", "White", false);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "addusermessage" && arg1.Length > 0 && arg2.Length > 0)
                {
                    ChatBox.AppendText("[ PM by ");
                    ChatBox.AppendText(arg1, "Green", true);
                    ChatBox.AppendText(" ] : ");
                    ChatBox.AppendText(arg2 + "\n", "White", false);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "adderrortext" && arg1.Length > 0)
                {
                    if (arg1.IndexOf("create game before ") > -1)
                        return;
                    ChatBox.AppendText(arg1 + "\n", "#ff3300", true);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "addinfotext" && arg1.Length > 0)
                {
                    ChatBox.AppendText(arg1 + "\n", "#0080ff", true);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "addannouncetext" && arg1.Length > 0)
                {
                    ChatBox.AppendText(arg1 + "\n", "Blue", true);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "addemotetext" && arg1.Length > 0 && arg2.Length > 0)
                {
                    ChatBox.AppendText(arg1, "DarkSlateGray", true);
                    ChatBox.AppendText(":", "White", false);
                    ChatBox.AppendText(arg2 + "\n", "Gray", false);
                    ChatBox.ScrollToEnd();
                }
                else if (Command == "clearchatusers")
                {
                    UsersInThisChannel.Items.Clear();
                }
                else if (Command == "addchatuser" && arg1.Length > 0)
                {
                    UsersInThisChannel.Items.Add(arg1);
                }
                else if (Command == "removechatuser" && arg1.Length > 0)
                {
                    UsersInThisChannel.Items.Remove(arg1);
                }
                else if (Command == "channelname" && arg1.Length > 0)
                {
                    Application.Current.Dispatcher.Invoke(new Action(() => this.Title = "Rebirth client. Channel:" + arg1));
                }
                else if (Command == "clearhostedgamelist")
                {
                    if (GlobalVariables.GameListOpened && GlobalVariables.gameListWindow != null)
                    {
                        GlobalVariables.gameListWindow.ClearHostedGameList();
                    }
                }
                else if (Command == "addnewgametolist" && arg1.Length > 0
                    && arg2.Length > 0 && arg3.Length > 0 && arg4.Length > 0 && arg5.Length > 0 && arg6.Length > 0)
                {
                    if (GlobalVariables.GameListOpened && GlobalVariables.gameListWindow != null)
                    {
                        GlobalVariables.gameListWindow.AddNewGameToList(arg1, arg2, arg3, arg4, arg5, arg6);
                    }
                }
                else if (Command == "setslotstatus" && arg1.Length > 0 && arg3.Length > 0)
                {
                    if (GlobalVariables.GameLobbyOpened && GlobalVariables.gameLobbyWindow != null)
                    {
                        //  MessageBox.Show("Swap slot " + arg1 + " to name " + arg2);
                        if (arg3 == "1")
                        {
                            GlobalVariables.gameLobbyWindow.Close();
                            GlobalVariables.GameLobbyOpened = false;
                            GlobalVariables.gameLobbyWindow = null;
                        }
                        else
                        {
                            GlobalVariables.gameLobbyWindow.SetSlotName(uint.Parse(arg1), arg2);
                        }
                    }
                }
                else if (Command == "setracestatus" && arg1.Length > 0 && arg3.Length > 0)
                {
                    if (GlobalVariables.GameLobbyOpened && GlobalVariables.gameLobbyWindow != null)
                    {
                        // MessageBox.Show("setracestatus " + arg1 + " to race " + arg2);
                        if (arg3 == "1")
                        {
                            GlobalVariables.gameLobbyWindow.Close();
                            GlobalVariables.GameLobbyOpened = false;
                            GlobalVariables.gameLobbyWindow = null;
                        }
                        else
                            GlobalVariables.gameLobbyWindow.SetSlotRace(uint.Parse(arg1), uint.Parse(arg2));
                    }
                }
                else if (Command == "updateallslots")
                {
                    if (GlobalVariables.GameLobbyOpened && GlobalVariables.gameLobbyWindow != null)
                    {
                        GlobalVariables.gameLobbyWindow.UpdateAllSlots();
                    }
                }
                else if (Command == "closelobby")
                {
                    if (GlobalVariables.GameLobbyOpened && GlobalVariables.gameLobbyWindow != null)
                    {
                        GlobalVariables.gameLobbyWindow.Exit();
                        GlobalVariables.gameLobbyWindow = null;
                        GlobalVariables.GameLobbyOpened = false;
                    }
                }
                else if (Command == "messagebox" && arg1.Length > 0 && arg2.Length > 0)
                {
                    GlobalFunctions.ShowMessagBox(arg1, arg2);
                }
                else if (Command == "startwar3game" && arg1.Length > 0)
                {
                    if (GlobalVariables.GameLobbyOpened && GlobalVariables.gameLobbyWindow != null)
                    {
                        GlobalVariables.gameLobbyWindow.Exit();
                    }
                    try
                    {
                        File.Delete(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"hide.me");
                    }
                    catch
                    {

                    }
                    try
                    {
                        File.Delete(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"log_in.me");
                    }
                    catch
                    {

                    }
                    ProcessCommand("/announcegame");
                    Thread.Sleep(200);
                    GlobalFunctions.StartWarcraftAndJoinGame(GlobalConfiguration["General"]["PathToWc3"].GetString(true, false), arg1);


                    Thread StartWatchgGameThread = new Thread(StartWatchGame);
                    StartWatchgGameThread.IsBackground = true;
                    StartWatchgGameThread.Start();

                    Thread ShowLoadGameThread = new Thread(ShowLoadGameWindow);
                    ShowLoadGameThread.IsBackground = true;
                    ShowLoadGameThread.Start();
                    // loadWindow = new LoadWindow();
                    //loadWindow.Show();
                }
                else if (Command == "addlobbbytext" && arg2.Length > 0 && arg1.Length > 0)
                {
                    if (GlobalVariables.GameLobbyChatBox != null)
                    {
                        GlobalVariables.GameLobbyChatBox.AppendText(arg1, "Red", true);
                        GlobalVariables.GameLobbyChatBox.AppendText(":", "White", false);
                        GlobalVariables.GameLobbyChatBox.AppendText(arg2 + "\n", "White", false);
                        GlobalVariables.GameLobbyChatBox.ScrollToEnd();
                    }
                }
                else if (Command == "closewar3")
                {
                    while (Process.GetProcessesByName("war3").Length > 0)
                    {
                        try
                        {
                            Process.GetProcessesByName("war3")[0].Kill();
                        }
                        catch (System.ComponentModel.Win32Exception ex)
                        {

                        }
                        catch
                        {

                        }
                    }
                    while (Process.GetProcessesByName("WAR.bin").Length > 0)
                    {
                        try
                        {
                            Process.GetProcessesByName("WAR.bin")[0].Kill();
                        }
                        catch (System.ComponentModel.Win32Exception ex)
                        {

                        }
                        catch
                        {

                        }
                    }

                }

                File.AppendAllText("OutCommands.txt", Command + "...4\n");
            }
        }

        private void StartWatchGame()
        {
            int foundfile = Environment.TickCount;
            int foundfile2 = Environment.TickCount;
            while (true)
            {
                if (foundfile == foundfile2)
                {
                    if (Environment.TickCount - foundfile2 > 20000)
                    {
                        while (Process.GetProcessesByName("WAR.bin").Length > 0)
                        {
                            try
                            {
                                Process.GetProcessesByName("WAR.bin")[0].Kill();
                            }
                            catch
                            {

                            }
                            try
                            {
                                ShowWindow(Process.GetProcessesByName("WAR.bin")[0].MainWindowHandle, 11);
                            }
                            catch
                            {

                            }
                            try
                            {
                                ShowWindow(Process.GetProcessesByName("WAR.bin")[0].MainWindowHandle, 0);
                            }
                            catch
                            {

                            }
                        }
                        MessageBox.Show("Игра не отвечала и была уничтожена. Уничтожена. УНИЧТОЖЕНА!");
                        break;
                    }
                }

                Thread.Sleep(10);
                if (Process.GetProcessesByName("WAR.bin").Length != 1)
                {
                    ProcessCommand("/unhost");
                    return;
                }
                else
                {
                    string fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"give_me_cookies.me";
                    if (File.Exists(fullpath))
                    {
                        foundfile = Environment.TickCount;
                        ProcessCommand("/stopannouncegame");
                        try
                        {
                            File.Delete(fullpath);
                        }
                        catch
                        {

                        }
                    }
                    fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"give_me_cookies2.me";
                    if (File.Exists(fullpath))
                    {
                        foundfile = Environment.TickCount;
                        ProcessCommand("/announcegame");
                        try
                        {
                            File.Delete(fullpath);
                        }
                        catch
                        {

                        }
                    }

                }
            }
        }

        //LoadWindow loadWindow = null;

        void ShowLoadGameWindow()
        {
            int maxseconds = 300;
            maxseconds = 300;
            string fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"log_in.me";

            while (--maxseconds > 0)
            {
                Thread.Sleep(100);

                if (File.Exists(fullpath))
                {
                    Thread.Sleep(100);
                    File.Delete(fullpath);
                    break;
                }
            }

            //try
            //{
            //    Application.Current.Dispatcher.Invoke(new Action(() => loadWindow.Close()));
            //    loadWindow = null;
            //}
            //catch
            //{

            //}
        }

        public static Button CreateGameButtonStatic = null;
        public static Button GameListButtonStatic = null;

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            if (!GlobalVariables.HostMenuOpened)
            {
                GlobalVariables.HostMenuOpened = true;
                var SysLog3 = new HostMenuWindow();
                CreateGameButtonStatic = CreateGameButton;
                CreateGameButton.Visibility = Visibility.Hidden;
                GameListButtonStatic = GameListButton;
                GameListButtonStatic.Visibility = Visibility.Hidden;
                if (GlobalVariables.LoginWindow != null)
                    GlobalVariables.LoginWindow.Hide();
                SysLog3.Show();
                if (GlobalVariables.LoginWindow != null)
                    GlobalVariables.LoginWindow.Hide();
            }
        }


        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            if (!GlobalVariables.GameListOpened)
            {
                GlobalVariables.GameListOpened = true;
                GlobalVariables.gameListWindow = new GameListWindow();

                CreateGameButtonStatic = CreateGameButton;
                CreateGameButton.Visibility = Visibility.Hidden;
                GameListButtonStatic = GameListButton;
                GameListButtonStatic.Visibility = Visibility.Hidden;

                if (GlobalVariables.LoginWindow != null)
                    GlobalVariables.LoginWindow.Hide();
                GlobalVariables.gameListWindow.Show();
                if (GlobalVariables.LoginWindow != null)
                    GlobalVariables.LoginWindow.Hide();
            }
        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {

            GlobalVariables.clientWindow = null;

            if (Application.Current.MainWindow == this)
            {
                Application.Current.MainWindow = null;
                for (int i = Application.Current.Windows.Count - 1; i > 0; i--)
                {
                    if (Application.Current.Windows[i] != this)
                        Application.Current.MainWindow = Application.Current.Windows[i];
                }
            }

            if (GlobalVariables.gameListWindow != null)
            {
                GlobalVariables.gameListWindow.Close();
                GlobalVariables.gameListWindow = null;
            }
            if (GlobalVariables.gameLobbyWindow != null)
            {
                GlobalVariables.gameLobbyWindow.Close();
                GlobalVariables.gameLobbyWindow = null;
            }

            if (GlobalVariables.Wc3ProxyThread != null)
            {
                try
                {
                    File.Delete("BadClient.ini");
                }
                catch
                {

                }

                try
                {
                    File.Delete("Client.ini");
                }
                catch
                {
                    try
                    {
                        File.Move("Client.ini", "BadClient.ini");
                    }
                    catch
                    {

                    }
                }
                GlobalFunctions.TerminateProxy();
                GlobalVariables.Wc3ProxyThread = null;
                GlobalConfiguration["General"]["LastAccess"] = DateTime.Now.ToShortTimeString();
                GlobalVariables.GlobalConfiguration.Save("Client.ini");
                //GlobalVariables.Wc3ProxyThread.Interrupt();
                //GlobalVariables.Wc3ProxyThread.Abort();
            }
        }

        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            AddFakeGame(1);

            MessageBox.Show("add 1 game");
        }

        private void Button_Click_4(object sender, RoutedEventArgs e)
        {
            AddFakeGame(-1);
            MessageBox.Show("add -11 game");
        }

        private void Button_Click_5(object sender, RoutedEventArgs e)
        {
            var SysLog3 = new War3ProxySettings();
            SysLog3.ShowDialog();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            GlobalVariables.clientWindow = this;
            Application curApp = Application.Current;
            Window mainWindow = curApp.MainWindow;
            this.Left = mainWindow.Left + (mainWindow.Width - this.ActualWidth) / 2;
            this.Top = mainWindow.Top + (mainWindow.Height - this.ActualHeight) / 2;
            Application.Current.MainWindow = this;
        }

    }
}
