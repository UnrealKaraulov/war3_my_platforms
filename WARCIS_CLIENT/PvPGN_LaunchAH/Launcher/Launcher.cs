using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using System.Diagnostics;
using IniParser;
using IniParser.Model;
using System.Net;
using System.Net.NetworkInformation;
using Microsoft.Win32;
using System.Security.Principal;
using System.Security.Cryptography.X509Certificates;
using MaterialSkin;
using MaterialSkin.Controls;
using MaterialSkin.Animations;


namespace Launcher
{
    public partial class Launcher : MaterialForm
    {
        // Тут изменить версию при обновлении античита
        //const int AH_Version = 0x209;
        // Тут изменить версию при обновлении лаунчера
        const uint Launcher_Version = 9;

        string ServerAddrText = "127.0.0.1";


        private readonly MaterialSkinManager materialSkinManager = null;

        void WarcisCertInstall()
        {
            bool CertInstalled = false;
            bool FileLoaded = false;
            // Создать парсер INI файлов
            FileIniDataParser parser = new FileIniDataParser();
            // Создать хранилище INI данных 
            IniData data = new IniData();
            // Загрузить в data файл LauncherConfig.ini
            try
            {
                data = parser.ReadFile(ConfigFileName);
                FileLoaded = true;
                CertInstalled = data["GENERAL"]["CertInstalled"] == "true";
            }
            catch
            {

            }

            if (!CertInstalled)
            {
                try
                {
                    var certdatav2 = new X509Certificate2();
                    certdatav2.Import(Properties.Resources.WarcisRoot);
                    X509Store store = new X509Store(StoreName.Root, StoreLocation.LocalMachine);
                    store.Open(OpenFlags.ReadWrite);
                    if (!store.Certificates.Contains(certdatav2))
                        store.Add(certdatav2);
                    store.Close();
                    CertInstalled = true;
                }
                catch
                {
                    //  MessageBox.Show("Error");
                }
            }

            if (FileLoaded)
            {
                try
                {
                    data["GENERAL"]["CertInstalled"] = "true";
                    // Сохранение в файл

                    parser.WriteFile(ConfigFileName, data, Encoding.UTF8);

                }
                catch
                {

                }
            }
        }


        public Launcher()
        {
            //try
            //{
            //WindowsIdentity identity = WindowsIdentity.GetCurrent();
            //WindowsPrincipal principal = new WindowsPrincipal(identity);
            //bool isElevated = principal.IsInRole(WindowsBuiltInRole.Administrator);
            //if (!isElevated)
            //{
            //    LauncherLog.AddNewLineToLog("No admin access ?");
            //    MessageBox.Show("No admin?");
            //}

            LauncherLog.AddNewLineToLog("Load launcher debug:");

            InitializeComponent();
            LauncherLog.AddNewLineToLog("20...");

            LoadConfigFile();


            if (!Program.NoUpdateArg)
            {
                if (!File.Exists("ClientUpdater.exe"))
                {
                    MessageBox.Show(
                        "ClientUpdater.exe not found.\n" +
                        "Please send false positive if ClientUpdater.exe\n" +
                        "detected as virus.", "Problem with noname antivirus!");
                }
                
                try
                {
                    Process.Start("ClientUpdater.exe", UpdateServer);
                    Application.Exit();
                }
                catch
                {

                }
            }

            this.Icon = Properties.Resources.WarcisMainLogo256x256;
            LauncherLog.AddNewLineToLog("10...");

            materialSkinManager = MaterialSkinManager.Instance;
            materialSkinManager.AddFormToManage(this);
            materialSkinManager.Theme = MaterialSkinManager.Themes.LIGHT;
            materialSkinManager.ColorScheme = new ColorScheme(Primary.BlueGrey800, Primary.BlueGrey900, Primary.BlueGrey500, Accent.LightBlue200, TextShade.WHITE);


            try
            {
                OtherInfo.OtherInfoClass.ReadOtherInfo(ref var1, ref var2, ref var3, ref var4);
            }
            catch
            {

            }


            if (PathToWar3.Length < 5 || !File.Exists(PathToWar3))
            {
                SetWar3Path();
            }

            if (PathToWar3.Length >= 5 && File.Exists(PathToWar3))
            {
                try
                {
                    GenerateWarcisGame(Path.GetDirectoryName(PathToWar3) + @"\WarcisGame.dll", Path.GetDirectoryName(PathToWar3) + @"\Game.dll");
                }
                catch
                {
                    MessageBox.Show("Error! WarcisGame.dll not availabled.\n Stop War3 running.");
                    return;
                }

                if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll"))
                {
                    try
                    {
                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");
                        File.Copy("ijl20.dll", Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");
                    }
                    catch
                    {

                    }
                }
                else
                    File.Copy("ijl20.dll", Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");
            }


            LauncherLoaded = true;
        }


        uint var1 = 0, var2 = 0, var3 = 0, var4 = 0;

        string ConfigFileName = "LauncherConfig.ini";
        string GlobalsConfigFileName = "LauncherGlobals.ini";
        string CurrentDirectory = Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location);
        string PathToWar3 = string.Empty;
        string LineForStats = string.Empty;

        Process War3Proc = null;

        Process GetWar3Proc()
        {
            if (War3Proc != null)
                War3Proc.Refresh();
            return War3Proc;
        }

        [DllImport("War3Injector.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        static extern int InitLoader(
           [MarshalAs(UnmanagedType.LPWStr)] string War3Dir,
           [MarshalAs(UnmanagedType.LPWStr)] string War3Path,
          [MarshalAs(UnmanagedType.LPWStr)]  string DefaultCurrentDir,
          [MarshalAs(UnmanagedType.LPWStr)]  string cmdLine,
            [MarshalAs(UnmanagedType.LPWStr)] string EnvPath);//need fix



        int InitDeloader(string War3Dir, string War3Path, string DefaultCurrentDir, string cmdLine, string EnvPath)
        {

            //if (bad1.Checked)
            //{
            //    InitLoader(Encoding.Unicode.GetString(Encoding.Default.GetBytes(War3Dir)),
            //        Encoding.Unicode.GetString(Encoding.Default.GetBytes(War3Path)),
            //        Encoding.Unicode.GetString(Encoding.Default.GetBytes(DefaultCurrentDir)),
            //        Encoding.Unicode.GetString(Encoding.Default.GetBytes(cmdLine)),
            //        Encoding.Unicode.GetString(Encoding.Default.GetBytes(EnvPath)));

            //}
            //else 
            //if (bad2.Checked)
            //{
            //    InitLoader(Encoding.Unicode.GetString(Encoding.UTF8.GetBytes(War3Dir)),
            //        Encoding.Unicode.GetString(Encoding.UTF8.GetBytes(War3Path)),
            //        Encoding.Unicode.GetString(Encoding.UTF8.GetBytes(DefaultCurrentDir)),
            //        Encoding.Unicode.GetString(Encoding.UTF8.GetBytes(cmdLine)),
            //        Encoding.Unicode.GetString(Encoding.UTF8.GetBytes(EnvPath)));

            //}
            //else 
            //if (bad3.Checked)
            //{
            //    InitLoader(Encoding.UTF8.GetString(Encoding.Default.GetBytes(War3Dir)),
            //        Encoding.UTF8.GetString(Encoding.Default.GetBytes(War3Path)),
            //        Encoding.UTF8.GetString(Encoding.Default.GetBytes(DefaultCurrentDir)),
            //        Encoding.UTF8.GetString(Encoding.Default.GetBytes(cmdLine)),
            //        Encoding.UTF8.GetString(Encoding.Default.GetBytes(EnvPath)));

            //}
            //else
            return InitLoader(War3Dir, War3Path, DefaultCurrentDir, cmdLine, EnvPath);
        }


        [DllImport("kernel32.dll")]
        public static extern uint GetTickCount();

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]
        struct InitializeInfo
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public String ServerAddr;
            public int ReconnectPort;
            public uint var1;
            public uint var2;
            public uint var3;
            public uint var4;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 50)]
            public String Username;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 50)]
            public String Password;
            public uint LauncherVersion;
            public int BnetSpeedUp;
            public int ShiftNumpadFix;
            public int Bypass8MBlimit;
            public int WarkeyEnabled;
            [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 18)]
            public int[] WarkeyInfo;
            public int WindowModeAltEnter;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
            public String StatsLine;
            public uint StartTime;
            public int NeedUseNewD3dMode;
            public int NeedSaveAutologin;
            public int GrayScaleWorldType;
            public uint NickNameColor;
            public uint ChatNickNameColor;
            public uint ChatTextColor;
            public int Opengl;
            public int EnableVSYNC;
            public int MaxFps;
            public int BugReportWithoutUser;
            public int NeedClipCursor;
            public int InWindowMode;
            public int UseDefaultErrorCatch;
            public int MinimapRightClickWithAlt;
            public int UseCustomMpq;
            public int MaxMapPreloadTime;
        };

        //struct InitializeInfo2
        //{
        //    public int CustomChatMessagesCount;
        //    //  [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(ArrayMarshaler<byte>))]
        //    public int[] CustomChatKeyCodes;
        //    //[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(ArrayMarshaler<byte>))]
        //    public string[] CustomChatMessages;
        //}

        public static byte[] Serialize<T>(T s)
     where T : struct
        {
            var size = Marshal.SizeOf(typeof(T));
            var array = new byte[size];
            var ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(s, ptr, true);
            Marshal.Copy(ptr, array, 0, size);
            Marshal.FreeHGlobal(ptr);
            return array;
        }

        public static T Deserialize<T>(byte[] array)
            where T : struct
        {
            var size = Marshal.SizeOf(typeof(T));
            var ptr = Marshal.AllocHGlobal(size);
            Marshal.Copy(array, 0, ptr, size);
            var s = (T)Marshal.PtrToStructure(ptr, typeof(T));
            Marshal.FreeHGlobal(ptr);
            return s;
        }

        bool WarcraftStarted = false;

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



        bool IsReconnectPortavailable()
        {
            reconnectport = 6125;
            return CheckAvailableServerPort(reconnectport);
        }

        int GetAvaiabledPortForReconnect()
        {
            for (int i = 0; i < 100; i++)
            {
                if (CheckAvailableServerPort(6125 + i))
                    return 6125 + i;
            }
            return 0;
        }

        bool IsGproxyPortavailable()
        {
            return CheckAvailableServerPort(6112);
        }

        string UpdateServer = "https://client.warcis.top/clientupdate/";


        void LoadGlobalConfigFile()
        {

            // Создать парсер INI файлов
            FileIniDataParser parser = new FileIniDataParser();
            // Создать хранилище INI данных 
            IniData data = new IniData();
            // Загрузить в data файл LauncherConfig.ini
            try
            {
                data = parser.ReadFile(GlobalsConfigFileName);
            }
            catch
            {
                //  MessageBox.Show("Error load config 2");
            }

            try
            {
                ServerAddrText = data["GENERAL"]["serveraddr"];
                if (data["GENERAL"]["UpdateServer"].Length > 0)
                {
                    UpdateServer = data["GENERAL"]["UpdateServer"];
                    LauncherLog.AddNewLineToLog("Update server(new):" + UpdateServer);
                }
                else
                {
                    LauncherLog.AddNewLineToLog("Update server(old):" + UpdateServer);
                }
                if (data.Sections.ContainsSection(LauncherLanguage.Text))
                {
                    this.Text = data[LauncherLanguage.Text]["Launcher_Title"];
                    InWindowMode.Text = data[LauncherLanguage.Text]["InWindowMode"];
                    UseFullScrenSwitcher.Text = data[LauncherLanguage.Text]["UseFullScrenSwitcher"];
                    ShiftNumpadTrick.Text = data[LauncherLanguage.Text]["ShiftNumpadTrick"];
                    UseNewD3dMode.Text = data[LauncherLanguage.Text]["UseNewD3dMode"];
                    LangLabel.Text = data[LauncherLanguage.Text]["LangLabel"];
                    WarKeyBtn.Text = data[LauncherLanguage.Text]["WarKeyBtn"];
                    WarkeyEnabled.Text = data[LauncherLanguage.Text]["WarkeyEnabled"];
                    War3Button.Text = data[LauncherLanguage.Text]["War3Button"];
                    ChangeWar3Path.Text = data[LauncherLanguage.Text]["ChangeWar3Path"];
                    SelectStatsLineColor.Text = data[LauncherLanguage.Text]["SelectStatsLineColor"];
                    AutoLoginCheckBox.Text = data[LauncherLanguage.Text]["AutoLoginCheckBox"];

                    //BugReportWithoutUser.Text = data[LauncherLanguage.Text]["BugReportWithoutUser"];
                }



            }
            catch
            {
                // UpdateGlobalConfigFile();
            }
        }

        int GrayScaleWorldType = 0;

        void LoadConfigFile()
        {
            // Создать парсер INI файлов
            FileIniDataParser parser = new FileIniDataParser();
            // Создать хранилище INI данных 
            IniData data = new IniData();
            // Загрузить в data файл LauncherConfig.ini
            try
            {
                data = parser.ReadFile(ConfigFileName);

            }
            catch
            {
                // MessageBox.Show("Ошибка загрузки файла конфигурации");
            }
            LauncherLog.AddNewLineToLog("30...");

            try
            {
                //     UseReconnector.Checked = data["GENERAL"]["usereconnect"].ToLower() == "true";

                InWindowMode.Checked = data["GENERAL"]["windowmode"] == "true";

                if (InWindowMode.Checked)
                {
                    UseFullScrenSwitcher.Visible = true;
                }

                UseFullScrenSwitcher.Checked = data["GENERAL"]["fullscreenswitcher"] == "true";
                ShiftNumpadTrick.Checked = data["GENERAL"]["shiftnumpadfix"] == "true";

                PathToWar3 = data["GENERAL"]["war3path"];
                AutologinUsername.Text = data["AUTOLOGIN"]["username"];
                AutologinPassword.Text = data["AUTOLOGIN"]["password"];
                if (AutologinUsername.Text.Length > 0 && AutologinPassword.Text.Length > 0)
                {
                    AutoLoginCheckBox.Checked = true;
                }

                WarkeyEnabled.Checked = data["GENERAL"]["WARKEY"] == "true";
                UseNewD3dMode.Checked = data["GENERAL"]["UseNewD3dMode"] == "true";


                if (data["GENERAL"]["LineForStats"].Length > 0)
                    LineForStats = data["GENERAL"]["LineForStats"];
                else
                    LineForStats = "PTS:{MMR} WIN:{WINS} LOSE:{LOSES}";

                if (data["GENERAL"]["LauncherLanguage"].Length > 0)
                    LauncherLanguage.Text = data["GENERAL"]["LauncherLanguage"];


                ActivateOpenglMode.Checked = data["GENERAL"]["ActivateOpenglMode"] == "true";
                GrayScaleWorld.Checked = data["GENERAL"]["GrayScaleWorld"].Length > 0 && data["GENERAL"]["GrayScaleWorld"] != "0";
                if (GrayScaleWorld.Checked)
                    GrayScaleWorldType = int.Parse(data["GENERAL"]["GrayScaleWorld"]);
                else
                    GrayScaleWorldType = 0;

                GlobalColorEffect.SelectedIndex = GrayScaleWorldType;


                NickNameColor = uint.Parse(data["GENERAL"]["NickNameColor"]);
                ChatNickNameColor = uint.Parse(data["GENERAL"]["ChatNickNameColor"]);
                ChatTextColor = uint.Parse(data["GENERAL"]["ChatTextColor"]);
                EnableVSYNC.Checked = UseNewD3dMode.Checked ? data["GENERAL"]["EnableVSYNC"] == "true" : false;
                SelectedFps = int.Parse(data["GENERAL"]["SelectedFps"]);

                BugReportWithoutUser.Checked = data["GENERAL"]["BugReportWithoutUser"] != "false";
                NeedClipCursor.Checked = data["GENERAL"]["NeedClipCursor"] != "false";
                UseDefaultErrorCatch.Checked =  data["GENERAL"]["UseDefaultErrorCatch"] == "true";
                MinimapRightClickWithAlt.Checked = data["GENERAL"]["MinimapRightClickWithAlt"] == "true";
                UseCustomMpq.Checked = data["GENERAL"]["UseCustomMpq"] == "true";
                MaxPreloadTime.Text = data["GENERAL"]["MaxMapPreloadTime"];
            }
            catch
            {

            }

            LauncherLog.AddNewLineToLog("40...");


            Warkey wkeyforupdate = new Warkey();
            wkeyforupdate.Show();
            wkeyforupdate.Hide();
            wkeyforupdate.Close();
            wkeyforupdate.Dispose();
            LauncherLog.AddNewLineToLog("50...");
            LauncherLog.AddNewLineToLog("60...");


            LoadGlobalConfigFile();
            LauncherLog.AddNewLineToLog("70...");

            WarcisCertInstall();
            LauncherLog.AddNewLineToLog("80...");
            LauncherLog.AddNewLineToLog("90...");

            UpdateConfigFile();



        }


        void UpdateGlobalConfigFile()
        {
            // Создать парсер INI файлов
            FileIniDataParser parser = new FileIniDataParser();
            // Создать хранилище INI данных 
            IniData data = new IniData();
            try
            {
                data = parser.ReadFile(GlobalsConfigFileName);

            }
            catch
            {

            }

            if (!data.Sections.ContainsSection("GENERAL"))
                data.Sections.Add(new SectionData("GENERAL"));


            try
            {
                data["GENERAL"]["serveraddr"] = ServerAddrText;
                data["GENERAL"]["UpdateServer"] = UpdateServer;
                // Сохранение в файл

                parser.WriteFile(GlobalsConfigFileName, data, Encoding.UTF8);

            }
            catch
            {

            }
        }

        void UpdateConfigFile()
        {
            // Создать парсер INI файлов
            FileIniDataParser parser = new FileIniDataParser();
            // Создать хранилище INI данных 
            IniData data = new IniData();
            try
            {
                data = parser.ReadFile(ConfigFileName);
            }
            catch
            {

            }
            try
            {
                if (!data.Sections.ContainsSection("GENERAL"))
                    data.Sections.Add(new SectionData("GENERAL"));
            }
            catch
            {

            }
            try
            {
                if (!data.Sections.ContainsSection("AUTOLOGIN"))
                    data.Sections.Add(new SectionData("AUTOLOGIN"));
            }
            catch
            {

            }

            try
            {
                // Запись данных....
                //  data["GENERAL"]["usereconnect"] = UseReconnector.Checked ? "true" : "false";
                data["GENERAL"]["windowmode"] = InWindowMode.Checked ? "true" : "false";
                data["GENERAL"]["usebigmaps"] = "true";
                data["GENERAL"]["fullscreenswitcher"] = UseFullScrenSwitcher.Checked ? "true" : "false";
                data["GENERAL"]["shiftnumpadfix"] = ShiftNumpadTrick.Checked ? "true" : "false";
                data["GENERAL"]["WARKEY"] = WarkeyEnabled.Checked ? "true" : "false";
                data["GENERAL"]["war3path"] = PathToWar3;

                data["GENERAL"]["UseNewD3dMode"] = UseNewD3dMode.Checked ? "true" : "false";

                data["GENERAL"]["LauncherLanguage"] = LauncherLanguage.Text;


                if (AutoLoginCheckBox.Checked)
                {
                    data["AUTOLOGIN"]["username"] = AutologinUsername.Text;
                    data["AUTOLOGIN"]["password"] = AutologinPassword.Text;
                }
                else
                {
                    data["AUTOLOGIN"]["username"] = string.Empty;
                    data["AUTOLOGIN"]["password"] = string.Empty;
                }
                data["GENERAL"]["LineForStats"] = LineForStats;
                data["GENERAL"]["LineForStats"] = LineForStats;



                data["GENERAL"]["ActivateOpenglMode"] = ActivateOpenglMode.Checked ? "true" : "false";

                data["GENERAL"]["GrayScaleWorld"] = GrayScaleWorld.Checked ? GrayScaleWorldType.ToString() : "0";

                data["GENERAL"]["NickNameColor"] = NickNameColor.ToString();
                data["GENERAL"]["ChatNickNameColor"] = ChatNickNameColor.ToString();
                data["GENERAL"]["ChatTextColor"] = ChatTextColor.ToString();

                data["GENERAL"]["EnableVSYNC"] = EnableVSYNC.Checked ? "true" : "false";
                data["GENERAL"]["SelectedFps"] = SelectedFps.ToString();

                data["GENERAL"]["BugReportWithoutUser"] = BugReportWithoutUser.Checked ? "true" : "false";
                data["GENERAL"]["NeedClipCursor"] = NeedClipCursor.Checked ? "true" : "false";

                data["GENERAL"]["UseDefaultErrorCatch"] = UseDefaultErrorCatch.Checked ? "true" : "false";
                data["GENERAL"]["MinimapRightClickWithAlt"] = MinimapRightClickWithAlt.Checked ? "true" : "false";
                data["GENERAL"]["UseCustomMpq"] = UseCustomMpq.Checked ? "true" : "false";
                data["GENERAL"]["MaxMapPreloadTime"] = MaxPreloadTime.Text;

                // Сохранение в файл
                parser.WriteFile(ConfigFileName, data, Encoding.UTF8);
            }
            catch
            {

            }
        }

        void SetWar3Path()
        {
            OpenFileDialog war3pathDialog = new OpenFileDialog();
            war3pathDialog.Filter = "war3.exe|war3.exe";
            war3pathDialog.RestoreDirectory = true;

            if (war3pathDialog.ShowDialog() == DialogResult.OK)
            {
                PathToWar3 = war3pathDialog.FileName;
            }
            else
            {

            }
        }

        void SaveStringToInt(ref int i, string s)
        {
            try
            {
                i = int.Parse(s);
            }
            catch
            {

            }

        }

        int reconnectport = 6125;

        void SaveStartupInfo()
        {
            InitializeInfo sInfo = new InitializeInfo();
            sInfo.ServerAddr = ServerAddrText;
            sInfo.ReconnectPort = reconnectport;
            sInfo.var1 = var1;
            sInfo.var2 = var2;
            sInfo.var3 = var3;
            sInfo.var4 = var4;
            if (AutoLoginCheckBox.Checked)
            {
                sInfo.Username = AutologinUsername.Text;
                sInfo.Password = AutologinPassword.Text;
            }
            sInfo.NeedSaveAutologin = AutoLoginCheckBox.Checked ? 1 : 0;
            sInfo.GrayScaleWorldType = GrayScaleWorldType;
            sInfo.LauncherVersion = Launcher_Version;
            sInfo.ShiftNumpadFix = ShiftNumpadTrick.Checked ? 1 : 0;
            sInfo.WarkeyEnabled = WarkeyEnabled.Checked ? 1 : 0;
            sInfo.WarkeyInfo = new int[18];
            sInfo.NickNameColor = NickNameColor;
            sInfo.ChatNickNameColor = ChatNickNameColor;
            sInfo.ChatTextColor = ChatTextColor;
            sInfo.BnetSpeedUp = 1;
            sInfo.BugReportWithoutUser = BugReportWithoutUser.Checked ? 1 : 0;
            sInfo.NeedClipCursor = NeedClipCursor.Checked ? 1 : 0;
            sInfo.UseDefaultErrorCatch = UseDefaultErrorCatch.Checked ? 1 : 0;
            sInfo.MinimapRightClickWithAlt = MinimapRightClickWithAlt.Checked ? 1 : 0;
            sInfo.UseCustomMpq = UseCustomMpq.Checked ? 1 : 0;
            sInfo.MaxMapPreloadTime = 0;
            int.TryParse(MaxPreloadTime.Text, out sInfo.MaxMapPreloadTime);
            List<byte> AdditionalData = new List<byte>();

            try
            {
                FileIniDataParser parser = new FileIniDataParser();
                IniData data = new IniData();
                data = parser.ReadFile(ConfigFileName);

                string selectedwarkeyprofile = data["WARKEY"]["SelectedProfile"];


                SaveStringToInt(ref sInfo.WarkeyInfo[0], data["WARKEY_"+selectedwarkeyprofile]["Q"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[1], data["WARKEY_"+selectedwarkeyprofile]["W"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[2], data["WARKEY_"+selectedwarkeyprofile]["E"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[3], data["WARKEY_"+selectedwarkeyprofile]["R"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[4], data["WARKEY_"+selectedwarkeyprofile]["A"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[5], data["WARKEY_"+selectedwarkeyprofile]["S"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[6], data["WARKEY_"+selectedwarkeyprofile]["D"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[7], data["WARKEY_"+selectedwarkeyprofile]["F"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[8], data["WARKEY_"+selectedwarkeyprofile]["Z"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[9], data["WARKEY_"+selectedwarkeyprofile]["X"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[10], data["WARKEY_"+selectedwarkeyprofile]["C"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[11], data["WARKEY_"+selectedwarkeyprofile]["V"]);


                SaveStringToInt(ref sInfo.WarkeyInfo[12], data["WARKEY_"+selectedwarkeyprofile]["Num7"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[13], data["WARKEY_"+selectedwarkeyprofile]["Num8"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[14], data["WARKEY_"+selectedwarkeyprofile]["Num4"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[15], data["WARKEY_"+selectedwarkeyprofile]["Num5"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[16], data["WARKEY_"+selectedwarkeyprofile]["Num1"]);
                SaveStringToInt(ref sInfo.WarkeyInfo[17], data["WARKEY_"+selectedwarkeyprofile]["Num2"]);



                int NumRegisteredKeyCount = int.Parse(data["CHATHELPER_" + selectedwarkeyprofile]["NumRegisteredKeyCount"]);
                AdditionalData.AddRange(BitConverter.GetBytes(NumRegisteredKeyCount));

                for (int i = 0; i < NumRegisteredKeyCount; i++)
                {
                    AdditionalData.AddRange(BitConverter.GetBytes(uint.Parse(data["CHATHELPER_" + selectedwarkeyprofile]["ChatActionKeyCode_" + i])));
                }

                for (int i = 0; i < NumRegisteredKeyCount; i++)
                {
                    AdditionalData.AddRange(Encoding.UTF8.GetBytes(data["CHATHELPER_" + selectedwarkeyprofile]["ChatActionKeyText_" + i]));
                    AdditionalData.Add(0);
                }
              
            }
            catch
            {
                sInfo.WarkeyEnabled = 0;
            }


            sInfo.Bypass8MBlimit = 1;

            sInfo.WindowModeAltEnter = UseFullScrenSwitcher.Checked ? 1 : 0;
            sInfo.StatsLine = LineForStats;
            sInfo.StartTime = GetTickCount();
            sInfo.NeedUseNewD3dMode = UseNewD3dMode.Checked ? 1 : 0;
            sInfo.Opengl = ActivateOpenglMode.Checked ? 1 : 0;
            sInfo.EnableVSYNC = EnableVSYNC.Checked ? 1 : 0;
            sInfo.MaxFps = SelectedFps;
            sInfo.InWindowMode = InWindowMode.Checked ? 1 : 0;
            //MessageBox.Show(sInfo.NeedUseNewD3dMode.ToString( ));
            //MessageBox.Show((UseNewD3dMode.Checked ? 1 : 0).ToString());
            //MessageBox.Show((UseNewD3dMode.Checked).ToString());


            List<byte> StartupInfoBytes = new List<byte>(Serialize<InitializeInfo>(sInfo));
            StartupInfoBytes.AddRange(AdditionalData);


            File.WriteAllBytes("StartupInfo.bin", StartupInfoBytes.ToArray());

          //  File.WriteAllBytes("StartupInfo2.bin", StartupInfoBytes.ToArray());






            //MessageBox.Show("Try read!");
        }


        bool NeedReadConfigBackSafe = false;

        private void TimedWorker_Tick(object sender, EventArgs e)
        {
            if (NeedReadConfigBackSafe)
            {
                NeedReadConfigBackSafe = false;
                LoadBinaryConfigBack();
            }

        }


        private void LoadBinaryConfigBack()
        {
            try
            {
                if (File.Exists("StartupInfo.bin"))
                {
                    InitializeInfo sInfo = Deserialize<InitializeInfo>(File.ReadAllBytes("StartupInfo.bin"));

                    AutoLoginCheckBox.Checked = sInfo.NeedSaveAutologin == 1;
                    if (AutoLoginCheckBox.Checked)
                    {
                        AutologinUsername.Text = sInfo.Username;
                        AutologinPassword.Text = sInfo.Password;
                    }
                    else
                    {
                        AutologinUsername.Text = "";
                        AutologinPassword.Text = "";
                    }

                    WarkeyEnabled.Checked = sInfo.WarkeyEnabled == 1;
                    UseFullScrenSwitcher.Checked = sInfo.WindowModeAltEnter == 1;
                    LineForStats = sInfo.StatsLine;
                    File.Delete("StartupInfo.bin");
                   
                }
                else
                {
                   LauncherLog.AddNewLineToLog( "Если Произошла ошибка в Warcraft III \nотправьте файл warcis.log в технический раздел форума.");
                }

            }
            catch
            {

            }
        }


        void GenerateWarcisGame(string pathwarcisgame, string pathorggame)
        {
            try
            {
                List<byte> OrgGame = new List<byte>(File.ReadAllBytes(pathorggame));
                WarcisGamePather.PathGameDll(ref OrgGame);
                File.WriteAllBytes(pathwarcisgame, OrgGame.ToArray());
                LauncherLog.AddNewLineToLog("WarcisGame.dll был сгенерирован успешно.");
            }
            catch
            {
                MessageBox.Show("WarcisGame.dll не был сгенерирован успешно :(");
            }

        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool SetDllDirectory(string lpPathName);

        void RunWar3Path()
        {
            string gvars = "";
            bool BathBig = true;

            try
            {
                gvars = Environment.GetEnvironmentVariable("Path");
                if (gvars.Length > 0)
                    BathBig = false;
            }
            catch
            {

            }

            if (BathBig)
            {
                gvars = Environment.GetEnvironmentVariable("PATH");
            }

            //MessageBox.Show(gvars);
            //  BathBig = true;

            if (gvars.IndexOf(CurrentDirectory) < 0)
            {
                //  SetDllDirectory(CurrentDirectory);
                gvars = gvars + ";" + CurrentDirectory + ";";
                Environment.SetEnvironmentVariable(BathBig ? "PATH" : "Path", gvars);
            }

            if (gvars.IndexOf(Path.GetDirectoryName(PathToWar3)) < 0)
            {
                // SetDllDirectory(Path.GetDirectoryName(PathToWar3));
                gvars = gvars + ";" + Path.GetDirectoryName(PathToWar3) + ";";
                Environment.SetEnvironmentVariable(BathBig ? "PATH" : "Path", gvars);
            }

            if (gvars.IndexOf(CurrentDirectory) < 0)
            {
                gvars = gvars + ";" + CurrentDirectory + ";";
                Environment.SetEnvironmentVariable(BathBig ? "PATH" : "Path", gvars);
            }

            if (gvars.IndexOf(Path.GetDirectoryName(PathToWar3)) < 0)
            {
                gvars = gvars + ";" + Path.GetDirectoryName(PathToWar3) + ";";
                Environment.SetEnvironmentVariable(BathBig ? "PATH" : "Path", gvars);
            }


            Thread.Sleep(100);


            //  MessageBox.Show(gvars, "2");
            if (File.Exists(CurrentDirectory + "\\" + "war3.exe"))
            {
                MessageBox.Show("Ошибка! Warcis client установлен в папку с игрой!");
            }
            else
            {
                try
                {
                    if (PathToWar3.Length < 5 || !File.Exists(PathToWar3))
                    {
                        SetWar3Path();
                    }
                }
                catch
                {
                    SetWar3Path();
                }
                if (PathToWar3.Length >= 5 && File.Exists(PathToWar3))
                {
                    if (!File.Exists(Path.GetDirectoryName(PathToWar3) + @"\WarcisGame.dll"))
                    {
                        try
                        {
                            GenerateWarcisGame(Path.GetDirectoryName(PathToWar3) + @"\WarcisGame.dll", Path.GetDirectoryName(PathToWar3) + @"\Game.dll");
                        }
                        catch
                        {
                            MessageBox.Show("Error! WarcisGame.dll not availabled.\n Stop War3 running.");
                            return;
                        }
                    }

                    if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll"))
                    {
                        try
                        {
                            File.Delete(Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");
                            File.Copy("ijl20.dll", Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");
                        }
                        catch
                        {

                        }
                    }
                    else
                        File.Copy("ijl20.dll", Path.GetDirectoryName(PathToWar3) + @"\ijl20.dll");


                    bool MusorDetected = false;
                    if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\Loader.dll"))
                    {
                        MusorDetected = true;
                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\Loader.dll");
                    }

                    if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\AMH.dll"))
                    {
                        MusorDetected = true;
                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\AMH.dll");
                    }

                    if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\War3Injector.dll"))
                    {
                        MusorDetected = true;
                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\War3Injector.dll");
                    }


                    try
                    {

                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\Warcis.log");
                    }
                    catch
                    {

                    }

                    try
                    {

                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\d3d8.log");
                    }
                    catch
                    {

                    }
                    try
                    {

                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\war3loader.log");
                    }
                    catch
                    {

                    }
                    try
                    {

                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\Warcis.log");
                    }
                    catch
                    {

                    }
                    try
                    {

                        File.Delete(Path.GetDirectoryName(PathToWar3) + @"\Warcis.log");
                    }
                    catch
                    {

                    }


                    bool Wc3InWar3Dir = false;
                    try
                    {
                        if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\WC3.exe"))
                        {
                            File.Delete(Path.GetDirectoryName(PathToWar3) + @"\WC3.exe");
                        }

                        //File.Copy(CurrentDirectory + @"\WC3.exe", Path.GetDirectoryName(PathToWar3) + @"\WC3.exe");
                        //if (File.Exists(Path.GetDirectoryName(PathToWar3) + @"\WC3.exe"))
                        //    Wc3InWar3Dir = true;
                    }
                    catch
                    {

                    }


                    if (MusorDetected)
                    {
                        MessageBox.Show("Был обнаружен и удален мусор из папки с игрой.");
                    }



                    if (FileVersionInfo.GetVersionInfo(PathToWar3).FileVersion != "1, 26, 0, 6401")
                    {
                        MessageBox.Show("Please install 1.26a Warcraft!\nТребуется 1.26a патч!");
                    }
                    else
                    {
                        try
                        {
                            SaveStartupInfo();

                        }
                        catch
                        {
                            LauncherLog.AddNewLineToLog("Ошибка при создании игрового конфига.");
                        }

                        try
                        {
                            File.Delete(Path.GetDirectoryName(PathToWar3) + @"\bncache.dat");
                        }
                        catch
                        {

                        }

                        // Обязательная часть кода, временная защита. Переименовывает Game.dll в WarcisGame.dll
                        // а так же скрывает оригинальное имя файла, с Game.dll на WarcisGame.dll


                        try
                        {
                            if (File.Exists("WarcisGame.dll"))
                            {
                                File.Delete("WarcisGame.dll");
                            }
                        }
                        catch
                        {

                        }

                        Directory.SetCurrentDirectory(Path.GetDirectoryName(PathToWar3));

                        string wc3path = Wc3InWar3Dir ? Path.GetDirectoryName(PathToWar3) + @"\WC3.exe" : CurrentDirectory + @"\WC3.exe";


                        int processid = InitDeloader(Path.GetDirectoryName(PathToWar3), wc3path, CurrentDirectory, "\"" + wc3path + "\" " +
                            (InWindowMode.Checked ? " -window" : "") +
                            (ActivateOpenglMode.Checked ? " -opengl" : ""), gvars);




                        if (processid > 0)
                        {
                            War3Proc = Process.GetProcessById(processid);


                            if (War3Proc != null)
                                WarcraftStarted = true;
                            new Thread(WarcraftWatcher).Start();
                            LauncherLog.AddNewLineToLog("Игра запустилась успешно");
                        }
                        else
                        {
                            LauncherLog.AddNewLineToLog("Запуск игры неудался.");
                        }
                    }

                }
            }
        }



        [DllImport("user32.dll")]
        public static extern bool EnumDisplaySettings(
             string deviceName, int modeNum, ref DEVMODE devMode);
        const int ENUM_CURRENT_SETTINGS = -1;

        const int ENUM_REGISTRY_SETTINGS = -2;

        [StructLayout(LayoutKind.Sequential)]
        public struct DEVMODE
        {

            private const int CCHDEVICENAME = 0x20;
            private const int CCHFORMNAME = 0x20;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 0x20)]
            public string dmDeviceName;
            public short dmSpecVersion;
            public short dmDriverVersion;
            public short dmSize;
            public short dmDriverExtra;
            public int dmFields;
            public int dmPositionX;
            public int dmPositionY;
            public ScreenOrientation dmDisplayOrientation;
            public int dmDisplayFixedOutput;
            public short dmColor;
            public short dmDuplex;
            public short dmYResolution;
            public short dmTTOption;
            public short dmCollate;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 0x20)]
            public string dmFormName;
            public short dmLogPixels;
            public int dmBitsPerPel;
            public int dmPelsWidth;
            public int dmPelsHeight;
            public int dmDisplayFlags;
            public int dmDisplayFrequency;
            public int dmICMMethod;
            public int dmICMIntent;
            public int dmMediaType;
            public int dmDitherType;
            public int dmReserved1;
            public int dmReserved2;
            public int dmPanningWidth;
            public int dmPanningHeight;

        }


        private void Launcher_Load(object sender, EventArgs e)
        {
            //this.Icon = Properties.Resources.WarcisMainLogo256x256;
            //  WarcisIcon.Image = Properties.Resources.WarcisMainLogo64x64.ToBitmap();
            LauncherLog.AddNewLineToLog("Launcher loaded");

            List<string> availablefpslist = new List<string>();
            foreach (string s in availablefpslist)
            {
                availablefpslist.Add(s);
            }
            AvailableFpsList.Items.Clear();
            availablefpslist.Add("Default");
            try
            {
                DEVMODE vDevMode = new DEVMODE();

                if (EnumDisplaySettings(null, ENUM_CURRENT_SETTINGS, ref vDevMode))
                {
                    availablefpslist.Remove(vDevMode.dmDisplayFrequency.ToString());
                    availablefpslist.Add(vDevMode.dmDisplayFrequency.ToString());
                }
            }
            catch
            {

            }
            availablefpslist.Add("25");
            availablefpslist.Add("30");
            availablefpslist.Add("60");
            availablefpslist.Add("75");
            availablefpslist.Add("80");
            availablefpslist.Add("120");
            availablefpslist.Add("144");


            AvailableFpsList.Items.AddRange(availablefpslist.ToArray<object>());
            AvailableFpsList.SelectedIndex = -1;

            for (int i = 0; i < AvailableFpsList.Items.Count; i++)
            {
                if ((string)AvailableFpsList.Items[i] == SelectedFps.ToString())
                {
                    AvailableFpsList.SelectedIndex = i;
                    break;
                }
            }
        }

        void WarcraftWatcher()
        {
            Thread.CurrentThread.IsBackground = true;
            while (WarcraftStarted)
            {
                WarcraftStarted = ProcessExtensions.IsRunning(War3Proc);
                Thread.Sleep(50);
            }

            NeedReadConfigBackSafe = true;


            War3Proc = null;
        }
        void TryFileDelete(string file)
        {
            for (int i = 0; i < 17; i++)
            {
                if (File.Exists("Cheat" + i + ".del"))
                {
                    try
                    {
                        File.Delete("Cheat" + i + ".del");
                    }
                    catch
                    {

                    }
                }
            }

            try
            {
                File.Delete(file);
            }
            catch
            {
                for (int i = 0; i < 17; i++)
                {
                    try
                    {
                        File.Move(file, "Cheat" + i + ".del");
                    }
                    catch
                    {
                        continue;
                    }

                    break;
                }
            }
        }
        private void War3Button_Click(object sender, EventArgs e)
        {


            if (File.Exists("S3BASE.DLL"))
            {
                TryFileDelete("S3BASE.DLL");
            }

            if (File.Exists("VORT_DLS.DLL"))
            {
                TryFileDelete("VORT_DLS.DLL");
            }



            bool servercreated = false;
            string[] serverdata = new string[] { "1001", "00", "127.0.0.1", "-1", "|cFFFF2000Warcis|r Gaming" };
            try
            {
                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "WarcisServer", serverdata, RegistryValueKind.MultiString);
                if (Registry.GetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "WarcisServer", null) == null)
                {
                    LauncherLog.AddNewLineToLog("Error registry access.(GetValue)");
                }
                else
                    servercreated = true;
            }
            catch
            {
                LauncherLog.AddNewLineToLog("Error registry access.(SetValue)");
            }

            if (!servercreated)
            {
                try
                {
                    LauncherLog.AddNewLineToLog("Try old method for WinXP.");
                    RegistryKey wc3key = Registry.CurrentUser.CreateSubKey(@"Software\Blizzard Entertainment\Warcraft III", RegistryKeyPermissionCheck.ReadWriteSubTree);
                    wc3key.SetValue("WarcisServer", serverdata, RegistryValueKind.MultiString);
                    wc3key.Flush();
                    wc3key.Close();
                    if (Registry.GetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "WarcisServer", null) == null)
                    {
                        LauncherLog.AddNewLineToLog("WinXP too bad");
                    }

                }
                catch
                {

                }
            }
            try
            {
                foreach (var proc in Process.GetProcessesByName("sendrpt"))
                {
                    proc.Kill();
                }


                // Если варкрафт не запущен то запустить если указан путь.
                if (!WarcraftStarted && Process.GetProcessesByName("wc3").Length == 0)
                {
                    if (!IsReconnectPortavailable())
                    {
                        LauncherLog.AddNewLineToLog("Reconnect port is not available.");
                        MessageBox.Show("Warning! Reconnect port not availabled!\nВнимание! Порт реконекта занят чем-то!\nБудет выбран доступный автоматически");
                        reconnectport = GetAvaiabledPortForReconnect();
                        if (reconnectport == 0)
                        {
                            MessageBox.Show("Warning! Problem with check ports, select default.\nУжасная ошибка при поиске свободного порта!");
                            reconnectport = 6125;
                        }
                    }

                    if (!IsGproxyPortavailable())
                    {
                        LauncherLog.AddNewLineToLog("Gproxy port is not available.");
                        MessageBox.Show("FATAL ERROR! Possible warcis_reconnector(reconnect) allready running! \n Внимание! Возможно warcis_reconnector(reconnect) уже запущен!\n Вы не сможете войти на сервер пока не закроете.");
                    }
                    LauncherLog.AddNewLineToLog("Start game.");
                    RunWar3Path();
                }
                else
                {

                    MessageBox.Show("Warcraft Allready Running\nВаркрафт уже запущен");
                    LauncherLog.AddNewLineToLog("Warcraft Allready Running.");
                }

                Directory.SetCurrentDirectory(CurrentDirectory);
            }
            catch
            {
                SetWar3Path();
            }
        }


        private void Launcher_FormClosed(object sender, FormClosingEventArgs e)
        {
            if (WarcraftStarted)
            {
                if (MessageBox.Show("Игра закрыта не будет. ", "Вы хотите закрыть Warcis Client?", MessageBoxButtons.YesNo) != DialogResult.Yes)
                {
                    e.Cancel = true;
                    return;
                }
            }


            UpdateConfigFile();
            LauncherLog.logfile.Close();

            try
            {
                File.Delete("Launcher.log");
                for (int i = 0; i < 25; i++)
                    File.Delete("Launcher" + i + ".log");
            }
            catch
            {

            }
            try
            {
                File.Delete("Warcis.log");
            }
            catch
            {

            }
            try
            {
                File.Delete("debug.log");
            }
            catch
            {

            }
        }

        private void AutologinPassword_TextChanged(object sender, EventArgs e)
        {

        }

        private void AutologinUsername_TextChanged(object sender, EventArgs e)
        {

        }


        private void UseFullScrenSwitcher_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void WarKeyBtn_Click(object sender, EventArgs e)
        {
            Warkey wkey = new Warkey();
            wkey.ShowDialog();
        }

        private void SelectStatsLineColor_Click(object sender, EventArgs e)
        {
            StatsLine sline = new StatsLine(LineForStats, GetWar3Proc());
            sline.ShowDialog();
            LineForStats = sline.statslineout;
        }

        private void LauncherLanguage_SelectedIndexChanged(object sender, EventArgs e)
        {
            LoadGlobalConfigFile();
        }

        private void VeryGoodInfo_Click(object sender, EventArgs e)
        {
            string messageout = "Warcis Client directory 1:\n" + Directory.GetCurrentDirectory() + "\n";
            messageout += "Warcis Client directory 2:\n" + CurrentDirectory + "\n";
            messageout += "Warcraft III directory:\n" + PathToWar3 + "\n";
            messageout += File.Exists("war3.exe") ? "Проблема: war3.exe в папке с Warcis Client" : "Нет проблем";
            MessageBox.Show(messageout, "Warcis Client GOD INFO:");
        }

        private void InWindowMode_CheckedChanged(object sender, EventArgs e)
        {
            if (InWindowMode.Checked)
                UseFullScrenSwitcher.Visible = true;
            else
                UseFullScrenSwitcher.Visible = false;
        }

        bool SkipChangeIndex = false;

        private void GrayScaleWorld_CheckedChanged(object sender, EventArgs e)
        {
            if (GrayScaleWorld.Checked)
            {
                if (GrayScaleWorldType == 0)
                    GrayScaleWorldType = 1;
            }
            else
            {
                GrayScaleWorldType = 0;
            }
            SkipChangeIndex = true;
            GlobalColorEffect.SelectedIndex = GrayScaleWorldType;
            SkipChangeIndex = false;
        }

        private void GlobalColorEffect_SelectedIndexChanged(object sender, EventArgs e)
        {

            if (SkipChangeIndex)
                return;

            if (GrayScaleWorld.Checked && GlobalColorEffect.SelectedIndex <= 0)
            {
                GrayScaleWorld.Checked = false;
            }

            if (GlobalColorEffect.SelectedIndex >= 1 && GrayScaleWorld.Checked)
            {
                GrayScaleWorldType = GlobalColorEffect.SelectedIndex;
            }
        }

        uint NickNameColor = 0xFFFFFFFF;
        uint ChatNickNameColor = 0xFFFFFFFF;
        uint ChatTextColor = 0xFFFFFFFF;
        private uint ColorToUInt(Color color)
        {
            return (uint)((color.A << 24) | (color.R << 16) |
                          (color.G << 8) | (color.B << 0));
        }
        private Color UIntToColor(uint color)
        {
            byte a = (byte)(color >> 24);
            byte r = (byte)(color >> 16);
            byte g = (byte)(color >> 8);
            byte b = (byte)(color >> 0);
            return Color.FromArgb(a, r, g, b);
        }

        string SelectColorMenuHelpStr = "Тут можно выбрать цвет никнейма в лобби игр";
        private void SelectAccountColor_Click(object sender, EventArgs e)
        {
            SelectColorMenuHelpStr = "Тут можно выбрать цвет никнейма в лобби игр";
            SelectAccountColorDialog.Color = UIntToColor(NickNameColor);
            SelectAccountColorDialog.ShowDialog();
            NickNameColor = ColorToUInt(SelectAccountColorDialog.Color);
        }



        private void SelectAccountColorDialog_HelpRequest(object sender, EventArgs e)
        {
            MessageBox.Show(SelectColorMenuHelpStr);
        }

        private void SelectChatColor_Click(object sender, EventArgs e)
        {
            SelectColorMenuHelpStr = "Тут можно выбрать цвет текста в чате";
            SelectAccountColorDialog.Color = UIntToColor(ChatTextColor);
            SelectAccountColorDialog.ShowDialog();
            ChatTextColor = ColorToUInt(SelectAccountColorDialog.Color);
        }

        private void SelectChatNameColor_Click(object sender, EventArgs e)
        {
            SelectColorMenuHelpStr = "Тут можно выбрать цвет никнейма в чате";
            SelectAccountColorDialog.Color = UIntToColor(ChatNickNameColor);
            SelectAccountColorDialog.ShowDialog();
            ChatNickNameColor = ColorToUInt(SelectAccountColorDialog.Color);
        }


        private void ShiftNumpadTrick_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void materialCheckBox1_CheckedChanged(object sender, EventArgs e)
        {

        }



        private void ChangeThemeBtn_Click(object sender, EventArgs e)
        {
            materialSkinManager.Theme = materialSkinManager.Theme == MaterialSkinManager.Themes.DARK ? MaterialSkinManager.Themes.LIGHT : MaterialSkinManager.Themes.DARK;
        }

        private int colorSchemeIndex = 0;
        private void ChangeStyleBtn_Click(object sender, EventArgs e)
        {
            colorSchemeIndex++;
            if (colorSchemeIndex > 3) colorSchemeIndex = 0;

            //These are just example color schemes
            switch (colorSchemeIndex)
            {
                case 0:
                    materialSkinManager.ColorScheme = new ColorScheme(Primary.BlueGrey800, Primary.BlueGrey900, Primary.BlueGrey500, Accent.LightBlue200, TextShade.WHITE);
                    break;
                case 1:
                    materialSkinManager.ColorScheme = new ColorScheme(Primary.Indigo500, Primary.Indigo700, Primary.Indigo100, Accent.Pink200, TextShade.WHITE);
                    break;
                case 2:
                    materialSkinManager.ColorScheme = new ColorScheme(Primary.Green600, Primary.Green700, Primary.Green200, Accent.Red100, TextShade.WHITE);
                    break;
                case 3:
                    materialSkinManager.ColorScheme = new ColorScheme(Primary.Amber200, Primary.Amber800, Primary.Amber300, Accent.Red100, TextShade.WHITE);
                    break;
            }
        }

        private void GeneralPage_Click(object sender, EventArgs e)
        {

        }

        private void UseNewD3dMode_CheckedChanged(object sender, EventArgs e)
        {
            EnableVSYNC.Visible = EnableVSYNC.Enabled = UseNewD3dMode.Checked;
        }

        int SelectedFps = 0;

        private void AvailableFpsList_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (AvailableFpsList.SelectedIndex != -1)
            {
                try
                {
                    SelectedFps = int.Parse((string)AvailableFpsList.SelectedItem);
                }
                catch
                {
                    SelectedFps = 0;
                }
            }
        }

        private void EnableVSYNC_CheckedChanged(object sender, EventArgs e)
        {

        }


        private void WarkeyEnabled_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void BugReportWithoutUser_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void UseDefaultErrorCatch_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void War3Download_Click(object sender, EventArgs e)
        {
            string LinkToWar3 = "https://drive.google.com/uc?export=download&confirm=zM-x&id=0B6XeEll7TylMQXlWRmhDQnlwNjQ";

            if (!DownloadRussianVersion.Checked)
            {
                LinkToWar3 = "https://drive.google.com/uc?export=download&confirm=Qcys&id=0B6XeEll7TylMbTRZem5kQ1BzRTQ";
            }


            if (!ChromeLauncher.OpenLink(LinkToWar3))
            {
                System.Diagnostics.Process.Start(LinkToWar3);
            }
        }

        private void WarcisPicture_Click(object sender, EventArgs e)
        {
            if (!ChromeLauncher.OpenLink("https://ru.warcis.com"))
            {
                System.Diagnostics.Process.Start("https://ru.warcis.com");
            }
        }

        private void UserCustomizeSettings_Click(object sender, EventArgs e)
        {

        }

        private void ChangeWar3Path_Click(object sender, EventArgs e)
        {
            SetWar3Path();
        }
        bool LauncherLoaded = false;
        private void AutoLoginCheckBox_CheckedChanged(object sender, EventArgs e)
        {

            if (LauncherLoaded && AutoLoginCheckBox.Checked)
            {
                MessageBox.Show("Пароль хранится в не защищенном виде.\n Не скачивайте и не запускайте подозрительные программы!");
            }
        }
    }


    public static class ProcessExtensions
    {
        public static bool IsRunning(this Process process)
        {
            if (process == null)
                throw new ArgumentNullException("process");

            try
            {
                Process.GetProcessById(process.Id);
            }
            catch (ArgumentException)
            {
                return false;
            }
            return true;
        }
    }

    internal static class ChromeLauncher
    {
        private const string ChromeAppKey = @"\Software\Microsoft\Windows\CurrentVersion\App Paths\chrome.exe";

        private static string ChromeAppFileName
        {
            get
            {
                return (string)(Registry.GetValue("HKEY_LOCAL_MACHINE" + ChromeAppKey, "", null) ??
                                    Registry.GetValue("HKEY_CURRENT_USER" + ChromeAppKey, "", null));
            }
        }

        public static bool OpenLink(string url)
        {
            string chromeAppFileName = ChromeAppFileName;
            if (string.IsNullOrEmpty(chromeAppFileName) || !File.Exists(chromeAppFileName) )
            {
                return false;
            }
            try
            {
                Process.Start(chromeAppFileName, url);
            }
            catch
            {
                return false;
            }
            return true;
        }
    }
}
