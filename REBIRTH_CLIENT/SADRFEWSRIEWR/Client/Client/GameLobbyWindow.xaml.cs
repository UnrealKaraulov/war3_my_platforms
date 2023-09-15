using Microsoft.Win32;
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

using Client.War3;
using System.Runtime.InteropServices;
using TgaLib;
using System.Windows.Threading;
using System.Threading;
using System.Diagnostics;

namespace Client
{
    /// <summary>
    /// Логика взаимодействия для GameLobbyWindow.xaml
    /// </summary>
    public partial class GameLobbyWindow
    {
        string MapFileName = "";
        string GameName = "";
        GlobalVariables.MapHostStruct StructMap = new GlobalVariables.MapHostStruct();


        DispatcherTimer dispatcherTimer2 = new DispatcherTimer();
        bool firstcheck = true;
        public GameLobbyWindow(string filename, GlobalVariables.MapHostStruct mapstruct, string NameGame)
        {
            try
            {
                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "Allow Local Files", 0);
            }
            catch { }

            MapFileName = filename;
            StructMap = mapstruct;
            GameName = NameGame;
            InitializeComponent();
            LoadMap();
            firstcheck = true;
            GlobalVariables.GameLobbyChatBox = ChatBox;
            if (IsHostMode)
            {
                GlobalFunctions.SendClientPacket(0x1020, 666, SlotFlags, CompFlags, 0, StructMap.MapHost, "", "", GameName);

                // GlobalFunctions.SendClientPacket(0x101511, 0, 0, 0, 0, "", "", "", "");

                for (int i = 0; i < PlayerStructList.Count; i++)
                {
                    if (PlayerStructList[i].realid >= 0 && PlayerStructList[i].realid <= 24)
                        GlobalFunctions.SendClientPacket(0x251512, (uint)PlayerStructList[i].colorid, PlayerStructList[i].realid, (uint)PlayerStructList[i].team, 0, "", "", "", "");
                }

                // GlobalFunctions.SendClientPacket(0x101511, 0, 0, 0, 0, "", "", "", "");
            }

            LobbyListView.Visibility = Visibility.Visible;

            dispatcherTimer2.Tick += DispatcherTimer2_Tick;
            dispatcherTimer2.Interval = new TimeSpan(0, 0, 2);
            dispatcherTimer2.Start();



        }

        private void DispatcherTimer2_Tick(object sender, EventArgs e)
        {
            GlobalFunctions.SendClientPacket(0x101511, 0, 0, 0, 0, "", "", "", "");
            if (firstcheck)
            {
                GlobalFunctions.SendClientPacket(0x101513, 5555555, 5555555, 5555555, 5555555, "", "", "", "");
                firstcheck = false;
            }
        }

        public static bool IsHostMode = false;

        public static BitmapSource CreateBitmapSourceFromBitmap(System.Drawing.Bitmap bitmap)
        {
            if (bitmap == null)
                throw new ArgumentNullException("bitmap");

            using (MemoryStream memoryStream = new MemoryStream())
            {
                try
                {
                    // You need to specify the image format to fill the stream. 
                    // I'm assuming it is PNG
                    bitmap.Save(memoryStream, System.Drawing.Imaging.ImageFormat.Png);
                    memoryStream.Seek(0, SeekOrigin.Begin);

                    BitmapDecoder bitmapDecoder = BitmapDecoder.Create(
                        memoryStream,
                        BitmapCreateOptions.PreservePixelFormat,
                        BitmapCacheOption.OnLoad);

                    // This will disconnect the stream from the image completely...
                    WriteableBitmap writable =
            new WriteableBitmap(bitmapDecoder.Frames.Single());
                    writable.Freeze();

                    return writable;
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }


        TriggerStringParser triggerStringParser = new TriggerStringParser();

        struct CallbackSlotItem
        {
            int id; // 1 = slot 2 = race 3 = team
        }


        Brush GetPlayerColor(uint id)
        {
            BrushConverter bc = new BrushConverter();

            string outplayercolor = "#FF0303";

            switch (id)
            {
                case 0:
                    outplayercolor = "#FF0303";
                    break;
                case 1:
                    outplayercolor = "#0042FF";
                    break;
                case 2:
                    outplayercolor = "#1CE6B9";
                    break;
                case 3:
                    outplayercolor = "#BF00FF";
                    break;
                case 4:
                    outplayercolor = "#FFFC00";
                    break;
                case 5:
                    outplayercolor = "#FE8A0E";
                    break;
                case 6:
                    outplayercolor = "#00FF00";
                    break;
                case 7:
                    outplayercolor = "#E635AA";
                    break;
                case 8:
                    outplayercolor = "#7E7E7E";
                    break;
                case 9:
                    outplayercolor = "#00EAFF";
                    break;
                case 10:
                    outplayercolor = "#6F3900";
                    break;
                case 11:
                    outplayercolor = "#055A42";
                    break;
                case 12:
                    outplayercolor = "#681212";
                    break;
                case 13:
                    outplayercolor = "#052B58";
                    break;
                case 14:
                    outplayercolor = "#0C6768";
                    break;
                case 15:
                    outplayercolor = "#57083B";
                    break;
                case 16:
                    outplayercolor = "#9C8E5C";
                    break;
                case 17:
                    outplayercolor = "#7D543F";
                    break;
                case 18:
                    outplayercolor = "#006822";
                    break;
                case 19:
                    outplayercolor = "#FF0303";
                    break;
                case 20:
                    outplayercolor = "#861C5B";
                    break;
                case 21:
                    outplayercolor = "#191919";
                    break;
                case 22:
                    outplayercolor = "#DCB9ED";
                    break;
                case 23:
                    outplayercolor = "#FFFFFF";
                    break;
                default:
                    outplayercolor = "#FF1030";
                    break;
            }

            try
            {
                return (Brush)bc.ConvertFromString(outplayercolor);
            }
            catch
            {
            }


            return Brushes.White;
        }

        List<ComboBox> slotItems = new List<ComboBox>();
        List<ComboBox> raceItems = new List<ComboBox>();



        uint SlotFlags = 0;
        uint CompFlags = 0;

        public void SetSlotName(uint colorid, string name)
        {
            //MessageBox.Show("Set slot name!");
            for (int i = 0; i < PlayerStructList.Count; i++)
            {
                var tmpPlayerStruct = PlayerStructList[i];
                if (tmpPlayerStruct.colorid == colorid)
                    tmpPlayerStruct.realname = name;
                PlayerStructList[i] = tmpPlayerStruct;
            }
        }

        public void SetSlotRace(uint colorid, uint race)
        {
            if (colorid == 4294967295 || race == 4294967295)
                return;
            //MessageBox.Show("Set slot name!");
            for (int i = 0; i < PlayerStructList.Count; i++)
            {
                var tmpPlayerStruct = PlayerStructList[i];
                if (tmpPlayerStruct.colorid == colorid)
                    tmpPlayerStruct.race = race;
                PlayerStructList[i] = tmpPlayerStruct;
            }

        }



        bool FixedRace = false;
        bool MeleeMap = false;
        public void UpdateAllSlots()
        {
            slotItems.Clear();
            LobbyListView.Items.Clear();
            SlotFlags = 0;
            CompFlags = 0;
            uint SlotIndexFlag = 1;
            uint SlotIndex = 0;
            uint SlotRealId = 0;

            foreach (var team in ForceStructList)
            {
                if (team.players == 0)
                    continue;
                // Create team 1 row
                StackPanel teamStackPanel = new StackPanel()
                {
                    Orientation = Orientation.Horizontal,
                    Width = Double.NaN,
                    Height = Double.NaN,
                    MinHeight = 24,
                    MinWidth = 120
                };

                bool needaddteam = true;

                {


                    Label teamLabel = new Label()
                    {
                        MinWidth = 300,
                        Content = "Team " + (team.name.Length > 0 ? team.name : team.id.ToString())
                    };

                    Button teamButton = new Button()
                    {
                        Background = new SolidColorBrush(Color.FromArgb(70, 100, 100, 100)),
                        Foreground = new SolidColorBrush(Colors.AntiqueWhite),
                        Content = "Join Team " + (team.id + 1),
                        DataContext = (uint)team.id
                    };

                    teamButton.Click += JointToTeamButtonClicked;

                    teamStackPanel.Children.Add(teamLabel);

                    teamStackPanel.Children.Add(teamButton);


                }

                for (int i = 0; i < PlayerStructList.Count; i++)
                {
                    var tmpPlayerStruct = PlayerStructList[i];

                    if (tmpPlayerStruct.team != team.id)
                        continue;



                    StackPanel stackPanel = new StackPanel
                    {
                        Orientation = Orientation.Horizontal,
                        Width = Double.NaN,
                        Height = Double.NaN
                    };

                    Rectangle rectangle = new Rectangle
                    {
                        Width = 13,
                        Height = 13,
                        Margin = new Thickness(2, 2, 2, 2),
                        HorizontalAlignment = HorizontalAlignment.Center,
                        VerticalAlignment = VerticalAlignment.Center,
                        Fill = GetPlayerColor(tmpPlayerStruct.colorid)
                    };


                    stackPanel.Children.Add(rectangle);

                    ComboBox SlotListBox = new ComboBox
                    {
                        FontSize = 10,
                        MinWidth = 250,
                        DataContext = 0xFFFFFFFF,
                        Foreground = Brushes.White
                    };


                    // SlotListBox.SelectedIndex = 0;
                    // if (IsHostMode)
                    //{
                    SlotListBox.IsEditable = true;
                    // }
                    SlotListBox.IsReadOnly = true;
                    SlotListBox.Text = "Open";



                    ComboBoxItem listBoxItem = new ComboBoxItem();

                    //listBoxItem = new ComboBoxItem();
                    //listBoxItem.Content = "Bot easy";
                    //SlotListBox.Items.Add(listBoxItem);
                    //listBoxItem = new ComboBoxItem();
                    //listBoxItem.Content = "Bot normal";
                    //SlotListBox.Items.Add(listBoxItem);
                    //listBoxItem = new ComboBoxItem();
                    //listBoxItem.Content = "Bot hard";
                    //SlotListBox.Items.Add(listBoxItem);

                    tmpPlayerStruct.realid = SlotIndex;

                    if (tmpPlayerStruct.type == 1)
                    {
                        if (needaddteam)
                        {
                            needaddteam = false;

                            LobbyListView.Items.Add(teamStackPanel);
                        }

                        SlotFlags = SlotFlags | SlotIndexFlag;
                        SlotIndexFlag *= 2;

                        if (IsHostMode)
                        {
                            listBoxItem.Content = "Open";
                            SlotListBox.Items.Add(listBoxItem);
                            listBoxItem = new ComboBoxItem();
                            listBoxItem.Content = "Close";
                            SlotListBox.Items.Add(listBoxItem);
                            listBoxItem = new ComboBoxItem();
                            listBoxItem.Content = "Comp";
                            SlotListBox.Items.Add(listBoxItem);
                        }
                        SlotListBox.Text = tmpPlayerStruct.realname;
                        if (tmpPlayerStruct.realname == "")
                        {
                            SlotListBox.Text = "Open";
                        }
                        else if (tmpPlayerStruct.realname == "?")
                        {
                            SlotListBox.Text = "Close";
                        }
                        else if (tmpPlayerStruct.realname == "#")
                        {
                            SlotListBox.Text = "Comp";
                        }

                        SlotListBox.DataContext = (uint)tmpPlayerStruct.colorid;
                        SlotListBox.SelectionChanged += SlotControlPacket;

                        SlotIndex++;
                        slotItems.Add(SlotListBox);
                    }
                    else if (tmpPlayerStruct.type == 2)
                    {
                        if (needaddteam)
                        {
                            needaddteam = false;

                            LobbyListView.Items.Add(teamStackPanel);
                        }

                        //   SlotFlags = SlotFlags | SlotIndex;
                        //   SlotIndex *= 2;
                        SlotIndexFlag *= 2;
                        CompFlags = CompFlags | SlotIndexFlag;
                        if (IsHostMode)
                        {
                            listBoxItem.Content = "Comp";
                            SlotListBox.Items.Add(listBoxItem);
                        }
                        SlotListBox.Text = "Comp";
                        //SlotListBox.DataContext = SlotIndex;
                        //SlotListBox.SelectionChanged += SlotControlPacket;

                        SlotIndex++;
                        SlotListBox.IsEnabled = false;
                        slotItems.Add(SlotListBox);
                    }
                    else
                    {
                        SlotIndexFlag *= 2;
                        slotItems.Add(SlotListBox);
                        continue;
                    }
                    stackPanel.Children.Add(SlotListBox);

                    SlotListBox = new ComboBox();
                    SlotListBox.FontSize = 10;
                    SlotListBox.MinWidth = 120;
                    SlotListBox.Foreground = Brushes.White;
                    SlotListBox.DataContext = (uint)(22200 + tmpPlayerStruct.colorid);
                    listBoxItem = new ComboBoxItem();
                    listBoxItem.Content = "Random Race";
                    SlotListBox.Items.Add(listBoxItem);
                    listBoxItem = new ComboBoxItem();
                    listBoxItem.Content = "Human";
                    SlotListBox.Items.Add(listBoxItem);
                    listBoxItem = new ComboBoxItem();
                    listBoxItem.Content = "Orc";
                    SlotListBox.Items.Add(listBoxItem);
                    listBoxItem = new ComboBoxItem();
                    listBoxItem.Content = "Undead";
                    SlotListBox.Items.Add(listBoxItem);
                    listBoxItem = new ComboBoxItem();
                    listBoxItem.Content = "Night Elf";
                    SlotListBox.Items.Add(listBoxItem);
                    if (!MeleeMap)
                        SlotListBox.SelectedIndex = (int)tmpPlayerStruct.race;
                    else
                        SlotListBox.SelectedIndex = 0;

                    if (SlotListBox.SelectedIndex < 0)
                    {
                        SlotListBox.SelectedIndex = (int)tmpPlayerStruct.defaultrace;
                    }

                    if (FixedRace)
                        SlotListBox.IsEnabled = false;
                    SlotListBox.SelectionChanged += RaceControlPacket;
                    SlotListBox.IsEditable = false;

                    stackPanel.Children.Add(SlotListBox);

                    //SlotListBox = new ComboBox();
                    //SlotListBox.FontSize = 10;
                    //SlotListBox.MinWidth = 120;
                    //SlotListBox.DataContext = 2;
                    //SlotListBox.SelectionChanged += SlotControlPacket;
                    //foreach (var curTeam in ForceStructList)
                    //{
                    //    listBoxItem = new ComboBoxItem();
                    //    listBoxItem.Content = curTeam.name.Length > 0 ? curTeam.name : ("Team " + curTeam.id);
                    //    SlotListBox.Items.Add(listBoxItem);
                    //}

                    //stackPanel.Children.Add(SlotListBox);

                    LobbyListView.Items.Add(stackPanel);

                    PlayerStructList[i] = tmpPlayerStruct;
                    SlotRealId++;
                }
            }


            //  MessageBox.Show(SlotFlags.ToString("x2"));
        }

        private void JointToTeamButtonClicked(object sender, RoutedEventArgs e)
        {
            Button button = sender as Button;
            GlobalFunctions.SendClientPacket(0x101512, (uint)button.DataContext, 0, 0, 0, "", "", "", "");
            // MessageBox.Show("YES! Start slot id:" + StartSlotId + ", End slot id:" + EndSlotId);
        }

        private void SlotControlPacket(object sender, SelectionChangedEventArgs e)
        {
            ComboBox button = sender as ComboBox;

            if (button.IsDropDownOpen)
            {
                //OPTIONAL:
                //Causes the combobox selection changed to not be fired again if anything
                //in the function below changes the selection (as in my weird case)
                button.IsDropDownOpen = false;
                /// MessageBox.Show(button.DataContext.ToString());
                //now put the code you want to fire when a user selects an option here
                if (button.SelectedIndex != -1)
                {
                    if (IsHostMode)
                        GlobalFunctions.SendClientPacket(0x101513, (uint)button.DataContext, (uint)button.SelectedIndex, 0, 0, "", "", "", "");
                    else if (!IsHostMode)
                    {
                        MessageBox.Show("НЕВОЗМОЖНО!");
                    }
                }
            }


        }

        private void RaceControlPacket(object sender, SelectionChangedEventArgs e)
        {
            ComboBox button = sender as ComboBox;

            if (button.IsDropDownOpen)
            {
                //OPTIONAL:
                //Causes the combobox selection changed to not be fired again if anything
                //in the function below changes the selection (as in my weird case)
                button.IsDropDownOpen = false;
                /// MessageBox.Show(button.DataContext.ToString());
                //now put the code you want to fire when a user selects an option here
                if (button.SelectedIndex != -1)
                {
                    GlobalFunctions.SendClientPacket(0x101513, (uint)button.DataContext, (uint)button.SelectedIndex, 0, 0, "", "", "", "");
                }
            }


        }
        //private void TestButtonClick(object sender, RoutedEventArgs e)
        //{
        //    Button button = sender as Button;
        //    // MessageBox.Show("Button " + (int)button.DataContext + " is pressed");


        //}



        struct PlayerStruct
        {
            public uint colorid;
            public uint realid;
            public int type;
            public uint race;
            public uint defaultrace;
            public int fixedpos;
            public string defaultname;
            public string realname;
            public float startx;
            public float starty;
            public uint allyflaglow;
            public uint allyflaghigh;
            public int team;
        }

        struct ForceStruct
        {
            public int id;
            public uint flags;
            public uint playermask;
            public string name;
            public int players;
        }
        List<PlayerStruct> PlayerStructList = new List<PlayerStruct>();
        List<ForceStruct> ForceStructList = new List<ForceStruct>();


        private void LoadMap()
        {
            GlobalFunctions._LogMessage("Button_Click:1");

            PlayerStructList.Clear();

            IntPtr hMpq = IntPtr.Zero;
            bool bOpenResult = StormLib.SFileOpenArchive(MapFileName, 0, StormLib.MPQ_OPEN_READ_ONLY, out hMpq);
            if (hMpq != IntPtr.Zero && bOpenResult)
            {

                try
                {
                    StormLib.SFileExtractFile(hMpq, "War3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }
                if (!File.Exists("code.j"))
                {
                    try
                    {
                        StormLib.SFileExtractFile(hMpq, "war3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                    }
                    catch
                    {

                    }
                }

                if (!File.Exists("code.j"))
                {

                    try
                    {
                        StormLib.SFileExtractFile(hMpq, "scripts\\war3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                    }
                    catch
                    {

                    }
                    if (!File.Exists("code.j"))
                    {
                        try
                        {
                            StormLib.SFileExtractFile(hMpq, "scripts\\War3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                        }
                        catch
                        {

                        }
                    }
                    if (!File.Exists("code.j"))
                    {
                        try
                        {
                            StormLib.SFileExtractFile(hMpq, "Scripts\\war3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                        }
                        catch
                        {

                        }
                    }
                    if (!File.Exists("code.j"))
                    {
                        try
                        {
                            StormLib.SFileExtractFile(hMpq, "Scripts\\War3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                        }
                        catch
                        {

                        }
                    }
                }


                if (!File.Exists("code.j"))
                {
                    MessageBox.Show("Error. This map without war3map.j");
                }

                try
                {
                    StormLib.SFileExtractFile(hMpq, "War3map.w3i", "map.info", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }

                if (!File.Exists("map.info"))
                {
                    try
                    {
                        StormLib.SFileExtractFile(hMpq, "war3map.w3i", "map.info", StormLib.SFILE_OPEN_FROM_MPQ);
                    }
                    catch
                    {

                    }
                }


                if (!File.Exists("map.info"))
                {
                    File.Delete("code.j");
                    MessageBox.Show("Error. This map without war3map.w3i");
                }


                try
                {
                    StormLib.SFileExtractFile(hMpq, "War3map.wts", "trigstr.wts", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }

                if (!File.Exists("trigstr.wts"))
                {
                    try
                    {
                        StormLib.SFileExtractFile(hMpq, "war3map.wts", "trigstr.wts", StormLib.SFILE_OPEN_FROM_MPQ);
                    }
                    catch
                    {

                    }
                }



                try
                {
                    StormLib.SFileExtractFile(hMpq, "war3mapPreview.blp", "map.preview.blp", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }

                try
                {
                    StormLib.SFileExtractFile(hMpq, "war3mapPreview.tga", "map.preview.tga", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }

                try
                {
                    StormLib.SFileExtractFile(hMpq, "war3mapMap.blp", "map.minimap.blp", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }

                try
                {
                    StormLib.SFileExtractFile(hMpq, "war3mapMap.tga", "map.minimap.tga", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

                }


                StormLib.SFileCloseArchive(hMpq);

                try
                {

                    GlobalFunctions._LogMessage("Button_Click:2");

                    triggerStringParser.ParseFile("trigstr.wts");


                    GlobalFunctions._LogMessage("Button_Click:3");

                    using (BinaryReader binaryReader = new BinaryReader(File.Open("map.info", FileMode.Open)))
                    {
                        GlobalFunctions._LogMessage("Button_Click:4");
                        string outstr = "";

                        int version = binaryReader.ReadInt32();// version 18 - roc , 25+ tft

                        outstr += " Version:" + version + "\n";


                        uint mapbuild = binaryReader.ReadUInt32();
                        outstr += " mapbuild:" + mapbuild + "\n";

                        binaryReader.ReadInt32();// editor version

                        string MapName = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());
                        string MapAuthor = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());
                        string MapDescription = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());
                        string RecommendedPlayers = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());

                        outstr += " MapName:" + triggerStringParser.GetTrigStrText(MapName) + "\n";

                        outstr += " MapDescription:" + MapDescription + "\n";

                        if (MapDescription.Length > 0)
                        {
                            MapDescriptionText.Text = MapDescription;
                        }


                        outstr += " MapAuthor:" + MapAuthor + "\n";
                        outstr += " RecommendedPlayers:" + RecommendedPlayers + "\n";

                        GlobalFunctions._LogMessage("Button_Click:5");

                        /* Camera Bounds */
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();
                        binaryReader.ReadSingle();

                        /* camera bounds complements */
                        binaryReader.ReadInt32();
                        binaryReader.ReadInt32();
                        binaryReader.ReadInt32();
                        binaryReader.ReadInt32();

                        /* map playable area */
                        binaryReader.ReadInt32();
                        binaryReader.ReadInt32();

                        /* flags */
                        uint flags = binaryReader.ReadUInt32();

                        if ((flags & (1 << 5)) > 0)
                        {
                            FixedRace = true;
                        }

                        if ((flags & (1 << 2)) > 0)
                        {
                            MeleeMap = true;
                        }



                        /* map main ground type */
                        binaryReader.ReadChar();

                        /*  Campaign background number */
                        binaryReader.ReadInt32();

                        string LoadScreenModel = "";

                        if (version != 18)
                            LoadScreenModel = binaryReader.ReadNullTerminatedStringWc3(); ;
                        GlobalFunctions._LogMessage("Button_Click:6");

                        string LoadingText = binaryReader.ReadNullTerminatedStringWc3();
                        string LoadingTitle = binaryReader.ReadNullTerminatedStringWc3();
                        string LoadingSubTitle = binaryReader.ReadNullTerminatedStringWc3();

                        outstr += " LoadingText:" + LoadingText + "\n";
                        outstr += " LoadingTitle:" + LoadingTitle + "\n";
                        outstr += " LoadingSubTitle:" + LoadingSubTitle + "\n";



                        /* Map loading screen number */
                        binaryReader.ReadInt32();

                        string PrologueScreenPath = "";

                        if (version != 18)
                            PrologueScreenPath = binaryReader.ReadNullTerminatedStringWc3(); ;


                        string PrologueText = binaryReader.ReadNullTerminatedStringWc3();
                        string PrologueTitle = binaryReader.ReadNullTerminatedStringWc3();
                        string PrologueSubTitle = binaryReader.ReadNullTerminatedStringWc3();

                        outstr += " PrologueText:" + PrologueText + "\n";
                        outstr += " PrologueTitle:" + PrologueTitle + "\n";
                        outstr += " PrologueSubTitle:" + PrologueSubTitle + "\n";



                        int terrainfog = 0;

                        if (version != 18)
                        {
                            terrainfog = binaryReader.ReadInt32();
                            binaryReader.ReadSingle();
                            binaryReader.ReadSingle();
                            binaryReader.ReadSingle();
                            binaryReader.ReadInt32();
                        }

                        int weatherid = 0;
                        if (version != 18)
                            weatherid = binaryReader.ReadInt32();

                        string customsound = "";
                        if (version != 18)
                            customsound = binaryReader.ReadNullTerminatedStringWc3();
                        GlobalFunctions._LogMessage("Button_Click:7");

                        outstr += " customsound:" + customsound + "\n";


                        if (version != 18)
                        {
                            // light

                            binaryReader.ReadByte();
                            binaryReader.ReadInt32();
                        }


                        int maxplayers = binaryReader.ReadInt32();

                        for (int i = 0; i < maxplayers; i++)
                        {
                            PlayerStruct playerStruct = new PlayerStruct();
                            playerStruct.colorid = binaryReader.ReadUInt32(); // color
                            playerStruct.realid = 0xFFFFFFFF;
                            playerStruct.type = binaryReader.ReadInt32(); // 1 = open,  2 = comp , = closed
                            playerStruct.race = binaryReader.ReadUInt32(); // 1-4 race other = random
                            playerStruct.defaultrace = playerStruct.race; // 1-4 race other = random
                            playerStruct.fixedpos = binaryReader.ReadInt32();
                            playerStruct.defaultname = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());
                            playerStruct.startx = binaryReader.ReadSingle();
                            playerStruct.starty = binaryReader.ReadSingle();


                            playerStruct.allyflaglow = binaryReader.ReadUInt32();
                            playerStruct.allyflaghigh = binaryReader.ReadUInt32();
                            PlayerStructList.Add(playerStruct);

                        }
                        outstr += " ----------------------------------------\n";
                        /* Map loading screen number */
                        int maxforces = binaryReader.ReadInt32();
                        for (int i = 0; i < maxforces; i++)
                        {
                            ForceStruct forceStruct = new ForceStruct();
                            forceStruct.flags = binaryReader.ReadUInt32();
                            forceStruct.players = 0;
                            forceStruct.playermask = binaryReader.ReadUInt32();
                            forceStruct.name = triggerStringParser.GetTrigStrText(binaryReader.ReadNullTerminatedStringWc3());
                            forceStruct.id = i;
                            ForceStructList.Add(forceStruct);

                            outstr += "Team  " + (i + 1) + "\n";
                            outstr += "flags:" + forceStruct.flags + "\n";
                            outstr += "playermask:" + forceStruct.playermask + "\n";
                            outstr += "name:" + forceStruct.name + "\n";


                            uint playermask = forceStruct.playermask;
                            for (byte j = 0; j < 12; j++)
                            {
                                if ((playermask & 1) > 0)
                                {
                                    for (int n = 0; n < PlayerStructList.Count(); n++)
                                    {
                                        var tmpplayerstr = PlayerStructList[n];

                                        if (tmpplayerstr.colorid == j)
                                            tmpplayerstr.team = i;

                                        PlayerStructList[n] = tmpplayerstr;
                                    }
                                }

                                playermask >>= 1;
                            }


                        }
                        outstr += "----------------------------------------\n";


                        outstr += " ----------------------------------------\n";
                        for (int i = 0; i < maxplayers; i++)
                        {
                            var playerStruct = PlayerStructList[i];
                            try
                            {
                                var forceStruct = ForceStructList[playerStruct.team];
                                forceStruct.players++;
                                ForceStructList[playerStruct.team] = forceStruct;
                            }
                            catch
                            {

                            }
                            outstr += "Player " + (playerStruct.colorid + 1) + "\n";
                            outstr += "Type:" + playerStruct.type + "\n";
                            outstr += "allyflaglow:" + playerStruct.allyflaglow + "\n";
                            outstr += "allyflaghigh:" + playerStruct.allyflaghigh + "\n";
                            outstr += "Team:" + playerStruct.team + "\n";
                        }


                        this.Title = "War3 Game Room. Players 0/" + maxplayers + ". Map : " + triggerStringParser.GetTrigStrText(MapName);


                        //  MessageBox.Show(outstr);
                    }

                    GlobalFunctions._LogMessage("Button_Click:8");

                    try
                    {
                        if (File.Exists("map.preview.tga"))
                        {
                            using (var fs = new System.IO.FileStream("map.preview.tga", FileMode.Open, FileAccess.Read, FileShare.Read))
                            using (var reader = new System.IO.BinaryReader(fs))
                            {
                                var tga = new TgaImage(reader);
                                PreviewImage.Source = tga.GetBitmap();
                                fs.Close();
                            }
                        }
                        else if (File.Exists("map.preview.blp"))
                        {
                            IntPtr OutDataArray = IntPtr.Zero;
                            uint width = 0;
                            uint height = 0;
                            byte[] filedata = File.ReadAllBytes("map.preview.blp");

                            GlobalFunctions.readblp(filedata, filedata.Length, ref OutDataArray, ref width, ref height);

                            byte[] RGBAbytes = new byte[width * height * 4];
                            Marshal.Copy(OutDataArray, RGBAbytes, 0, (int)(width * height * 4));
                            RgbaBitmapSource rgbaBitmapSource = new RgbaBitmapSource(RGBAbytes, 256);
                            PreviewImage.Source = rgbaBitmapSource;
                        }
                    }
                    catch
                    {

                    }
                    try
                    {
                        if (File.Exists("map.minimap.tga"))
                        {
                            using (var fs = new System.IO.FileStream("map.minimap.tga", FileMode.Open, FileAccess.Read, FileShare.Read))
                            using (var reader = new System.IO.BinaryReader(fs))
                            {
                                var tga = new TgaImage(reader);
                                MinimapImage.Source = tga.GetBitmap();
                                fs.Close();
                            }
                        }
                        else if (File.Exists("map.minimap.blp"))
                        {
                            IntPtr OutDataArray = IntPtr.Zero;
                            uint width = 0;
                            uint height = 0;
                            byte[] filedata = File.ReadAllBytes("map.minimap.blp");

                            GlobalFunctions.readblp(filedata, filedata.Length, ref OutDataArray, ref width, ref height);

                            byte[] RGBAbytes = new byte[width * height * 4];
                            Marshal.Copy(OutDataArray, RGBAbytes, 0, (int)(width * height * 4));
                            RgbaBitmapSource rgbaBitmapSource = new RgbaBitmapSource(RGBAbytes, 256);
                            MinimapImage.Source = rgbaBitmapSource;
                        }
                    }
                    catch
                    {

                    }


                    GlobalFunctions._LogMessage("Button_Click:11");

                    try
                    {
                        File.Delete("code.j");
                        File.Delete("map.info");
                        File.Delete("trigstr.wts");

                        File.Delete("map.preview.tga");
                        File.Delete("map.preview.blp");
                        File.Delete("map.minimap.tga");
                        File.Delete("map.minimap.blp");
                    }
                    catch
                    {

                    }

                    UpdateAllSlots();
                }
                catch
                {
                    MessageBox.Show("Fatal error");
                }
            }

            //if (IsHostMode)
            //{
            //    LobbyListView.Visibility = Visibility.Hidden;

            //    // MessageBox.Show("YES GAME CREATED!");
            //}
        }


        public void Exit()
        {
            Application.Current.Dispatcher.Invoke(new Action(() => this.Close()));
        }


        private void Button_Click(object sender, RoutedEventArgs e)
        {
            StartBtn.IsEnabled = false;
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

            string War3Path = GlobalVariables.GlobalConfiguration["General"]["PathToWc3"].GetString(true, false);

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

            try
            {
                File.Copy(Directory.GetCurrentDirectory() + "\\WAR.bin", War3Path + "WAR.bin");
            }
            catch
            {

            }

            try
            {
                File.Copy(Directory.GetCurrentDirectory() + "\\War3.dll", War3Path + "War3.dll");
            }
            catch
            {

            }
            if (IsHostMode)
            {
                GlobalFunctions.SendClientPacket(0x101510, 0, 0, 0, 0, "", "", "", "");
            }
            else
            {
                MessageBox.Show("НЕВОЗМОЖНО!");
            }
        }

        private void MetroWindow_Closed(object sender, EventArgs e)
        {
            try
            {
                Registry.SetValue(@"HKEY_CURRENT_USER\Software\Blizzard Entertainment\Warcraft III", "Allow Local Files", 0);
            }
            catch { }

            GlobalVariables.GameLobbyOpened = false;
            GlobalVariables.gameLobbyWindow = null;
            dispatcherTimer2.Stop();
            if (IsHostMode)
            {
                GlobalFunctions.SendClientPacket(0x101509, 0, 0, 0, 0, "", "", "", "");
            }
            else
            {
                GlobalFunctions.SendClientPacket(0x101508, 0, 0, 0, 0, "", "", "", "");
            }
            IsHostMode = false;
            ClientWindow.CreateGameButtonStatic.Visibility = Visibility.Visible;
            ClientWindow.GameListButtonStatic.Visibility = Visibility.Visible;
            GlobalVariables.GameLobbyChatBox = null;
        }
        void SendMessageToServer(string Message)
        {
            Message = new string(Message.Where(c => !char.IsControl(c)).ToArray());
            GlobalFunctions.DefaultToUtf8(ref Message);
            GlobalFunctions.SendClientPacket(0x32140556, 0, 0, 0, 0, Message, "", "", "");
        }
        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            SendMessageToServer(ChatInputTextBox.Text);
            ChatInputTextBox.Clear();
        }

        private void MetroWindow_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Return || e.Key == Key.Enter)
            {
                SendMessageToServer(ChatInputTextBox.Text);
                ChatInputTextBox.Clear();
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Application curApp = Application.Current;
            Window mainWindow = curApp.MainWindow;
            this.Left = mainWindow.Left + (mainWindow.Width - this.ActualWidth) / 2;
            this.Top = mainWindow.Top + (mainWindow.Height - this.ActualHeight) / 2;
        }

    }


    static class BinaryReaderex
    {
        public static byte[] GetDecodedBytes(this System.IO.BinaryReader stream)
        {
            List<byte> decoded = new List<byte>();
            int pos = 0;
            byte mask = 0;

            byte b = stream.ReadByte();
            while (b != 0)
            {
                if (pos % 8 == 0)
                {
                    mask = b;
                }
                else
                {
                    if ((mask & (0x1 << (pos % 8))) == 0)
                        decoded.Add((byte)(b - 1));
                    else
                        decoded.Add(b);
                }

                b = stream.ReadByte();
                pos++;
            }
            decoded.Add(0);
            return decoded.ToArray();
        }



        public static byte[] GetStringNullterminated(this System.IO.BinaryReader stream)
        {
            List<byte> decoded = new List<byte>();
            byte b = 0;
            while ((b = stream.ReadByte()) != 0)
            {
                decoded.Add(b);
            }
            return decoded.ToArray();
        }

        public static string ReadNullTerminatedStringWc3Encoded(this System.IO.BinaryReader stream)
        {
            return Encoding.UTF8.GetString(GetDecodedBytes(stream));
        }

        public static string ReadNullTerminatedStringWc3(this System.IO.BinaryReader stream)
        {
            return Encoding.UTF8.GetString(GetStringNullterminated(stream));
        }
    }


}
