using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.Threading;

namespace Launcher
{
    static class Program
    {
        public static bool NoUpdateArg = false;

       

        /// <summary>
        /// Главная точка входа для приложения.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            foreach (string s in args)
            {
                if (s.ToLower().IndexOf("-noupdate") == 0)
                {
                    NoUpdateArg = true;

                }
            }
       
            Process CurrentProcess = Process.GetCurrentProcess();


            Process[] wmiservices = Process.GetProcessesByName("WmiPrvSE");
            foreach (Process p in wmiservices)
            {
                try
                {
                    p.Kill();
                }
                catch
                {

                }
            }


            foreach (Process p in Process.GetProcesses())
            {
                try
                {
                    if (p.MainModule.FileName.ToLower().IndexOf((Path.GetDirectoryName(CurrentProcess.MainModule.FileName) + "/").ToLower()) > -1 && CurrentProcess.Id != p.Id)
                    {
                        // MessageBox.Show(p.MainModule.FileName, CurrentProcess.MainModule.FileName);
                        p.Kill();
                    }
                }
                catch
                {

                }

                try
                {
                    if (p.MainModule.FileName.ToLower().IndexOf((Path.GetDirectoryName(CurrentProcess.MainModule.FileName) + "\\").ToLower()) > -1 && CurrentProcess.Id != p.Id)
                    {
                        // MessageBox.Show(p.MainModule.FileName, CurrentProcess.MainModule.FileName);
                        p.Kill();
                    }
                }
                catch
                {

                }
            }



            Mutex mutex = new System.Threading.Mutex(false, "WarcisClientLauncher");
            try
            {
                if (mutex.WaitOne(0, false))
                {

                }
                else
                {
                    Application.Exit();
                }
            }
            finally
            {
                if (mutex != null)
                {
                    mutex.Close();
                    mutex = null;
                }
            }

            int fileid = 0;
            createnewlog:
            try
            {
                if (fileid == 0)
                    LauncherLog.logfile = new FileStream(@"Launcher.log", FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.Read);
                else
                    LauncherLog.logfile = new FileStream(@"Launcher" + fileid + ".log", FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.Read);
            }
            catch
            {
                fileid++;
                if (fileid < 25)
                {
                    goto createnewlog;
                }
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new splash_screen());


        }
    }

    static class LauncherLog
    {
        public static FileStream logfile = null;
        public static int errorfound = 3;
        public static void AddNewLineToLog(string line)
        {
            try
            {

                byte[] writebytes = Encoding.UTF8.GetBytes(line + "\r\n");
                logfile.Write(writebytes, 0, writebytes.Length);
                logfile.Flush();
            }
            catch
            {

            }
        }

    }

}
