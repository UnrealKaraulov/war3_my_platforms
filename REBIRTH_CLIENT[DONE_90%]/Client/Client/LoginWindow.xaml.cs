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
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Management;
using System.ServiceProcess;
using WindowsFirewallHelper;
using System.Net;
using System.Net.NetworkInformation;
using System.Windows.Threading;
using OtherInfo;
using System.ComponentModel;
using System.Diagnostics;
using static Client.GlobalVariables;
using static Client.GlobalFunctions;
using FluentFTP;
using System.Security.Cryptography;
using Microsoft.Win32;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>


    public partial class LoginWindow
    {
        /// 
        static Mutex mutex = null;

        static List<string> UpdateScript = new List<string>();

        public static void AddFileToUpdate(string filename)
        {
            UpdateScript.Add("del " + filename + ".del2");
            UpdateScript.Add("del " + filename + ".del");
            UpdateScript.Add("del " + filename);
            UpdateScript.Add("ren " + filename + ".del " + filename + ".del2");
            UpdateScript.Add("ren " + filename + " " + filename + ".del");
            UpdateScript.Add("del " + filename + ".del2");
            UpdateScript.Add("del " + filename + ".del");
            UpdateScript.Add("del " + filename);
            UpdateScript.Add("ren " + filename + ".upd " + filename);
        }

        static string CalculateMD5(string filename)
        {
            try
            {
                using (var md5 = MD5.Create())
                {
                    using (var stream = File.OpenRead(filename))
                    {
                        var hash = md5.ComputeHash(stream);
                        return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                    }
                }
            }
            catch
            {
                return "";
            }
        }

        public static string GetPureFilenameFromFtp(string filename)
        {
            return filename.Replace("./", "").Replace("/", "").Replace(".\\", "").Replace("\\", "");
        }

        public static void LoadUpdatesFromFtp()
        {
            try
            {
                if (File.Exists("UpdateScript.bat"))
                {
                    File.Delete("UpdateScript.bat");
                    return;
                }
            }
            catch
            {
                return;
            }

            try
            {
                UpdateScript.Add(@"PING -n 3 127.0.0.1 >NUL 2>&1 || PING -n 3 ::1 >NUL 2>&1");

                FtpClient ClientUpdaterFtp = new FtpClient(GetStringByRegion("UpdaterFtp"), 21, new NetworkCredential("clientdataread", "clientdataread"));

                ClientUpdaterFtp.Connect();
                ClientUpdaterFtp.Execute("OPTS HASH MD5");

                var UpdateFiles = ClientUpdaterFtp.GetListing();

                if (UpdateFiles.Length == 0)
                    return;

                List<string> IgnoreDeleteFiles = new List<string>();
                IgnoreDeleteFiles.Add("Client.ini");
                IgnoreDeleteFiles.Add("w3proxy.txt");
                IgnoreDeleteFiles.Add("LogClient.txt");
                IgnoreDeleteFiles.Add("reloadmaplist.txt");

                List<string> ClientFileList = new List<string>();
                List<string> UpdateFileList = new List<string>();
                List<string> UpdateFileListServer = new List<string>();
                List<string> FilesForDeleting = new List<string>();

                foreach (var updfile in UpdateFiles)
                {
                    var replay = ClientUpdaterFtp.Execute("HASH " + updfile.FullName);
                    string purefilename = GetPureFilenameFromFtp(updfile.FullName);
                    ClientFileList.Add(purefilename);
                    string md5hash_client = CalculateMD5(purefilename).ToLower();
                    string md5hash_server = replay.Message.Replace("MD5", "").Trim().ToLower();
                    if (md5hash_server.IndexOf("error") > 0)
                    {
                        MessageBox.Show("Client update server error!!");
                        return;
                    }
                    if (md5hash_client != md5hash_server)
                    {
                        AddFileToUpdate(purefilename);
                        UpdateFileList.Add(purefilename);
                        UpdateFileListServer.Add(updfile.FullName);
                    }
                }


                foreach (var badname in Directory.GetFiles(Directory.GetCurrentDirectory()))
                {
                    string s = System.IO.Path.GetFileName(badname);
                    if (!ClientFileList.Any(x => x.Equals(s, StringComparison.OrdinalIgnoreCase)))
                    {
                        if (!UpdateFileList.Any(x => x.Equals(s, StringComparison.OrdinalIgnoreCase)))
                        {
                            if (!IgnoreDeleteFiles.Any(x => x.Equals(s, StringComparison.OrdinalIgnoreCase)))
                            {
                                AddFileToUpdate(s);
                            }
                        }
                    }
                }

                if (UpdateFileList.Count > 0 && !File.Exists("noupdate.noupdate"))
                {
                    bool AllFilesDownloaded = false;
                    try
                    {
                        var clientupdwindow = new ClientUpdaterWindow(ClientUpdaterFtp, UpdateFileList.ToArray(), UpdateFileListServer.ToArray());

                        AllFilesDownloaded = (bool)clientupdwindow.ShowDialog();
                    }
                    catch
                    {
                        AllFilesDownloaded = false;
                    }


                    if (UpdateFileList.Count > 0 && AllFilesDownloaded)
                    {
                        UpdateScript.Add(@"start Client.exe");
                    }

                    if (AllFilesDownloaded)
                    {
                        File.WriteAllLines("UpdateScript.bat", UpdateScript.ToArray());
                        ProcessStartInfo process = new ProcessStartInfo("UpdateScript.bat");
                        process.CreateNoWindow = true;
                        process.WindowStyle = ProcessWindowStyle.Hidden;
                        Process.Start(process);
                        if (UpdateFileList.Count > 0)
                        {
                            Environment.Exit(0);
                        }
                    }
                }

                try
                {
                    ClientUpdaterFtp.Disconnect();
                }
                catch
                {

                }
            }
            catch
            {

            }
        }

        public void UpdateLanguageForCurrentWindow()
        {
            LoginWindowUsernameLabel.Content = GetLocalizedString("LoginWindowUsernameLabel");
            LoginWindowPasswordLabel.Content = GetLocalizedString("LoginWindowPasswordLabel");
            LoginWindowVersionLabel.Content = GetLocalizedString("LoginWindowVersionLabel");
            PlatformLabel.Content = GetLocalizedString("PlatformLabel");
            LoginWindowSettingsButton.Content = GetLocalizedString("LoginWindowSettingsButton");
            LoginWindowLoginButton.Content = GetLocalizedString("LoginWindowLoginButton");
            LoginWindowWebsiteButton.Content = GetLocalizedString("LoginWindowWebsiteButton");
            LoginWindowRegisterButton.Content = GetLocalizedString("LoginWindowRegisterButton");
            LoginWindowRememberCheckbox.Content = GetLocalizedString("LoginWindowRememberCheckbox");
        }

        bool ThisWindowLoaded = false;
        bool BadFirstInit = true;
        public LoginWindow()
        {
            GlobalVariables.LoginWindow = this;



            FileInfo fileInfo = new FileInfo("Client.ini");
            string lastaccess = fileInfo.LastAccessTime.ToShortTimeString();

            GlobalConfiguration = new IniFile();
            try
            {
                if (File.Exists("GlobalSettings.ini"))
                {
                    File.AppendAllLines("Client.ini", File.ReadAllLines("GlobalSettings.ini"));
                }
            }
            catch
            {

            }
            try
            {
                GlobalConfiguration.Load("Client.ini", true);
            }
            catch
            {
                MessageBox.Show("Error load client.ini");
            }


            string insideaccess = GlobalConfiguration["General"]["LastAccess"].GetString();

            if (lastaccess != insideaccess)
            {
                //MessageBox.Show("Something read your configuaration file." + insideaccess + " / " + lastaccess, "Possible security warning.");
            }

            GlobalConfiguration["General"]["LastAccess"] = DateTime.Now.ToShortTimeString();

            if (DesignerProperties.GetIsInDesignMode(this))
            {
                InitializeComponent();
            }
            else
            {
                if (!GlobalVariables.OneInstance)
                    mutex = new Mutex(true, "{1F1F0EE4-3BA2-62fe-AC3F-24F0526BDE64}");

                if (GlobalVariables.OneInstance || mutex.WaitOne(TimeSpan.Zero, true))
                {
                    InitializeComponent();
                    UpdateLanguageForCurrentWindow();

                    int Regions = GlobalConfiguration["RegionSettings"]["Regions"].ToInt();
                    for (int i = 1; i <= Regions; i++)
                    {
                        LoginWindowRegionChange.Items.Add(GlobalConfiguration["Region" + i]["Name"].GetString() + "/" + GlobalConfiguration["Region" + i]["Language"].GetString());
                    }
                    LoginWindowRegionChange.SelectedIndex = GlobalConfiguration["General"]["Region"].ToInt() - 1;
                    if (BadFirstInit)
                    {
                        BadFirstInit = false;
                        dispatcherTimer.Tick += new EventHandler(CheckLoginStatus);
                    }
                    dispatcherTimer.Interval = new TimeSpan(0, 0, 1);

                    UsernameInput.Text = GlobalConfiguration["General"]["Username"].GetString();
                    PasswordInput.Password = GlobalConfiguration["General"]["Password"].GetString(true, false).Unprotect();

                    // MessageBox.Show(GlobalConfiguration["General"]["SavePassword"].GetString( ));

                    LoginWindowRememberCheckbox.IsChecked = GlobalConfiguration["General"]["SavePassword"].ToBool();

                    GlobalVariables.OneInstance = true;

                    LoadUpdatesFromFtp();
                    try
                    {
                        if (File.Exists("w3proxy.txt"))
                        {
                            List<string> LogFile = new List<string>(File.ReadAllLines("w3proxy.txt"));
                            if (LogFile.Count - 100 > 0)
                            {
                                LogFile.RemoveRange(0, LogFile.Count - 100);
                                File.WriteAllLines("w3proxy.txt", LogFile.ToArray());
                            }
                        }
                    }
                    catch
                    {

                    }

                    try
                    {
                        if (File.Exists("OutCommands.txt"))
                        {
                            List<string> LogFile = new List<string>(File.ReadAllLines("OutCommands.txt"));
                            if (LogFile.Count - 100 > 0)
                            {
                                LogFile.RemoveRange(0, LogFile.Count - 100);
                                File.WriteAllLines("OutCommands.txt", LogFile.ToArray());
                            }
                        }
                    }
                    catch
                    {

                    }
                    try
                    {
                        string War3Path = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false);

                        if (File.Exists(War3Path + "AntihackDebugger.txt"))
                        {
                            File.Delete(War3Path + "AntihackDebugger.txt");
                        }
                        File.WriteAllText(War3Path + "AntihackDebugger.txt", "Поздравляю вы смогли открыть этот файл.\n", Encoding.Default);

                    }
                    catch
                    {
                        MessageBox.Show("Нет доступа к логу античита");
                        Environment.Exit(0);
                    }
                }
                else
                {
                    MessageBox.Show("only one instance at a time");
                    Environment.Exit(0);
                }
            }


            ThisWindowLoaded = true;
        }

        enum PvPGN_Login_Status : uint
        {
            Login_Status_NONE = 0,
            Login_Status_FAILED = 1,
            Login_Status_OK = 2
        };


        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern int InitializeClientProxy(int istft, string War3Path, string Server,
        string Username, string Password, uint War3Version, ushort Port, string _EXEVersion, string _EXEVersionHash, bool IsRegister);


        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern PvPGN_Login_Status LoginStatus();
        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void ResetLoginStatus();

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        private static extern void SetVars(UInt32 var1, UInt32 var2, UInt32 var3, UInt32 var4);


        public static bool PortInUse(int port)
        {
            bool inUse = false;

            IPGlobalProperties ipProperties = IPGlobalProperties.GetIPGlobalProperties();
            IPEndPoint[] ipEndPoints = ipProperties.GetActiveTcpListeners();


            foreach (IPEndPoint endPoint in ipEndPoints)
            {
                if (endPoint.Port == port)
                {
                    inUse = true;
                    break;
                }
            }


            return inUse;
        }


        void InitFirewallPatcher()
        {
            GlobalVariables.InternalLog.Add("Initialize. Adding firewall rules...");

            int errval = 15;
            while (errval > 0)
            {
                errval--;
                try
                {
                    var myRule = FirewallManager.Instance.Rules.SingleOrDefault(r => r.RemotePorts.Contains<ushort>(6113) || r.RemotePorts.Contains<ushort>(6200) || r.RemotePorts.Contains<ushort>(7122));
                    if (myRule != null)
                    {
                        FirewallManager.Instance.Rules.Remove(myRule);
                    }
                    else
                        break;
                }
                catch
                {
                    break;
                }
            }


            errval = 15;
            while (errval > 0)
            {
                errval--;
                try
                {
                    IEnumerable<IFirewallRule> myRule = FirewallManager.Instance.Rules.Where(r => r.Name == "Rebirth Client");
                    if (myRule != null)
                    {
                        foreach (var curRule in myRule)
                            FirewallManager.Instance.Rules.Remove(curRule);
                    }
                    else
                        break;
                }
                catch
                {
                    break;
                }
            }

            errval = 15;
            while (errval > 0)
            {
                errval--;
                try
                {
                    var myRule = FirewallManager.Instance.Rules.SingleOrDefault(r => r.Name == "Warcraft III Rebirth Client");
                    if (myRule != null)
                    {
                        FirewallManager.Instance.Rules.Remove(myRule);
                    }
                    else
                        break;
                }
                catch
                {
                    break;
                }
            }

            try
            {
                var rule = FirewallManager.Instance.CreateApplicationRule(
            FirewallManager.Instance.GetActiveProfile().Type, @"Rebirth Client",
            FirewallAction.Allow, System.Reflection.Assembly.GetEntryAssembly().Location, FirewallProtocol.Any);

                rule.Direction = FirewallDirection.Outbound;
                FirewallManager.Instance.Rules.Add(rule);
                rule = FirewallManager.Instance.CreateApplicationRule(
            FirewallManager.Instance.GetActiveProfile().Type, @"Rebirth Client",
            FirewallAction.Allow, System.Reflection.Assembly.GetEntryAssembly().Location, FirewallProtocol.Any);

                rule.Direction = FirewallDirection.Inbound;
                FirewallManager.Instance.Rules.Add(rule);
                GlobalVariables.InternalLog.Add("firewall rules added");
            }
            catch
            {
                GlobalVariables.InternalLog.Add("Error adding firewall rules");
            }

            try
            {
                var rule = FirewallManager.Instance.CreateApplicationRule(
            FirewallManager.Instance.GetActiveProfile().Type, @"Warcraft III Rebirth Client",
            FirewallAction.Allow, GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + "WAR.bin", FirewallProtocol.Any);

                rule.Direction = FirewallDirection.Outbound;
                FirewallManager.Instance.Rules.Add(rule);
                rule.Direction = FirewallDirection.Inbound;
                FirewallManager.Instance.Rules.Add(rule);
                GlobalVariables.InternalLog.Add("firewall rules added");
            }
            catch
            {
                GlobalVariables.InternalLog.Add("Error adding firewall rules");
            }


            try
            {
                if (!IsWc3PortNotUsed())
                {
                    MessageBox.Show("Port 6112 needed for game - used by another program!", "Fatal error!");
                    Environment.Exit(0);
                }

                if (!IsClient3PortNotUsed())
                {
                    MessageBox.Show("Port 7122 needed for client - used by another program!", "Fatal error!");
                    Environment.Exit(0);
                }

            }
            catch
            {

            }

        }


        private bool CheckAvailableServerPort(int port)
        {
            bool isAvailable = true;

            // Evaluate current system tcp connections. This is the same information provided
            // by the netstat command line application, just in .Net strongly-typed object
            // form.  We will look through the list, and if our port we would like to use
            // in our TcpClient is occupied, we will set isAvailable to false.
            IPGlobalProperties ipGlobalProperties = IPGlobalProperties.GetIPGlobalProperties();
            IPEndPoint[] tcpConnInfoArray = ipGlobalProperties.GetActiveTcpListeners();

            foreach (IPEndPoint endpoint in tcpConnInfoArray)
            {
                if (endpoint.Port == port)
                {
                    isAvailable = false;
                    break;
                }
            }

            return isAvailable;
        }



        bool IsClient3PortNotUsed()
        {
            return CheckAvailableServerPort(7122);
        }

        bool IsWc3PortNotUsed()
        {
            return CheckAvailableServerPort(6113);
        }

        int MaxTryCount = 8;

        private void CheckLoginStatus(object sender, EventArgs e)
        {
            var logStatus = LoginStatus();

            MaxTryCount--;

            if (MaxTryCount == 0)
            {
                //if (GlobalVariables.Wc3ProxyThread != null)
                //{
                //    GlobalVariables.Wc3ProxyThread.Interrupt();
                //    GlobalVariables.Wc3ProxyThread.Abort();
                //    GlobalVariables.War3ProxyStarted = false;
                //    GlobalVariables.Wc3ProxyThread = null;
                //}
                ((DispatcherTimer)sender).Stop();
                if (!IsRegister)
                    MessageBox.Show("Сервер недоступен!");

                LoginWindowLoginButton.IsEnabled = true;
                LoginWindowRegisterButton.IsEnabled = true;

                ResetLoginStatus();
                MaxTryCount = 8;
                return;
            }

            if (logStatus == PvPGN_Login_Status.Login_Status_FAILED)
            {
                ((DispatcherTimer)sender).Stop();

                //if (GlobalVariables.Wc3ProxyThread != null)
                //{
                //    GlobalVariables.Wc3ProxyThread.Interrupt();
                //    GlobalVariables.Wc3ProxyThread.Abort();
                //    GlobalVariables.War3ProxyStarted = false;
                //    GlobalVariables.Wc3ProxyThread = null;
                //}


                if (!IsRegister)
                    MessageBox.Show("Ошибка входа, неверный логин или пароль!");
                else
                    MessageBox.Show("Ошибка при регистрации. Пароль или имя пользователя уже занято");

                LoginWindowLoginButton.IsEnabled = true;
                LoginWindowRegisterButton.IsEnabled = true;

                ResetLoginStatus();
                MaxTryCount = 8;
                return;
            }
            else if (logStatus == PvPGN_Login_Status.Login_Status_OK)
            {
                if (!IsRegister)
                {
                    ((DispatcherTimer)sender).Stop();

                    var tmpClientWindow = new ClientWindow();
                    this.Visibility = Visibility.Hidden;
                    //tmpClientWindow.Owner = this;
                    tmpClientWindow.ShowDialog();
                    this.Visibility = Visibility.Visible;
                    EndOnClose = false;

                    LoginWindowLoginButton.IsEnabled = true;
                    LoginWindowRegisterButton.IsEnabled = true;
                }

                ResetLoginStatus();
                MaxTryCount = 8;
                return;
            }


        }

        DispatcherTimer dispatcherTimer = new DispatcherTimer();

        void InitWc3Proxy()
        {

            if (!GlobalVariables.War3ProxyStarted)
            {

                GlobalVariables.War3ProxyStarted = true;
                //try
                // {
                if (BadFirstInit)
                {
                    BadFirstInit = false;
                    dispatcherTimer.Tick += new EventHandler(CheckLoginStatus);
                }
                dispatcherTimer.Interval = new TimeSpan(0, 0, 1);

                //GlobalVariables.Wc3ProxyThread.IsBackground = !GlobalVariables.Wc3ProxyThread.IsBackground;
                UInt32 var1 = 0, var2 = 0, var3 = 0, var4 = 0;

                OtherInfoClass.ReadOtherInfo(ref var1, ref var2, ref var3, ref var4);

                SetVars(var1, var2, var3, var4);


                if (InitializeClientProxy(1, GlobalConfiguration["General"]["PathToWc3"].GetString(true, false),
                     GlobalConfiguration[GetRegionName()]["ServerIP"].GetString(true, false),
                     GlobalVariables.Username,
                     GlobalVariables.Password, 26, 6113, "26", "", IsRegister) == -5)
                {
                    var SysLog3 = new War3ProxySettings();
                    SysLog3.ShowDialog();
                }

                GlobalVariables.War3ProxyStarted = false;
                GlobalVariables.Wc3ProxyThread = null;
                //}
                //catch
                //{
                //    try
                //    {
                //        GlobalFunctions.TerminateProxy();
                //        GlobalVariables.War3ProxyStarted = false;
                //        GlobalVariables.Wc3ProxyThread = null;
                //    }
                //    catch { }

                //}
            }
        }

        void LoginRegisterClick()
        {
            MaxTryCount = 11;

            if (GlobalVariables.Wc3ProxyThread != null)
            {
                GlobalVariables.Wc3ProxyThread = null;
            }

            if (GlobalVariables.War3ProxyStarted)
            {

                try
                {
                    GlobalFunctions.TerminateProxy();
                }
                catch
                {

                }
                try
                {
                    GlobalFunctions.TerminateProxy();
                }
                catch
                {

                }


                GlobalVariables.War3ProxyStarted = false;
            }

            try
            {
                if (!GlobalVariables.Firewallpatched)
                {
                    InitFirewallPatcher();
                    GlobalVariables.Firewallpatched = true;
                }
            }
            catch { }

            GlobalVariables.Username = UsernameInput.Text;
            GlobalVariables.Password = PasswordInput.Password;
            GlobalConfiguration["General"]["Username"] = GlobalVariables.Username;

            if ((bool)LoginWindowRememberCheckbox.IsChecked)
                GlobalConfiguration["General"]["Password"] = GlobalVariables.Password.Protect();
            else
                GlobalConfiguration["General"]["Password"] = "";

            GlobalConfiguration["General"]["SavePassword"] = (bool)LoginWindowRememberCheckbox.IsChecked;

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
            GlobalConfiguration["General"]["LastAccess"] = DateTime.Now.ToShortTimeString();
            GlobalVariables.GlobalConfiguration.Save("Client.ini");

            string War3Path = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false);

            if (!File.Exists(War3Path + "Storm.dll"))
            {
                MessageBox.Show("Watal error!", "Bad war3 path");
                if (!ThisWindowLoaded)
                    return;
                var SysLog3 = new War3ProxySettings();
                // SysLog3.Owner = this;
                SysLog3.ShowDialog();
                return;
            }
            if (File.Exists(War3Path + "Client.exe"))
            {
                MessageBox.Show("Matal error!", "Bad client path(in war3 dir)");
                if (!ThisWindowLoaded)
                    return;
                var SysLog3 = new War3ProxySettings();
                // SysLog3.Owner = this;
                SysLog3.ShowDialog();
                return;
            }

            while (Process.GetProcessesByName("war3").Length > 0)
            {
                try
                {
                    Process.GetProcessesByName("war3")[0].Kill();
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
                catch
                {

                }
            }

            try
            {
                File.Delete(War3Path + "WAR.bin");
            }
            catch
            {
                MessageBox.Show("Ошибка файл WAR.bin занят");
                Environment.Exit(0);
            }

            try
            {
                File.Delete(War3Path + "War3.dll");
            }
            catch
            {
                MessageBox.Show("Ошибка файл War3.dll занят");
                Environment.Exit(0);
            }

            if (File.Exists(War3Path + "WAR.bin"))
            {
                MessageBox.Show("Banan error!");
            }

            try
            {
                File.Copy("WAR.bin", War3Path + "WAR.bin");
            }
            catch
            {

            }


            if (!File.Exists(War3Path + "WAR.bin"))
            {
                MessageBox.Show("Banan error2!");
            }
            else
            {
                var finfo = new FileInfo(War3Path + "WAR.bin");
                if (finfo.Length != 471040)
                {
                    MessageBox.Show("Banan error 3!");
                    return;
                }
                finfo = new FileInfo(Directory.GetCurrentDirectory() + "\\WAR.bin");
                if (finfo.Length != 471040)
                {
                    MessageBox.Show("Banan error 4!");
                    return;
                }
            }
            try
            {
                File.Copy("War3.dll", War3Path + "War3.dll");
            }
            catch
            {

            }


            if (UsernameInput.Text.Length > 0 && PasswordInput.Password.Length > 0)
            {
                if (GlobalVariables.Wc3ProxyThread == null)
                {
                    GlobalVariables.Wc3ProxyThread = new Thread(InitWc3Proxy);
                    GlobalVariables.Wc3ProxyThread.Priority = ThreadPriority.Highest;
                    GlobalVariables.Wc3ProxyThread.Start();
                }
            }
            else
            {
                var SysLog3 = new War3ProxySettings();
                //SysLog3.Owner = this;
                SysLog3.ShowDialog();
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {

            CheckForPatches();
            dispatcherTimer.Stop();
            GlobalVariables.MapHostList.Clear();
            LoginWindowLoginButton.IsEnabled = false;
            LoginWindowRegisterButton.IsEnabled = false;
            IsRegister = false;
            ResetLoginStatus();
            MaxTryCount = 11;

            try
            {
                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "Allow Local Files", 0);
            }
            catch { }

            try
            {
                Registry.CurrentUser.CreateSubKey(@"Software").Close();
                Registry.CurrentUser.CreateSubKey(@"Software\Blizzard Entertainment").Close();
                Registry.CurrentUser.CreateSubKey(@"Software\Blizzard Entertainment\Warcraft III").Close();
                Registry.CurrentUser.CreateSubKey(@"Software\Blizzard Entertainment\Warcraft III\String").Close();
                Registry.CurrentUser.CreateSubKey(@"Software\Blizzard Entertainment\Warcraft III\Misc").Close();

                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III\String", "userlocal", UsernameInput.Text);
                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III\Misc", "seenintromovie", 1);
            }
            catch { }
            IsRegister = false;

            dispatcherTimer.Start();

            LoginRegisterClick();
        }

        bool EndOnClose = true;

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;
            var SysLog3 = new War3ProxySettings();
            SysLog3.ShowDialog();
        }

        private void Button_Click_3(object sender, RoutedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;

        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {
            // MessageBox.Show("2");
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
            GlobalConfiguration["General"]["LastAccess"] = DateTime.Now.ToShortTimeString();
            GlobalVariables.GlobalConfiguration.Save("Client.ini");

            //if (EndOnClose && GlobalVariables.Wc3ProxyThread != null)
            //{
            //    //GlobalVariables.Wc3ProxyThread.Interrupt();
            //    //GlobalVariables.Wc3ProxyThread.Abort();
            //}

            if (GlobalVariables.Wc3ProxyThread != null)
            {
                try
                {
                    GlobalVariables.Wc3ProxyThread.IsBackground = true;
                }
                catch
                { }
            }
            Environment.Exit(0);
        }

        private void UsernameInput_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;
            GlobalVariables.Username = UsernameInput.Text;
            GlobalConfiguration["General"]["Username"] = GlobalVariables.Username;
        }

        private void PasswordInput_PasswordChanged(object sender, RoutedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;
            GlobalVariables.Password = PasswordInput.Password;
            if (!(bool)LoginWindowRememberCheckbox.IsChecked)
            {
                GlobalConfiguration["General"]["Password"] = "";
            }
            else
            {
                GlobalConfiguration["General"]["Password"] = PasswordInput.Password.Protect();
            }
            GlobalConfiguration["General"]["SavePassword"] = false;
            LoginWindowRememberCheckbox.IsChecked = false;
        }

        private void LoginWindowRememberCheckbox_Checked(object sender, RoutedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;
            if ((bool)LoginWindowRememberCheckbox.IsChecked)
            {
                GlobalConfiguration["General"]["Password"] = PasswordInput.Password.Protect();
            }
            else
            {
                GlobalConfiguration["General"]["Password"] = "";
            }
            GlobalConfiguration["General"]["SavePassword"] = (bool)LoginWindowRememberCheckbox.IsChecked;
        }

        bool IsRegister = false;

        private void RegisterButtonClick(object sender, RoutedEventArgs e)
        {

            CheckForPatches();
            GlobalVariables.MapHostList.Clear();
            dispatcherTimer.Stop();
            LoginWindowLoginButton.IsEnabled = false;
            LoginWindowRegisterButton.IsEnabled = false;
            IsRegister = true;
            ResetLoginStatus();
            IsRegister = true;
            MaxTryCount = 8;
            dispatcherTimer.Start();
            LoginRegisterClick();
        }

        private void WebsiteButtonClick(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process.Start(GetStringByRegion("WebsiteURL"));
        }

        private void LoginWindowRegionChange_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!ThisWindowLoaded)
                return;
            GlobalConfiguration["General"]["Region"] = LoginWindowRegionChange.SelectedIndex + 1;
            UpdateLanguageForCurrentWindow();
        }

        private void MetroWindow_Loaded(object sender, RoutedEventArgs e)
        {
            try
            {
                Process.GetCurrentProcess().PriorityBoostEnabled = true;
            }
            catch
            {

            }
            try
            {
                Process.GetCurrentProcess().PriorityClass = ProcessPriorityClass.High;
            }
            catch
            { }

            Application curApp = Application.Current;
            Window mainWindow = curApp.MainWindow;
            this.Left = mainWindow.Left + (mainWindow.Width - this.ActualWidth) / 2;
            this.Top = mainWindow.Top + (mainWindow.Height - this.ActualHeight) / 2;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Application.Current.MainWindow = this;
        }
    }



}
