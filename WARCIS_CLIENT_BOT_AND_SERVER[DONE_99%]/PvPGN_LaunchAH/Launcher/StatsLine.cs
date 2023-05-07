using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Diagnostics;
using Syringe.Win32;
using System.Runtime.InteropServices;

namespace Launcher
{
    public partial class StatsLine : Form
    {

        Process war3process = null;

        public StatsLine(string oldstatsline, Process war3proc)
        {
            InitializeComponent();
            this.Icon = Properties.Resources.WarcisMainLogo256x256;
            StatsEnterLine.Text = oldstatsline;
            war3process = war3proc;
        }

        public string statslineout = "";

        private void availabledStats_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (availabledStats.SelectedIndex >= 0)
            {
                string insertstr = "{" + availabledStats.Items[availabledStats.SelectedIndex] + "}";
                StatsEnterLine.Text = StatsEnterLine.Text.Insert(StatsEnterLine.SelectionStart, insertstr);
                StatsEnterLine.SelectionStart = StatsEnterLine.SelectionStart + insertstr.Length;
            }
        }

        public bool OnlyHexInString(string test)
        {
            // For C-style hex notation (0xFF) you can use @"\A\b(0[xX])?[0-9a-fA-F]+\b\Z"
            return System.Text.RegularExpressions.Regex.IsMatch(test, @"^[0-9a-fA-F]+$");
        }

        private void StatsEnterLine_TextChanged(object sender, EventArgs e)
        {
            try
            {
                statslineout = StatsEnterLine.Text;
                string regexwar3color = @"(.*?)(\|c\w\w\w\w\w\w\w\w)(.*?)(\|\w.*|$)";
                string resultline = StatsEnterLine.Text.Replace("|r", "|cFFFFFFFF");
                Match regexmatch = null;
                PreviewStatsString.Clear();
                Color latestcolor = Color.White;

                while ((regexmatch = Regex.Match(resultline, regexwar3color)).Success)
                {
                    // MessageBox.Show(regexmatch.Groups[1].Value + "-> " + regexmatch.Groups[2].Value + "-> " + regexmatch.Groups[3].Value + "-> " + regexmatch.Groups[4].Value + "-> ");
                    PreviewStatsString.AppendText(regexmatch.Groups[1].Value, Color.White);
                    string colorstr = regexmatch.Groups[2].Value;
                    if (colorstr.Length == 10 && OnlyHexInString((colorstr = colorstr.Remove(0, 2))))
                    {
                        latestcolor = System.Drawing.ColorTranslator.FromHtml("#" + colorstr);
                        PreviewStatsString.AppendText(regexmatch.Groups[3].Value, latestcolor);
                    }
                    else
                    {
                        PreviewStatsString.AppendText(regexmatch.Groups[2].Value, latestcolor);
                        PreviewStatsString.AppendText(regexmatch.Groups[3].Value, latestcolor);
                    }
                    resultline = regexmatch.Groups[4].Value;

                }
                if (resultline.Length > 0)
                    PreviewStatsString.AppendText(resultline, latestcolor);
            }
            catch
            {
                PreviewStatsString.Clear();
                PreviewStatsString.AppendText("ERROR", Color.Red);
            }
        }

        struct StatsLineStr
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 1024)]
            public string StatsLine;
        }



        private void button1_Click(object sender, EventArgs e)
        {
            if (war3process != null)
            {
                try
                {
                    Syringe.Injector war3inject = new Syringe.Injector(war3process);
                    StatsLineStr tmpstatsline = new StatsLineStr();
                    tmpstatsline.StatsLine = statslineout;
                    war3inject.CallExport<StatsLineStr>("AMH.dll", "UpdateStatsLine", tmpstatsline);
                }
                catch
                {

                }
            }
            this.Close();
        }

        private void StatsLine_Load(object sender, EventArgs e)
        {

        }
    }

    public static class RichTextBoxExtensions
    {
        public static void AppendText(this RichTextBox box, string text, Color color)
        {
            box.SelectionStart = box.TextLength;
            box.SelectionLength = 0;

            box.SelectionColor = color;
            box.AppendText(text);
            box.SelectionColor = box.ForeColor;
        }
    }
}
