using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.IO;
using System.Diagnostics;
using System.Threading;

namespace ClientUpdater
{
    static class Program
    {
        /// <summary>
        /// Главная точка входа для приложения.
        /// </summary>
        /// 
        public static string UpdateServer = "https://client.warcis.top/clientupdate/";
        public static string UpdateDirPath = "UpdateDir";
        public static bool NeedGenerateListfile = false;
        [STAThread]
        static void Main(string[] args)
        {
            Process CurrentProcess = Process.GetCurrentProcess();

            //
            if (File.Exists("updatefilelist.txt"))
            {
                try
                {
                    File.Delete("updatefilelist.txt");
                    File.Create("updatefilelist.txt").Close();
                    Console.WriteLine("Create updatefilelist.txt file for save new md5 hashes!");
                }
                catch
                {

                }
            }
            if (args.Length >= 1 && args[0] != "-installhere-")
            {
                UpdateServer = args[0];
                //  MessageBox.Show("Update server(new):" + UpdateServer);
            }
            else
            {
                // MessageBox.Show("Update server(old):" + UpdateServer);
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


            Mutex mutex = new System.Threading.Mutex(false, "WarcisClientUpdater");
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



            //MessageBox.Show(UpdateServer);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            if (args.Length == 0 && !Directory.Exists(UpdateDirPath) && (!File.Exists("AMH.dll") || (File.Exists("war3.exe") && File.Exists("Game.dll"))))
            {
                if (Directory.GetDirectories(Directory.GetCurrentDirectory()).Length != 0
                    || Directory.GetFiles(Directory.GetCurrentDirectory()).Length != 1 || (File.Exists("war3.exe") && File.Exists("Game.dll")))
                {

                    if (!File.Exists("war3.exe") && !File.Exists("Game.dll"))
                    {
                        DialogResult sdres = MessageBox.Show("ДА для выбора, ОТМЕНА для отмены, НЕТ для установки в текущую папку."
                            , "Выбрать путь для установки!", MessageBoxButtons.YesNoCancel);

                        if (sdres == DialogResult.No)
                        {
                            Application.Run(new WarcisUpdateWindow());
                            return;
                        }

                        if (sdres == DialogResult.Cancel)
                        {
                            Application.Exit();
                            return;
                        }
                    }
                    else
                    {
                        MessageBox.Show("Внимание, произошла неисправимая ошибка!\nВы додумались установить Warcis клиент в\nпапку с игрой! Выберите новый путь!", "Неисправимая ошибка!");
                    }
                    //MessageBox.Show(Directory.GetDirectories(Directory.GetCurrentDirectory()).Length.ToString(), "Directories:");
                    //MessageBox.Show(Directory.GetFiles(Directory.GetCurrentDirectory()).Length.ToString(), "Files:");

                    //trynew:
                    FolderBrowserDialog fd = new FolderBrowserDialog();
                    fd.Description = "Select folder for Warcis Launcher";
                    fd.ShowNewFolderButton = true;
                    DialogResult fres = DialogResult.Abort;
                    try
                    {
                        fres = fd.ShowDialog();
                    }
                    catch
                    {
                        MessageBox.Show("Не достаточно прав для запуска!\nЗапустите с админ правами!", "Ошибка.");
                    }

                    if (fres == DialogResult.OK
                        || fres == DialogResult.Yes)
                    {
                        string self = System.Reflection.Assembly.GetEntryAssembly().Location;
                        string selffilename = Path.GetFileName(self);
                        string newfilepath = fd.SelectedPath + @"\" + selffilename;

                        if (File.Exists(fd.SelectedPath + "\\war3.exe")
                            && File.Exists(fd.SelectedPath + "\\game.dll")
                            )
                        {
                            MessageBox.Show("Внимание, произошла неисправимая ошибка!\nВы додумались установить Warcis клиент в\nпапку с игрой! Выберите новый путь!", "Неисправимая ошибка!");
                            Application.Exit();
                            return;
                        }

                        try
                        {
                            if (File.Exists(newfilepath))
                                File.Delete(newfilepath);
                            File.Copy(self, newfilepath);
                        }
                        catch
                        {

                        }

                        try
                        {

                            ProcessStartInfo si = new ProcessStartInfo(newfilepath);
                            si.Arguments = "-installhere-";
                            si.WorkingDirectory = fd.SelectedPath;
                            Process.Start(si);
                        }
                        catch
                        {
                            MessageBox.Show(newfilepath, "Не достаточно прав для запуска:");
                        }
                    }
                    Application.Exit();
                    return;
                }


            }

            Application.Run(new WarcisUpdateWindow());
        }
    }
}
