using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using IniParser;
using IniParser.Model;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Launcher
{
    public partial class Warkey : Form
    {
        public Warkey()
        {
            InitializeComponent();


            this.Icon = Properties.Resources.WarcisMainLogo256x256;

            try
            {
                LoadConfigFile();
                UpdateConfigFile();
            }
            catch
            {

            }
        }
        string ConfigFileName = "LauncherConfig.ini";

        string QKeyTextText = string.Empty;
        string WKeyTextText = string.Empty;
        string EKeyTextText = string.Empty;
        string RKeyTextText = string.Empty;
        string AKeyTextText = string.Empty;
        string SKeyTextText = string.Empty;
        string DKeyTextText = string.Empty;
        string FKeyTextText = string.Empty;
        string ZKeyTextText = string.Empty;
        string XKeyTextText = string.Empty;
        string CKeyTextText = string.Empty;
        string VKeyTextText = string.Empty;

        string Num1KeyTextText = string.Empty;
        string Num2KeyTextText = string.Empty;
        string Num4KeyTextText = string.Empty;
        string Num5KeyTextText = string.Empty;
        string Num7KeyTextText = string.Empty;
        string Num8KeyTextText = string.Empty;

        private void SelectedProfileName_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                LoadConfigFile();
            }
            catch
            {

            }
        }

        public void LoadConfigFile()
        {
            ListOfChatEvents.Items.Clear();
            ListOfChatEvents.Update(); // In case there is databinding
            ListOfChatEvents.Refresh(); // Redraw items

            QKeyText.Text =
            WKeyText.Text =
            EKeyText.Text =
            RKeyText.Text =
            AKeyText.Text =
            SKeyText.Text =
            DKeyText.Text =
            FKeyText.Text =
            ZKeyText.Text =
            XKeyText.Text =
            CKeyText.Text =
            VKeyText.Text =
            Num7KeyText.Text =
            Num8KeyText.Text =
            Num4KeyText.Text =
            Num5KeyText.Text =
            Num1KeyText.Text =
            Num2KeyText.Text =
            TargetChatKeyCode.Text =
            TargetChatMessage.Text =
          QKeyTextText =
            WKeyTextText =
            EKeyTextText =
            RKeyTextText =
            AKeyTextText =
            SKeyTextText =
            DKeyTextText =
            FKeyTextText =
            ZKeyTextText =
            XKeyTextText =
            CKeyTextText =
            VKeyTextText =
            Num7KeyTextText =
            Num8KeyTextText =
            Num4KeyTextText =
            Num5KeyTextText =
            Num1KeyTextText =
            Num2KeyTextText =
            "";


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

            }
            try
            {
                if (!data.Sections.ContainsSection("WARKEY"))
                {
                    data.Sections.Add(new SectionData("WARKEY"));
                }


                if (SelectedProfileName.Text.Length == 0)
                {
                    SelectedProfileName.Text = data["WARKEY"]["SelectedProfile"];
                    if (SelectedProfileName.Text.Length == 0)
                    {
                        SelectedProfileName.Text = "DEFAULT";
                    }
                }


                int.TryParse(data["WARKEY"]["Profiles"], out int profilescount);

                for (int i = 0; i < profilescount; i++)
                {
                    if (data["WARKEY"]["Profile_" + (i + 1)].Length > 0 &&
                        !SelectedProfileName.Items.Contains(data["WARKEY"]["Profile_" + (i + 1)])
                        )
                        SelectedProfileName.Items.Add(data["WARKEY"]["Profile_" + (i + 1)]);
                }

                if (profilescount == 0)
                {
                    if (!SelectedProfileName.Items.Contains("DEFAULT"))
                        SelectedProfileName.Items.Add("DEFAULT");
                }



                if (!data.Sections.ContainsSection("WARKEY_" + SelectedProfileName.Text))
                {
                    data.Sections.Add(new SectionData("WARKEY_" + SelectedProfileName.Text));
                }

                QKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Q"];
                WKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["W"];
                EKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["E"];
                RKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["R"];
                AKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["A"];
                SKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["S"];
                DKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["D"];
                FKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["F"];
                ZKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Z"];
                XKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["X"];
                CKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["C"];
                VKeyTextText = data["WARKEY_" + SelectedProfileName.Text]["V"];

                Num1KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num1"];
                Num2KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num2"];
                Num4KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num4"];
                Num5KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num5"];
                Num7KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num7"];
                Num8KeyTextText = data["WARKEY_" + SelectedProfileName.Text]["Num8"];




                QKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(QKeyTextText.Length > 0 ?
                                   QKeyTextText : "0")
                                    );
                WKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(WKeyTextText.Length > 0 ?
                                   WKeyTextText : "0")
                                    );
                EKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(EKeyTextText.Length > 0 ?
                                   EKeyTextText : "0")
                                    );
                RKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(RKeyTextText.Length > 0 ?
                                   RKeyTextText : "0")
                                    );
                AKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(AKeyTextText.Length > 0 ?
                                   AKeyTextText : "0")
                                    );
                SKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(SKeyTextText.Length > 0 ?
                                  SKeyTextText : "0")
                                    );
                DKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(DKeyTextText.Length > 0 ?
                                   DKeyTextText : "0")
                                    );
                FKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(FKeyTextText.Length > 0 ?
                                   FKeyTextText : "0")
                                    );
                ZKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(ZKeyTextText.Length > 0 ?
                                   ZKeyTextText : "0")
                                    );
                XKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(XKeyTextText.Length > 0 ?
                                   XKeyTextText : "0")
                                    );
                CKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(CKeyTextText.Length > 0 ?
                                   CKeyTextText : "0")
                                    );
                VKeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(VKeyTextText.Length > 0 ?
                                   VKeyTextText : "0")
                                    );
                Num1KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num1KeyTextText.Length > 0 ?
                                     Num1KeyTextText : "0")
                                    );


                Num2KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num2KeyTextText.Length > 0 ?
                                     Num2KeyTextText : "0")
                                    );
                Num4KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num4KeyTextText.Length > 0 ?
                                     Num4KeyTextText : "0")
                                    );
                Num5KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num5KeyTextText.Length > 0 ?
                                     Num5KeyTextText : "0")
                                    );
                Num7KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num7KeyTextText.Length > 0 ?
                                     Num7KeyTextText : "0")
                                    );
                Num8KeyText.Text = KeyHelper.MyKeyToStr(
                                    uint.Parse(Num8KeyTextText.Length > 0 ?
                                     Num8KeyTextText : "0")
                                    );


                int.TryParse(data["CHATHELPER_" + SelectedProfileName.Text]["NumRegisteredKeyCount"], out int KeyChatActionsCount);


                for (int i = 0; i < KeyChatActionsCount; i++)
                {
                    uint.TryParse(data["CHATHELPER_" + SelectedProfileName.Text]["ChatActionKeyCode_" + i], out uint TmpRegisteredKeyCode);


                    if (TmpRegisteredKeyCode == 0)
                        continue;


                    string TmpRegisteredKeyMessage = data["CHATHELPER_" + SelectedProfileName.Text]["ChatActionKeyText_" + i];

                    ListOfChatEvents.Items.Add(new ListViewItem(new string[]
                    {
                        KeyHelper.MyKeyToStr( TmpRegisteredKeyCode   ) ,
                        TmpRegisteredKeyMessage,
                        TmpRegisteredKeyCode.ToString(),
                    }));

                }


            }
            catch
            {
                UpdateConfigFile();
            }

        }

        public void UpdateConfigFile()
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
                if (!data.Sections.ContainsSection("WARKEY"))
                    data.Sections.Add(new SectionData("WARKEY"));
                if (!data.Sections.ContainsSection("WARKEY_" + SelectedProfileName.Text))
                    data.Sections.Add(new SectionData("WARKEY_" + SelectedProfileName.Text));

            }
            catch
            {

            }

            if (!SelectedProfileName.Items.Contains(SelectedProfileName.Text))
            {
                SelectedProfileName.Items.Add(SelectedProfileName.Text);
            }

            try
            {
                if (!data.Sections.ContainsSection("CHATHELPER_" + SelectedProfileName.Text))
                    data.Sections.Add(new SectionData("CHATHELPER_" + SelectedProfileName.Text));

            }
            catch
            {

            }

            try
            {
                data["WARKEY"]["SelectedProfile"] = SelectedProfileName.Text;
                for (int i = 0; i < SelectedProfileName.Items.Count; i++)
                {
                    data["WARKEY"]["Profile_" + (i + 1)] = SelectedProfileName.Items[i].ToString();
                }
                data["WARKEY"]["Profiles"] = SelectedProfileName.Items.Count.ToString();
            }
            catch
            {

            }
            try
            {
                data["WARKEY_" + SelectedProfileName.Text]["Q"] = QKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["W"] = WKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["E"] = EKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["R"] = RKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["A"] = AKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["S"] = SKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["D"] = DKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["F"] = FKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Z"] = ZKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["X"] = XKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["C"] = CKeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["V"] = VKeyTextText;

                data["WARKEY_" + SelectedProfileName.Text]["Num1"] = Num1KeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Num2"] = Num2KeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Num4"] = Num4KeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Num5"] = Num5KeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Num7"] = Num7KeyTextText;
                data["WARKEY_" + SelectedProfileName.Text]["Num8"] = Num8KeyTextText;


                data["CHATHELPER_" + SelectedProfileName.Text]["NumRegisteredKeyCount"] = ListOfChatEvents.Items.Count.ToString();


                for (int i = 0; i < ListOfChatEvents.Items.Count; i++)
                {
                    data["CHATHELPER_" + SelectedProfileName.Text]["ChatActionKeyCode_" + i] = ListOfChatEvents.Items[i].SubItems[2].Text;
                    data["CHATHELPER_" + SelectedProfileName.Text]["ChatActionKeyText_" + i] = ListOfChatEvents.Items[i].SubItems[1].Text;
                }



                // Сохранение в файл

                parser.WriteFile(ConfigFileName, data, Encoding.UTF8);

            }
            catch
            {

            }
        }

        private void Warkey_Load(object sender, EventArgs e)
        {

        }



        private void ClearBtn_Click(object sender, EventArgs e)
        {
            ListOfChatEvents.Items.Clear();
            ListOfChatEvents.Update(); // In case there is databinding
            ListOfChatEvents.Refresh(); // Redraw items

            QKeyText.Text =
            WKeyText.Text =
            EKeyText.Text =
            RKeyText.Text =
            AKeyText.Text =
            SKeyText.Text =
            DKeyText.Text =
            FKeyText.Text =
            ZKeyText.Text =
            XKeyText.Text =
            CKeyText.Text =
            VKeyText.Text =
            Num7KeyText.Text =
            Num8KeyText.Text =
            Num4KeyText.Text =
            Num5KeyText.Text =
            Num1KeyText.Text =
            Num2KeyText.Text =
            TargetChatKeyCode.Text =
            TargetChatMessage.Text =
          QKeyTextText =
            WKeyTextText =
            EKeyTextText =
            RKeyTextText =
            AKeyTextText =
            SKeyTextText =
            DKeyTextText =
            FKeyTextText =
            ZKeyTextText =
            XKeyTextText =
            CKeyTextText =
            VKeyTextText =
            Num7KeyTextText =
            Num8KeyTextText =
            Num4KeyTextText =
            Num5KeyTextText =
            Num1KeyTextText =
            Num2KeyTextText =
            "";

            UpdateConfigFile();
        }

        private void ExitBtn_Click(object sender, EventArgs e)
        {
            try
            {
                UpdateConfigFile();
            }
            catch
            {

            }
            this.Close();
        }


        private void LoadBtn_Click(object sender, EventArgs e)
        {
            try
            {
                LoadConfigFile();
            }
            catch
            {

            }
        }


        private void SaveBtn_Click(object sender, EventArgs e)
        {
            try
            {
                UpdateConfigFile();
            }
            catch
            {

            }
        }



        private void VKeyText_KeyDown(object sender, KeyEventArgs e)
        {
            TextBox _sender = sender as TextBox;
            _sender.Text = KeyHelper.KeyAllKeys(2);

            if (_sender == QKeyText)
                QKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == WKeyText)
                WKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == EKeyText)
                EKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == RKeyText)
                RKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == AKeyText)
                AKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == SKeyText)
                SKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == DKeyText)
                DKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == FKeyText)
                FKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == ZKeyText)
                ZKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == XKeyText)
                XKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == VKeyText)
                VKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == CKeyText)
                CKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();

            if (_sender == Num1KeyText)
                Num1KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num2KeyText)
                Num2KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num4KeyText)
                Num4KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num5KeyText)
                Num5KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num7KeyText)
                Num7KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num8KeyText)
                Num8KeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();


        }

        private void VKeyText_MouseDown(object sender, MouseEventArgs e)
        {
            TextBox _sender = sender as TextBox;
            _sender.Text = KeyHelper.KeyAllKeys(2);
            bool skipkey = false;
            if (KeyHelper.latestkeycount == 1 && e.Button == MouseButtons.Left)
            {
                _sender.Text = "";
                skipkey = true;
            }

            if (_sender == QKeyText)
                QKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == WKeyText)
                WKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == EKeyText)
                EKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == RKeyText)
                RKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == AKeyText)
                AKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == SKeyText)
                SKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == DKeyText)
                DKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == FKeyText)
                FKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == ZKeyText)
                ZKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == XKeyText)
                XKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == VKeyText)
                VKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == CKeyText)
                CKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();

            if (_sender == Num1KeyText)
                Num1KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num2KeyText)
                Num2KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num4KeyText)
                Num4KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num5KeyText)
                Num5KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num7KeyText)
                Num7KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
            if (_sender == Num8KeyText)
                Num8KeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();

        }

        private void InsBtn_Click(object sender, EventArgs e)
        {
            if (ChatKeyTextText.Length == 0 || ChatKeyTextText == "0")
            {
                MessageBox.Show("Error. Please select key for this message!");
                return;
            }

            if (TargetChatMessage.Text.Length > 126 || TargetChatMessage.Text.Length < 2)
            {
                MessageBox.Show("Error. Bad message len!");
                return;
            }

            foreach (ListViewItem item in ListOfChatEvents.Items)
            {
                if (item.SubItems[2].Text == ChatKeyTextText)
                {
                    MessageBox.Show("Error. This key already used in another QuickChat message!");
                    return;
                }

                if (item.SubItems[1].Text == TargetChatMessage.Text)
                {
                    try
                    {
                        MessageBox.Show("Error. This message already used with '" + KeyHelper.MyKeyToStr(uint.Parse(item.SubItems[2].Text)) + "' key!");
                    }
                    catch
                    {

                    }
                    return;
                }
            }

            ListOfChatEvents.Items.Add(new ListViewItem(new string[]
                   {
                         KeyHelper.MyKeyToStr( uint.Parse(ChatKeyTextText)   ) ,
                        TargetChatMessage.Text,
                   ChatKeyTextText,
                   }));
        }

        private void DelBtn_Click(object sender, EventArgs e)
        {
            try
            {
                ListOfChatEvents.Items.RemoveAt(ListOfChatEvents.Items.IndexOf(ListOfChatEvents.SelectedItems[0]));
            }
            catch
            {

            }
        }

        string ChatKeyTextText = "0";

        private void ChatKeyText_KeyDown(object sender, KeyEventArgs e)
        {
            TextBox _sender = sender as TextBox;
            _sender.Text = KeyHelper.KeyAllKeys(2);


            ChatKeyTextText = KeyHelper.KeyAllKeys_code(2).ToString();
        }

        private void ChatKeyText_MouseDown(object sender, MouseEventArgs e)
        {
            TextBox _sender = sender as TextBox;
            _sender.Text = KeyHelper.KeyAllKeys(2);
            bool skipkey = false;
            if (KeyHelper.latestkeycount == 1 && e.Button == MouseButtons.Left)
            {
                _sender.Text = "";
                skipkey = true;
            }

            ChatKeyTextText = skipkey ? "0" : KeyHelper.KeyAllKeys_code(2).ToString();
        }

    }

    public class KeyHelper
    {

        [DllImport("User32.dll")]
        private static extern short GetAsyncKeyState(int vKey);

        public static int latestkeycount = 0;
        public static string KeyAllKeys(int max)
        {
            latestkeycount = 0;
            string keyBuffer = string.Empty;
            bool pressed = false;
            for (int i = 0; i < 255; i++)
            {
                if (i == (int)Keys.ShiftKey)
                    continue;
                if (i == (int)Keys.Menu)
                    continue;
                if (i == (int)Keys.ControlKey)
                    continue;



                short x = GetAsyncKeyState(i);
                if ((x & 0x8000) > 0)
                {
                    if (i == (int)Keys.LShiftKey)
                    {
                        keyBuffer = "SHIFT+" + keyBuffer;
                    }
                    else if (i == (int)Keys.LMenu)
                    {
                        keyBuffer = "ALT+" + keyBuffer;
                    }
                    else if (i == (int)Keys.LControlKey)
                    {
                        keyBuffer = "CTRL+" + keyBuffer;
                    }
                    else
                    {
                        if (!pressed)
                        {
                            pressed = true;
                            keyBuffer = Enum.GetName(typeof(Keys), i) + "+" + keyBuffer;
                        }
                    }
                    latestkeycount++;
                    max--;
                    if (max == 0)
                        break;
                }

            }

            if (keyBuffer.Length > 1)
            {
                keyBuffer = keyBuffer.Remove(keyBuffer.Length - 1);
            }
            return keyBuffer;
        }


        public static string MyKeyToStr(uint val)
        {
            if (val == 0)
            {
                return string.Empty;
            }
            string keyBuffer = string.Empty;

            if ((val & 0x40000) > 0)
            {
                keyBuffer = "SHIFT+";
            }
            if ((val & 0x10000) > 0)
            {
                keyBuffer = "ALT+";
            }
            if ((val & 0x20000) > 0)
            {
                keyBuffer = "CTRL+";
            }

            int KeyVal = (int)(val & 0xFF);

            keyBuffer += Enum.GetName(typeof(Keys), KeyVal);

            return keyBuffer;
        }

        public static uint KeyAllKeys_code(int max)
        {
            uint code = 0;
            latestkeycount = 0;
            for (int i = 0; i < 255; i++)
            {
                if (i == (int)Keys.ShiftKey)
                    continue;
                if (i == (int)Keys.Menu)
                    continue;
                if (i == (int)Keys.ControlKey)
                    continue;



                short x = GetAsyncKeyState(i);
                if ((x & 0x8000) > 0)
                {
                    if (i == (int)Keys.LShiftKey)
                    {
                        code += 0x40000;
                    }
                    else if (i == (int)Keys.LMenu)
                    {
                        code += 0x10000;
                    }
                    else if (i == (int)Keys.LControlKey)
                    {
                        code += 0x20000;
                    }
                    else
                    {
                        if ((code & 0xFF) == 0)
                            code += (uint)i;
                    }
                    latestkeycount++;
                    max--;
                    if (max == 0)
                        break;
                }

            }

            return code;
        }

    }

}
