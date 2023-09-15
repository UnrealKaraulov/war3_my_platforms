using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using static Client.GlobalVariables;
using System.IO;
using System.Diagnostics;
using System.Security.Cryptography;

using FluentFTP;

using System.Net;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Shapes;
using System.Diagnostics.Contracts;

using static Client.GlobalFunctions;
using DemoApp;

namespace Client
{
    public static class GlobalVariables
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct RGBA
        {
            public byte r;
            public byte g;
            public byte b;
            public byte a;
        };

        public class MapHostStruct
        {
            public string MapName { get; set; }
            public string MapNameForList { get; set; }
            public string MapHost { get; set; }
            public string MapCategory { get; set; }
            public string MapFileName { get; set; }
            // string [] MapModes;
            // string [] MapPlayers;
            public uint crc32 { get; set; }
            public bool ForStats { get; set; }
            public bool availabled { get; set; }
        };
        public static IniFile GlobalConfiguration = null;

        public static List<MapHostStruct> MapHostList = new List<MapHostStruct>();

        public static bool War3ProxyStarted = false;
        public static bool GameLobbyOpened = false;
        public static string Password = "";
        public static string Username = "";
        public static List<string> InternalLog = new List<string>();
        public static Thread Wc3ProxyThread = null;
        public static bool HostMenuOpened = false;
        public static bool GameListOpened = false;
        public static GameLobbyWindow gameLobbyWindow = null;
        public static GameListWindow gameListWindow = null;
        public static bool OneInstance = false;
        public static bool War3WorkerStarted = false;
        public static bool Firewallpatched = false;

        public static LoginWindow LoginWindow = null;

        public static RichTextBoxExt GameLobbyChatBox = null;

        public static ClientWindow clientWindow = null;
        internal static Process War3Process;
    }

    public static class GlobalFunctions
    {
        public static void War3Worker()
        {
            if (War3WorkerStarted)
                return;
            War3WorkerStarted = true;

            int maxwaittime = 70;
            while (maxwaittime-- > 0)
            {
                Thread.Sleep(100);


            }


            War3WorkerStarted = false;
        }

        public static void CheckForPatches()
        {
            bool foundpatch = true;
            while (foundpatch)
            {
                foundpatch = false;
                try
                {
                    string War3Path = GlobalVariables.GlobalConfiguration["General"]["PathToWc3"].GetString(true, false);

                    try
                    {
                        if (!Directory.Exists(War3Path + "Patches"))
                            Directory.CreateDirectory(War3Path + "Patches");
                    }
                    catch
                    {

                    }

                    try
                    {
                        if (!Directory.Exists(War3Path + "Trash"))
                            Directory.CreateDirectory(War3Path + "Trash");
                    }
                    catch
                    {

                    }

                    foreach (string file in Directory.GetFiles(War3Path, "*.asi*"))
                    {
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.m3d*"))
                    {
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.mix*"))
                    {
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.flt*"))
                    {
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }

                    War3Path += "redist\\";


                    foreach (string file in Directory.GetFiles(War3Path, "*.asi*",SearchOption.AllDirectories))
                    {
                        if (file.ToLower().IndexOf("mp3dec") > -1)
                            continue;
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "..\\Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.m3d*", SearchOption.AllDirectories))
                    {
                        if (file.ToLower().IndexOf("mssfast") > -1)
                            continue;
                        if (file.ToLower().IndexOf("msseax2") > -1)
                            continue; 
                        if (file.ToLower().IndexOf("mssdolby") > -1)
                            continue;
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "..\\Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.mix*", SearchOption.AllDirectories))
                    {
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "..\\Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }
                    foreach (string file in Directory.GetFiles(War3Path, "*.flt*", SearchOption.AllDirectories))
                    {
                        if (file.ToLower().IndexOf("reverb3") > -1)
                            continue;
                        try
                        {
                            if (MessageBox.Show("Использовать файл:" + file + "\nКак MPQ патч?", "Внимание обнаружен патч", MessageBoxButton.YesNo) == MessageBoxResult.Yes)
                            {
                                foundpatch = true;
                                File.Move(file, War3Path + "..\\Patches\\" + System.IO.Path.GetFileName(file));
                            }
                            else
                            {
                                foundpatch = true;
                                MessageBox.Show("Спасибо за выбор, теперь этот файл будет удален!");
                                File.Delete(War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                                File.Move(file, War3Path + "..\\Trash\\" + System.IO.Path.GetFileName(file));
                            }
                        }
                        catch
                        {

                        }
                    }


                }
                catch
                {

                }
            }
        }

        public static string GetRegionName()
        {
            string regname = GlobalConfiguration["General"]["Region"].GetString();
            if (regname.Length == 0)
                regname = "1";
            return "Region" + regname;
        }

        public static void SetRegion(int id)
        {
            GlobalConfiguration["General"]["Region"] = id;
        }

        public static int GetRegions()
        {
            return GlobalConfiguration["RegionSettings"]["Regions"].ToInt();
        }

        public static string GetCurrentLanguage()
        {
            return GlobalConfiguration[GetRegionName()]["LanguageInternal"].GetString();
        }

        public static string GetCurrentLanguageText()
        {
            return GlobalConfiguration[GetRegionName()]["Language"].GetString();
        }

        public static string GetStringByRegion(string key)
        {
            return GlobalConfiguration[GetRegionName()][key].GetString();
        }


        public static string GetLocalizedString(string key)
        {
            return GlobalConfiguration[GetCurrentLanguage()][key].GetString();
        }

        public static void StartWarcraftAndJoinGame(string War3Path, string GameName)
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

            try
            {
                File.Delete(War3Path + "d3d8.dll");
            }
            catch
            {
                MessageBox.Show("Не могу удалить испорченный d3d8.dll из попки с игрой!");
                Environment.Exit(0);
            }

            try
            {
                File.WriteAllBytes(War3Path + @"redist\miles\Mp3dec.asi", Properties.Resources.Mp3dec);
                File.WriteAllBytes(War3Path + @"redist\miles\Mssdolby.m3d", Properties.Resources.Mssdolby);
                File.WriteAllBytes(War3Path + @"redist\miles\Msseax2.m3d", Properties.Resources.Msseax2);
                File.WriteAllBytes(War3Path + @"redist\miles\Mssfast.m3d", Properties.Resources.Mssfast);
                File.WriteAllBytes(War3Path + @"redist\miles\Reverb3.flt", Properties.Resources.Reverb3);
            }
            catch
            {

            }

            CheckForPatches();

            if (Process.GetProcessesByName("WAR.bin").Length == 0 && Process.GetProcessesByName("war3").Length == 0)
            {
                var processStartInfo = new ProcessStartInfo
                {
                    FileName = War3Path + "WAR.bin",
                    Arguments = (GlobalConfiguration["General"]["WindowModeCheck"].ToBool() ? " -window " : "")
                    + (GlobalConfiguration["General"]["VideoDriver"].ToInt() == 2 ? " -opengl " : ""),
                    UseShellExecute = false
                };

                GlobalVariables.War3Process = Process.Start(processStartInfo);
            }
            else
            {
                MessageBox.Show("Sorry man, need close all old game copies! pies pies pies!");
            }
        }

        public static Process GetWarcraftProcess( )
        {
            if (Process.GetProcessesByName("WAR.bin").Length > 0)
                if (Process.GetProcessesByName("WAR.bin")[0] == GlobalVariables.War3Process)
                    return GlobalVariables.War3Process;
            return null;
        }

        public static void Utf8ToDefault(ref string s)
        {
            byte[] bytes = Encoding.Default.GetBytes(s);
            s = Encoding.UTF8.GetString(bytes);
        }
        public static void DefaultToUtf8(ref string s)
        {
            byte[] bytes = Encoding.UTF8.GetBytes(s);
            s = Encoding.Default.GetString(bytes);
        }
        private static bool IngameMessageBoxAccess = true;
        public static void ShowMessagBox(string message, string title = "Info")
        {
            if (IngameMessageBoxAccess)
            {
                var mb = new MessageBoxWindow(message, title);
                //mb.Owner = Application.Current.MainWindow;
                mb.ShowDialog();
                IngameMessageBoxAccess = false;
            }
        }

        public static string GetMapFullPath(MapHostStruct tmpMapHostStruct)
        {
            string fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(true, false) + @"maps\download\" + tmpMapHostStruct.MapFileName;
            if (!File.Exists(fullpath))
            {
                return String.Empty;
            }

            return fullpath;
        }


        public static bool CheckMapAvaiabled(MapHostStruct tmpMapHostStruct)
        {
            string fullpath = GlobalConfiguration["General"]["PathToWc3"].GetString(false, false) + @"maps\download\" + tmpMapHostStruct.MapFileName;
            if (!File.Exists(fullpath))
            {
                return false;
            }

            File.AppendAllText("OutCommands.txt", "Check map:" + tmpMapHostStruct.MapFileName + ". Crc:" + tmpMapHostStruct.crc32.ToString("X2") + "\n");
            try
            {
                File.Delete("code.j");
            }
            catch
            {

            }

            IntPtr hMpq = IntPtr.Zero;
            bool bOpenResult = StormLib.SFileOpenArchive(fullpath, 0, StormLib.MPQ_OPEN_READ_ONLY, out hMpq);
            if (hMpq != IntPtr.Zero && bOpenResult)
            {

                try
                {
                    StormLib.SFileExtractFile(hMpq, "war3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                }
                catch
                {

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
                        StormLib.SFileExtractFile(hMpq, "scripts\\war3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
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
                        StormLib.SFileExtractFile(hMpq, "War3map.j", "code.j", StormLib.SFILE_OPEN_FROM_MPQ);
                    }
                    catch
                    {

                    }
                }


                StormLib.SFileCloseArchive(hMpq);

                if (!File.Exists("code.j"))
                {
                }
                else
                {
                    var checksum = GetFileChecksum("code.j");


                    if (checksum != tmpMapHostStruct.crc32 )
                    {
                        MessageBox.Show("Crc1:" + checksum.ToString("X2") + ", Crc2:" + tmpMapHostStruct.crc32.ToString("X2"), "Checksum Mismatch Error");
                        try
                        {
                            File.Delete("code.j");
                        }
                        catch
                        {

                        }

                        return false;
                    }
                    else
                    {
                        try
                        {
                            File.Delete("code.j");
                        }
                        catch
                        {

                        }

                        return true;
                    }
                }

                try
                {
                    File.Delete("code.j");
                }
                catch { }
            }
            if (hMpq != IntPtr.Zero)
                StormLib.SFileCloseArchive(hMpq);

            return false;
        }


        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void SendClientPacket(uint command, uint var1,
            uint var2, uint var3, uint var4, string strvar, string strvar2, string strvar3, string strvar4);

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void TerminateProxy();

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern void LogMessage(string str);

        public static void _LogMessage(string str)
        {
            try
            {
                LogMessage(str);
            }
            catch
            {

            }
        }


        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern uint GetFileChecksum(string str);

        [DllImport("War3proxy.dll", CallingConvention = CallingConvention.StdCall)]
        public static extern bool readblp(byte[] input, int inputlen, ref IntPtr _output, ref uint Width, ref uint Height);

        public static RGBA[] getRGBAfromBytes(byte[] bytes)
        {
            int rgbasize = bytes.Length;

            RGBA[] retRGBA = new RGBA[rgbasize];

            for (int i = 0; i < rgbasize; i += 4)
            {
                retRGBA[i / 4].r = bytes[i];
                retRGBA[i / 4].g = bytes[i + 1];
                retRGBA[i / 4].b = bytes[i + 2];
                retRGBA[i / 4].a = bytes[i + 3];
            }



            return new RGBA[0];
        }

    }




    public class RgbaBitmapSource : BitmapSource
    {
        private byte[] rgbaBuffer;
        private int pixelWidth;
        private int pixelHeight;

        public RgbaBitmapSource(byte[] rgbaBuffer, int pixelWidth)
        {
            this.rgbaBuffer = rgbaBuffer;
            this.pixelWidth = pixelWidth;
            this.pixelHeight = rgbaBuffer.Length / (4 * pixelWidth);
        }

        public override void CopyPixels(
            Int32Rect sourceRect, Array pixels, int stride, int offset)
        {
            for (int y = sourceRect.Y; y < sourceRect.Y + sourceRect.Height; y++)
            {
                for (int x = sourceRect.X; x < sourceRect.X + sourceRect.Width; x++)
                {
                    int i = stride * y + 4 * x;
                    byte a = rgbaBuffer[i + 3];
                    byte b = (byte)(rgbaBuffer[i] * a / 256); // pre-multiplied R
                    byte g = (byte)(rgbaBuffer[i + 1] * a / 256); // pre-multiplied G
                    byte r = (byte)(rgbaBuffer[i + 2] * a / 256); // pre-multiplied B

                    pixels.SetValue(b, i + offset);
                    pixels.SetValue(g, i + offset + 1);
                    pixels.SetValue(r, i + offset + 2);
                    pixels.SetValue(a, i + offset + 3);
                }
            }
        }

        protected override Freezable CreateInstanceCore()
        {
            return new RgbaBitmapSource(rgbaBuffer, pixelWidth);
        }

        public override event EventHandler<DownloadProgressEventArgs> DownloadProgress;
        public override event EventHandler DownloadCompleted;
        public override event EventHandler<ExceptionEventArgs> DownloadFailed;
        public override event EventHandler<ExceptionEventArgs> DecodeFailed;

        public override double DpiX
        {
            get { return 96; }
        }

        public override double DpiY
        {
            get { return 96; }
        }

        public override PixelFormat Format
        {
            get { return PixelFormats.Pbgra32; }
        }

        public override int PixelWidth
        {
            get { return pixelWidth; }
        }

        public override int PixelHeight
        {
            get { return pixelHeight; }
        }

        public override double Width
        {
            get { return pixelWidth; }
        }

        public override double Height
        {
            get { return pixelHeight; }
        }
    }


    public static class MyWpfExtensions
    {
        public static System.Windows.Forms.IWin32Window GetIWin32Window(this System.Windows.Media.Visual visual)
        {
            var source = System.Windows.PresentationSource.FromVisual(visual) as System.Windows.Interop.HwndSource;
            System.Windows.Forms.IWin32Window win = new OldWindow(source.Handle);
            return win;
        }

        private class OldWindow : System.Windows.Forms.IWin32Window
        {
            private readonly System.IntPtr _handle;
            public OldWindow(System.IntPtr handle)
            {
                _handle = handle;
            }

            #region IWin32Window Members
            System.IntPtr System.Windows.Forms.IWin32Window.Handle
            {
                get { return _handle; }
            }
            #endregion
        }
    }


    public struct IniValue
    {
        private static bool TryParseInt(string text, out int value)
        {
            int res;
            if (Int32.TryParse(text,
                System.Globalization.NumberStyles.Integer,
                System.Globalization.CultureInfo.InvariantCulture,
                out res))
            {
                value = res;
                return true;
            }
            value = 0;
            return false;
        }

        private static bool TryParseDouble(string text, out double value)
        {
            double res;
            if (Double.TryParse(text,
                System.Globalization.NumberStyles.Float,
                System.Globalization.CultureInfo.InvariantCulture,
                out res))
            {
                value = res;
                return true;
            }
            value = Double.NaN;
            return false;
        }

        public string Value;

        public IniValue(object value)
        {
            var formattable = value as IFormattable;
            if (formattable != null)
            {
                Value = formattable.ToString(null, System.Globalization.CultureInfo.InvariantCulture);
            }
            else
            {
                Value = value != null ? value.ToString() : null;
            }
        }

        public IniValue(string value)
        {
            Value = value;
        }

        public bool ToBool(bool valueIfInvalid = false)
        {
            bool res;
            if (TryConvertBool(out res))
            {
                return res;
            }
            return valueIfInvalid;
        }

        public bool TryConvertBool(out bool result)
        {
            if (Value == null)
            {
                result = default(bool);
                return false;
            }
            var boolStr = Value.Trim().ToLowerInvariant();
            if (boolStr == "true")
            {
                result = true;
                return true;
            }
            else if (boolStr == "false")
            {
                result = false;
                return true;
            }
            result = default(bool);
            return false;
        }

        public int ToInt(int valueIfInvalid = 0)
        {
            int res;
            if (TryConvertInt(out res))
            {
                return res;
            }
            return valueIfInvalid;
        }

        public bool TryConvertInt(out int result)
        {
            if (Value == null)
            {
                result = default(int);
                return false;
            }
            if (TryParseInt(Value.Trim(), out result))
            {
                return true;
            }
            return false;
        }

        public double ToDouble(double valueIfInvalid = 0)
        {
            double res;
            if (TryConvertDouble(out res))
            {
                return res;
            }
            return valueIfInvalid;
        }

        public bool TryConvertDouble(out double result)
        {
            if (Value == null)
            {
                result = default(double);
                return false; ;
            }
            if (TryParseDouble(Value.Trim(), out result))
            {
                return true;
            }
            return false;
        }

        public string GetString()
        {
            return GetString(true, false);
        }

        public string GetString(bool preserveWhitespace)
        {
            return GetString(true, preserveWhitespace);
        }

        public string GetString(bool allowOuterQuotes, bool preserveWhitespace)
        {
            if (Value == null)
            {
                return "";
            }
            var trimmed = Value.Trim();
            if (allowOuterQuotes && trimmed.Length >= 2 && trimmed[0] == '"' && trimmed[trimmed.Length - 1] == '"')
            {
                var inner = trimmed.Substring(1, trimmed.Length - 2);
                return preserveWhitespace ? inner : inner.Trim();
            }
            else
            {
                return preserveWhitespace ? Value : Value.Trim();
            }
        }

        public override string ToString()
        {
            return Value;
        }

        public static implicit operator IniValue(byte o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(short o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(int o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(sbyte o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(ushort o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(uint o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(float o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(double o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(bool o)
        {
            return new IniValue(o);
        }

        public static implicit operator IniValue(string o)
        {
            return new IniValue(o);
        }

        private static readonly IniValue _default = new IniValue();
        public static IniValue Default { get { return _default; } }
    }

    public class IniFile : IEnumerable<KeyValuePair<string, IniSection>>, IDictionary<string, IniSection>
    {
        private Dictionary<string, IniSection> sections;
        public IEqualityComparer<string> StringComparer;

        public bool SaveEmptySections;

        public IniFile()
            : this(DefaultComparer)
        {

        }

        public IniFile(string path, bool ordered = false)
        {
            StringComparer = DefaultComparer;
            sections = new Dictionary<string, IniSection>();
            SaveEmptySections = false;

            try
            {
                this.Load(path, ordered);
            }
            catch
            {
                StringComparer = DefaultComparer;
                sections = new Dictionary<string, IniSection>();
                SaveEmptySections = false;

            }
        }


        public IniFile(IEqualityComparer<string> stringComparer)
        {
            StringComparer = stringComparer;
            sections = new Dictionary<string, IniSection>(StringComparer);
        }

        public void Save(string path, FileMode mode = FileMode.Create)
        {
            using (var stream = new FileStream(path, mode, FileAccess.Write))
            {
                Save(stream);
            }
        }

        public void Save(Stream stream)
        {
            using (var writer = new StreamWriter(stream))
            {
                Save(writer);
            }
        }

        public void Save(StreamWriter writer)
        {
            foreach (var section in sections)
            {
                if (section.Value.Count > 0 || SaveEmptySections)
                {
                    writer.WriteLine(string.Format("[{0}]", section.Key.Trim()));
                    foreach (var kvp in section.Value)
                    {
                        writer.WriteLine(string.Format("{0}={1}", kvp.Key, kvp.Value));
                    }
                    writer.WriteLine("");
                }
            }
        }

        public void Load(string path, bool ordered = false)
        {
            using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                Load(stream, ordered);
            }
        }

        public void Load(Stream stream, bool ordered = false)
        {
            using (var reader = new StreamReader(stream))
            {
                Load(reader, ordered);
            }
        }

        public void Load(StreamReader reader, bool ordered = false)
        {
            IniSection section = null;

            while (!reader.EndOfStream)
            {
                var line = reader.ReadLine();

                if (line != null)
                {
                    var trimStart = line.TrimStart();

                    if (trimStart.Length > 0)
                    {
                        if (trimStart[0] == '[')
                        {
                            var sectionEnd = trimStart.IndexOf(']');
                            if (sectionEnd > 0)
                            {
                                var sectionName = trimStart.Substring(1, sectionEnd - 1).Trim();
                                section = new IniSection(StringComparer) { Ordered = ordered };
                                sections[sectionName] = section;
                            }
                        }
                        else if (section != null && trimStart[0] != ';')
                        {
                            string key;
                            IniValue val;

                            if (LoadValue(line, out key, out val))
                            {
                                section[key] = val;
                            }
                        }
                    }
                }
            }
        }

        private bool LoadValue(string line, out string key, out IniValue val)
        {
            var assignIndex = line.IndexOf('=');
            if (assignIndex <= 0)
            {
                key = null;
                val = null;
                return false;
            }

            key = line.Substring(0, assignIndex).Trim();
            var value = line.Substring(assignIndex + 1);

            val = new IniValue(value);
            return true;
        }

        public bool ContainsSection(string section)
        {
            return sections.ContainsKey(section);
        }

        public bool TryGetSection(string section, out IniSection result)
        {
            return sections.TryGetValue(section, out result);
        }

        bool IDictionary<string, IniSection>.TryGetValue(string key, out IniSection value)
        {
            return TryGetSection(key, out value);
        }

        public bool Remove(string section)
        {
            return sections.Remove(section);
        }

        public IniSection Add(string section, Dictionary<string, IniValue> values, bool ordered = false)
        {
            return Add(section, new IniSection(values, StringComparer) { Ordered = ordered });
        }

        public IniSection Add(string section, IniSection value)
        {
            if (value.Comparer != StringComparer)
            {
                value = new IniSection(value, StringComparer);
            }
            sections.Add(section, value);
            return value;
        }

        public IniSection Add(string section, bool ordered = false)
        {
            var value = new IniSection(StringComparer) { Ordered = ordered };
            sections.Add(section, value);
            return value;
        }

        void IDictionary<string, IniSection>.Add(string key, IniSection value)
        {
            Add(key, value);
        }

        bool IDictionary<string, IniSection>.ContainsKey(string key)
        {
            return ContainsSection(key);
        }

        public ICollection<string> Keys
        {
            get { return sections.Keys; }
        }

        public ICollection<IniSection> Values
        {
            get { return sections.Values; }
        }

        void ICollection<KeyValuePair<string, IniSection>>.Add(KeyValuePair<string, IniSection> item)
        {
            ((IDictionary<string, IniSection>)sections).Add(item);
        }

        public void Clear()
        {
            sections.Clear();
        }

        bool ICollection<KeyValuePair<string, IniSection>>.Contains(KeyValuePair<string, IniSection> item)
        {
            return ((IDictionary<string, IniSection>)sections).Contains(item);
        }

        void ICollection<KeyValuePair<string, IniSection>>.CopyTo(KeyValuePair<string, IniSection>[] array, int arrayIndex)
        {
            ((IDictionary<string, IniSection>)sections).CopyTo(array, arrayIndex);
        }

        public int Count
        {
            get { return sections.Count; }
        }

        bool ICollection<KeyValuePair<string, IniSection>>.IsReadOnly
        {
            get { return ((IDictionary<string, IniSection>)sections).IsReadOnly; }
        }

        bool ICollection<KeyValuePair<string, IniSection>>.Remove(KeyValuePair<string, IniSection> item)
        {
            return ((IDictionary<string, IniSection>)sections).Remove(item);
        }

        public IEnumerator<KeyValuePair<string, IniSection>> GetEnumerator()
        {
            return sections.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public IniSection this[string section]
        {
            get
            {
                IniSection s = null;
                if (sections.TryGetValue(section, out s))
                {
                    return s;
                }
                s = new IniSection(StringComparer);
                sections[section] = s;
                return s;
            }
            set
            {
                var v = value;
                if (v.Comparer != StringComparer)
                {
                    v = new IniSection(v, StringComparer);
                }
                sections[section] = v;
            }
        }

        public string GetContents()
        {
            using (var stream = new MemoryStream())
            {
                Save(stream);
                stream.Flush();
                var builder = new StringBuilder(Encoding.UTF8.GetString(stream.ToArray()));
                return builder.ToString();
            }
        }

        public static IEqualityComparer<string> DefaultComparer = new CaseInsensitiveStringComparer();

        class CaseInsensitiveStringComparer : IEqualityComparer<string>
        {
            public bool Equals(string x, string y)
            {
                return String.Compare(x, y, true) == 0;
            }

            public int GetHashCode(string obj)
            {
                return obj.ToLowerInvariant().GetHashCode();
            }

#if JS
        public new bool Equals(object x, object y) {
            var xs = x as string;
            var ys = y as string;
            if (xs == null || ys == null) {
                return xs == null && ys == null;
            }
            return Equals(xs, ys);
        }

        public int GetHashCode(object obj) {
            if (obj is string) {
                return GetHashCode((string)obj);
            }
            return obj.ToStringInvariant().ToLowerInvariant().GetHashCode();
        }
#endif
        }
    }

    public class IniSection : IEnumerable<KeyValuePair<string, IniValue>>, IDictionary<string, IniValue>
    {
        private Dictionary<string, IniValue> values;

        #region Ordered
        private List<string> orderedKeys;

        public int IndexOf(string key)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call IndexOf(string) on IniSection: section was not ordered.");
            }
            return IndexOf(key, 0, orderedKeys.Count);
        }

        public int IndexOf(string key, int index)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call IndexOf(string, int) on IniSection: section was not ordered.");
            }
            return IndexOf(key, index, orderedKeys.Count - index);
        }

        public int IndexOf(string key, int index, int count)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call IndexOf(string, int, int) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            if (count < 0)
            {
                throw new IndexOutOfRangeException("Count cannot be less than zero." + Environment.NewLine + "Parameter name: count");
            }
            if (index + count > orderedKeys.Count)
            {
                throw new ArgumentException("Index and count were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.");
            }
            var end = index + count;
            for (int i = index; i < end; i++)
            {
                if (Comparer.Equals(orderedKeys[i], key))
                {
                    return i;
                }
            }
            return -1;
        }

        public int LastIndexOf(string key)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call LastIndexOf(string) on IniSection: section was not ordered.");
            }
            return LastIndexOf(key, 0, orderedKeys.Count);
        }

        public int LastIndexOf(string key, int index)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call LastIndexOf(string, int) on IniSection: section was not ordered.");
            }
            return LastIndexOf(key, index, orderedKeys.Count - index);
        }

        public int LastIndexOf(string key, int index, int count)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call LastIndexOf(string, int, int) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            if (count < 0)
            {
                throw new IndexOutOfRangeException("Count cannot be less than zero." + Environment.NewLine + "Parameter name: count");
            }
            if (index + count > orderedKeys.Count)
            {
                throw new ArgumentException("Index and count were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.");
            }
            var end = index + count;
            for (int i = end - 1; i >= index; i--)
            {
                if (Comparer.Equals(orderedKeys[i], key))
                {
                    return i;
                }
            }
            return -1;
        }

        public void Insert(int index, string key, IniValue value)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call Insert(int, string, IniValue) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            values.Add(key, value);
            orderedKeys.Insert(index, key);
        }

        public void InsertRange(int index, IEnumerable<KeyValuePair<string, IniValue>> collection)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call InsertRange(int, IEnumerable<KeyValuePair<string, IniValue>>) on IniSection: section was not ordered.");
            }
            if (collection == null)
            {
                throw new ArgumentNullException("Value cannot be null." + Environment.NewLine + "Parameter name: collection");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            foreach (var kvp in collection)
            {
                Insert(index, kvp.Key, kvp.Value);
                index++;
            }
        }

        public void RemoveAt(int index)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call RemoveAt(int) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            var key = orderedKeys[index];
            orderedKeys.RemoveAt(index);
            values.Remove(key);
        }

        public void RemoveRange(int index, int count)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call RemoveRange(int, int) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            if (count < 0)
            {
                throw new IndexOutOfRangeException("Count cannot be less than zero." + Environment.NewLine + "Parameter name: count");
            }
            if (index + count > orderedKeys.Count)
            {
                throw new ArgumentException("Index and count were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.");
            }
            for (int i = 0; i < count; i++)
            {
                RemoveAt(index);
            }
        }

        public void Reverse()
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call Reverse() on IniSection: section was not ordered.");
            }
            orderedKeys.Reverse();
        }

        public void Reverse(int index, int count)
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call Reverse(int, int) on IniSection: section was not ordered.");
            }
            if (index < 0 || index > orderedKeys.Count)
            {
                throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
            }
            if (count < 0)
            {
                throw new IndexOutOfRangeException("Count cannot be less than zero." + Environment.NewLine + "Parameter name: count");
            }
            if (index + count > orderedKeys.Count)
            {
                throw new ArgumentException("Index and count were out of bounds for the array or count is greater than the number of elements from index to the end of the source collection.");
            }
            orderedKeys.Reverse(index, count);
        }

        public ICollection<IniValue> GetOrderedValues()
        {
            if (!Ordered)
            {
                throw new InvalidOperationException("Cannot call GetOrderedValues() on IniSection: section was not ordered.");
            }
            var list = new List<IniValue>();
            for (int i = 0; i < orderedKeys.Count; i++)
            {
                list.Add(values[orderedKeys[i]]);
            }
            return list;
        }

        public IniValue this[int index]
        {
            get
            {
                if (!Ordered)
                {
                    throw new InvalidOperationException("Cannot index IniSection using integer key: section was not ordered.");
                }
                if (index < 0 || index >= orderedKeys.Count)
                {
                    throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
                }
                return values[orderedKeys[index]];
            }
            set
            {
                if (!Ordered)
                {
                    throw new InvalidOperationException("Cannot index IniSection using integer key: section was not ordered.");
                }
                if (index < 0 || index >= orderedKeys.Count)
                {
                    throw new IndexOutOfRangeException("Index must be within the bounds." + Environment.NewLine + "Parameter name: index");
                }
                var key = orderedKeys[index];
                values[key] = value;
            }
        }

        public bool Ordered
        {
            get
            {
                return orderedKeys != null;
            }
            set
            {
                if (Ordered != value)
                {
                    orderedKeys = value ? new List<string>(values.Keys) : null;
                }
            }
        }
        #endregion

        public IniSection()
            : this(IniFile.DefaultComparer)
        {
        }

        public IniSection(IEqualityComparer<string> stringComparer)
        {
            this.values = new Dictionary<string, IniValue>(stringComparer);
        }

        public IniSection(Dictionary<string, IniValue> values)
            : this(values, IniFile.DefaultComparer)
        {
        }

        public IniSection(Dictionary<string, IniValue> values, IEqualityComparer<string> stringComparer)
        {
            this.values = new Dictionary<string, IniValue>(values, stringComparer);
        }

        public IniSection(IniSection values)
            : this(values, IniFile.DefaultComparer)
        {
        }

        public IniSection(IniSection values, IEqualityComparer<string> stringComparer)
        {
            this.values = new Dictionary<string, IniValue>(values.values, stringComparer);
        }

        public void Add(string key, IniValue value)
        {
            values.Add(key, value);
            if (Ordered)
            {
                orderedKeys.Add(key);
            }
        }

        public bool ContainsKey(string key)
        {
            return values.ContainsKey(key);
        }

        /// <summary>
        /// Returns this IniSection's collection of keys. If the IniSection is ordered, the keys will be returned in order.
        /// </summary>
        public ICollection<string> Keys
        {
            get { return Ordered ? (ICollection<string>)orderedKeys : values.Keys; }
        }

        public bool Remove(string key)
        {
            var ret = values.Remove(key);
            if (Ordered && ret)
            {
                for (int i = 0; i < orderedKeys.Count; i++)
                {
                    if (Comparer.Equals(orderedKeys[i], key))
                    {
                        orderedKeys.RemoveAt(i);
                        break;
                    }
                }
            }
            return ret;
        }

        public bool TryGetValue(string key, out IniValue value)
        {
            return values.TryGetValue(key, out value);
        }

        /// <summary>
        /// Returns the values in this IniSection. These values are always out of order. To get ordered values from an IniSection call GetOrderedValues instead.
        /// </summary>
        public ICollection<IniValue> Values
        {
            get
            {
                return values.Values;
            }
        }

        void ICollection<KeyValuePair<string, IniValue>>.Add(KeyValuePair<string, IniValue> item)
        {
            ((IDictionary<string, IniValue>)values).Add(item);
            if (Ordered)
            {
                orderedKeys.Add(item.Key);
            }
        }

        public void Clear()
        {
            values.Clear();
            if (Ordered)
            {
                orderedKeys.Clear();
            }
        }

        bool ICollection<KeyValuePair<string, IniValue>>.Contains(KeyValuePair<string, IniValue> item)
        {
            return ((IDictionary<string, IniValue>)values).Contains(item);
        }

        void ICollection<KeyValuePair<string, IniValue>>.CopyTo(KeyValuePair<string, IniValue>[] array, int arrayIndex)
        {
            ((IDictionary<string, IniValue>)values).CopyTo(array, arrayIndex);
        }

        public int Count
        {
            get { return values.Count; }
        }

        bool ICollection<KeyValuePair<string, IniValue>>.IsReadOnly
        {
            get { return ((IDictionary<string, IniValue>)values).IsReadOnly; }
        }

        bool ICollection<KeyValuePair<string, IniValue>>.Remove(KeyValuePair<string, IniValue> item)
        {
            var ret = ((IDictionary<string, IniValue>)values).Remove(item);
            if (Ordered && ret)
            {
                for (int i = 0; i < orderedKeys.Count; i++)
                {
                    if (Comparer.Equals(orderedKeys[i], item.Key))
                    {
                        orderedKeys.RemoveAt(i);
                        break;
                    }
                }
            }
            return ret;
        }

        public IEnumerator<KeyValuePair<string, IniValue>> GetEnumerator()
        {
            if (Ordered)
            {
                return GetOrderedEnumerator();
            }
            else
            {
                return values.GetEnumerator();
            }
        }

        private IEnumerator<KeyValuePair<string, IniValue>> GetOrderedEnumerator()
        {
            for (int i = 0; i < orderedKeys.Count; i++)
            {
                yield return new KeyValuePair<string, IniValue>(orderedKeys[i], values[orderedKeys[i]]);
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public IEqualityComparer<string> Comparer { get { return values.Comparer; } }

        public IniValue this[string name]
        {
            get
            {
                IniValue val;
                if (values.TryGetValue(name, out val))
                {
                    return val;
                }
                return IniValue.Default;
            }
            set
            {
                if (Ordered && !orderedKeys.Contains(name, Comparer))
                {
                    orderedKeys.Add(name);
                }
                values[name] = value;
            }
        }

        public static implicit operator IniSection(Dictionary<string, IniValue> dict)
        {
            return new IniSection(dict);
        }

        public static explicit operator Dictionary<string, IniValue>(IniSection section)
        {
            return section.values;
        }
    }

    public static class DataProtectionExtensions
    {
        public static string Protect(
            this string clearText,
            string optionalEntropy = "SuperProtectedPassword",
            DataProtectionScope scope = DataProtectionScope.CurrentUser)
        {
            try
            {
                byte[] clearBytes = Encoding.UTF8.GetBytes(clearText);
                byte[] entropyBytes = string.IsNullOrEmpty(optionalEntropy)
                    ? null
                    : Encoding.UTF8.GetBytes(optionalEntropy);
                byte[] encryptedBytes = ProtectedData.Protect(clearBytes, entropyBytes, scope);
                string retval = Convert.ToBase64String(encryptedBytes);
                //MessageBox.Show(retval);
                return retval;

            }
            catch
            {
                return "";
            }
        }

        public static string Unprotect(
            this string encryptedText,
            string optionalEntropy = "SuperProtectedPassword",
            DataProtectionScope scope = DataProtectionScope.CurrentUser)
        {
            if (encryptedText.Length <= 1)
                return "";
            try
            {
                byte[] encryptedBytes = Convert.FromBase64String(encryptedText);
                byte[] entropyBytes = string.IsNullOrEmpty(optionalEntropy)
                    ? null
                    : Encoding.UTF8.GetBytes(optionalEntropy);
                byte[] clearBytes = ProtectedData.Unprotect(encryptedBytes, entropyBytes, scope);
                string outval = Encoding.UTF8.GetString(clearBytes);
                //  MessageBox.Show(outval);
                return outval;
            }
            catch
            {
                return "";
            }
        }
    }



}
