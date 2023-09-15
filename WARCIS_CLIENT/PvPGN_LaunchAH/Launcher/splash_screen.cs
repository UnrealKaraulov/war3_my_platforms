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
namespace Launcher
{
    public partial class splash_screen : Form
    {
        public splash_screen()
        {
            InitializeComponent();
            this.Icon = Properties.Resources.WarcisMainLogo256x256;
        }


        private void splash_screen_Load(object sender, EventArgs e)
        {
            LauncherLog.AddNewLineToLog("Start launcher...");

        
        }

        bool StopCreditsShow = false;

     
        void ChangeCreditsCoord()
        {
            //try
            //{
            //    CreditsPictuleLine.Left -= 4;
            //    CreditsPictuleLine.Refresh();
            //}
            //catch
            //{

            //}
        }

        void CreditLineUpdateThread()
        {
            Thread.CurrentThread.IsBackground = true;

            while (!StopCreditsShow)
            {
                Thread.Sleep(10);

                ChangeCreditsCoord();// Without fu*king Invoke
                //Invoke(new Action(() =>
                //{

                //   
                //}));
            }

        }

        private void StarTimer_Tick(object sender, EventArgs e)
        {
            LauncherLog.AddNewLineToLog("Start 10...");
            StarTimer.Enabled = false;
            LauncherLog.AddNewLineToLog("20...");
            new Thread(CreditLineUpdateThread).Start();
            LauncherLog.AddNewLineToLog("30...");
            var launch = new Launcher();
            LauncherLog.AddNewLineToLog("40...");
            this.Hide();
            LauncherLog.AddNewLineToLog("50...");
            launch.Closed += (s, args) => this.Close();
            LauncherLog.AddNewLineToLog("60...");
            launch.Show(null);
            LauncherLog.AddNewLineToLog("70...");
            launch.BringToFront();
            LauncherLog.AddNewLineToLog("80...");
            launch.Activate();
            LauncherLog.AddNewLineToLog("90...");
            StopCreditsShow = true;
            LauncherLog.AddNewLineToLog("Ok.");
        }

        private void WarcisPicture_Click(object sender, EventArgs e)
        {

        }
    }
}
